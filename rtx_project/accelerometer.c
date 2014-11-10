#include "accelerometer.h"
#include "math.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "stm32f4_discovery_lis302dl.h"
#include "modeHandler.h"
#include "moving_average.h"
#include "stm32f4xx.h"
#include "cmsis_os.h"


// Global Variables
// Accelerometer thread ID
extern osThreadId tid_accelerometer_thread; // for setting up the DMA data ready signal
extern osThreadId tid_display_UI;
// OS Signals
extern int32_t accelerometer_tick;   		// timer 
extern int32_t accelerometer_data_ready; // DMA data ready
extern int32_t accSampled;
// Moving Averages
extern ma_state ma_x;
extern ma_state ma_y;
extern ma_state ma_z;

// Semaphores
osSemaphoreId all_axes; // calibrated axes
osSemaphoreDef(all_axes);

osSemaphoreId all_raw_axes; //non-calibrated axes
osSemaphoreDef(all_raw_axes);

osSemaphoreId all_angles; // roll and pitch
osSemaphoreDef(all_angles);

// DMA and EXTI Config structures
DMA_InitTypeDef    DMA_InitStructure;
EXTI_InitTypeDef   EXTI_InitStructure;

// Local Variables
static float axisParameters[3];
static float rawData[3];
static float angles[2];

// arrays for SPI Rx/Tx
static int8_t accelerometer_spi_in[7];
static int8_t accelerometer_spi_out[7] =  {LIS302DL_OUT_X_ADDR , 0, 0, 0, 0, 0, 0};



/*************************************************************************************************************************/
/*  ACCELEROMETER INIT */
// intialize accelerometer with appropriate parameters for measuring 3D acceleration
// and enabling single tap detection
/*************************************************************************************************************************/
void accelerometer_init(void)
{
  uint8_t ctrl = 0;
  
  LIS302DL_InitTypeDef LIS302DL_InitStruct; // Declare structure
	
	/* Set configuration of LIS302DL */
	LIS302DL_InitStruct.Power_Mode = LIS302DL_LOWPOWERMODE_ACTIVE;
	LIS302DL_InitStruct.Output_DataRate = LIS302DL_DATARATE_100;
	LIS302DL_InitStruct.Axes_Enable = LIS302DL_X_ENABLE | LIS302DL_Y_ENABLE |
	LIS302DL_Z_ENABLE;     // enable sensing along all 3 axe?
	LIS302DL_InitStruct.Full_Scale = LIS302DL_FULLSCALE_2_3;
	LIS302DL_InitStruct.Self_Test = LIS302DL_SELFTEST_NORMAL;
	LIS302DL_Init(&LIS302DL_InitStruct); // Actually initialize the accelerometer

  /* Enabling interrupts */
  ctrl = 0x38;//111000; //ctrl = 0x07; // 00000111 for click interrupt on exti 1		
	LIS302DL_Write(&ctrl, LIS302DL_CTRL_REG3_ADDR, 1);
	/* Enable Interrupt generation on click/double click on Z axis */
  ctrl = 0x10; // there are 8 bits I can write to REG_ADDR, each mean something, I need to enable 4th bit, and also 6th, that's 0x50
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_CFG_REG_ADDR, 1);
  /* Configure Click Threshold on X/Y axis (10 x 0.5g) */
  ctrl = 0xFF;// which means 10, and 10
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_THSY_X_REG_ADDR, 1);
  /* Configure Click Threshold on Z axis (10 x 0.5g) */
  ctrl = 0x0F; // 14 -- 7g
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_THSZ_REG_ADDR, 1);
  /* Configure Time Limit */
  ctrl = 0x03;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_TIMELIMIT_REG_ADDR, 1);
  /* Configure Latency */
  ctrl = 0x7F;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_LATENCY_REG_ADDR, 1);
  /* Configure Click Window */
  ctrl = 0x7F;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_WINDOW_REG_ADDR, 1);  
	

	/* External interrupt config: */
	
	// Init structures for GPIO and NViC
	GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
	
	
	/*Enable GPIOE clock*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  /* Configure PE0 pin as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // we'll read for interrupts
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // floating
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  // mapped to pin 0 (hardware restriction)
  GPIO_Init(GPIOE, &GPIO_InitStructure);  // passes configuration parameters ot GPIOE struct

  /* Connect EXTI Line1 to PE0 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1); // need to connect external interrupt to pin

  /* Configure EXTI Line1 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // interrupt will be triggered on rising edge 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI Line1 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn; // add EXTI0_IRQn to NVIC   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; // set priority level
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
		
	
	/* DMA settings */
	/* Enable DMA2 clock (set on high-speed AHB1 clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	 /* DMA2 Stream0 channel 3 configuration (receiving)  **************************************/

  DMA_InitStructure.DMA_Channel = DMA_Channel_3;  
	// SPI1 data register (Reference Manual p722)
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&SPI1->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = 0;//like this???//(uint32_t)&ADC1ConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	
	/* DMA2 Stream3 channel 3 configuration (sending)  **************************************/
  // sending
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_Init(DMA2_Stream3, &DMA_InitStructure);
	
	// Enable transmission / receiving: starts
	SPI_DMACmd(SPI1, SPI_DMAReq_Rx | SPI_DMAReq_Tx , ENABLE);	
	
	/* first value of sent will be address to start reading from SPI */
	accelerometer_spi_out[0] = LIS302DL_OUT_X_ADDR | 0x40 | 0x80;

	/* Set up DMA interrupt on receiving end */
	NVIC_InitTypeDef nvic_init_s;

	nvic_init_s.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	nvic_init_s.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_init_s.NVIC_IRQChannelSubPriority = 2;
	nvic_init_s.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_s);

	/* Set interrupt when receiving data complete */
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	
}
/*************************************************************************************************************************/
/*  Return calibrated x, y, z  */ // Protected by semaphore
/*************************************************************************************************************************/
void get_axes(float* led_axes){
	osSemaphoreWait(all_axes, osWaitForever);
		led_axes[0] = axisParameters[0];
		led_axes[1] = axisParameters[1];
		led_axes[2] = axisParameters[2];
	osSemaphoreRelease(all_axes);
}
/*************************************************************************************************************************/
/*  Return raw x, y, z  */ // Protected by semaphore
/*************************************************************************************************************************/
void get_rawAxes(float* led_axes){
	osSemaphoreWait(all_raw_axes, osWaitForever);
		led_axes[0] = rawData[0];
		led_axes[1] = rawData[1];
		led_axes[2] = rawData[2];
	osSemaphoreRelease(all_raw_axes);
}
/*************************************************************************************************************************/
/*  Return x */  // DEPRECATED
/*************************************************************************************************************************/
float get_x(){
	return axisParameters[0]; // x

}/*************************************************************************************************************************/
/*  Return y */ // DEPRECATED
/*************************************************************************************************************************/
float get_y(){
	return axisParameters[1]; // y

}/*************************************************************************************************************************/
/*  Return z */ // DEPRECATED
/*************************************************************************************************************************/
float get_z(){
	return axisParameters[2];// z
}

/*************************************************************************************************************************/
/*  Return roll, pitch */ // Protected by Semaphore
/*************************************************************************************************************************/
void get_angles(float* led_angles){
	osSemaphoreWait(all_angles, osWaitForever);
		led_angles[0] = angles[0]; // roll
		led_angles[1] = angles[1]; // pitch
	osSemaphoreRelease(all_angles);
}
/*************************************************************************************************************************/
/*  Return roll */ // DEPRECATED
/*************************************************************************************************************************/
float get_roll(){
	return angles[0]; // roll
}
/*************************************************************************************************************************/
/*  Return pitch */ // DEPRECATED
/*************************************************************************************************************************/
float get_pitch(){
	return angles[1]; // pitch
}

/*************************************************************************************************************************/
/*  ACCELEROMETER THREAD */
/*************************************************************************************************************************/

osThreadDef(accelerometer_thread, osPriorityNormal, 1, 0);

void accelerometer_thread (void const *argument){

	while(1){
		//wait on OS timer signal for sampling (100Hz)
		osSignalWait(accelerometer_tick, osWaitForever);
			
		//get_accelerometer_data();
		DMA_get_accelerometer_data();
		
		// wait for DMA transmission to terminate
		osSignalWait(accelerometer_data_ready, osWaitForever);

		osSemaphoreWait(all_raw_axes, osWaitForever);
				rawData[0]=(float)accelerometer_spi_in[1];
				rawData[1]=(float)accelerometer_spi_in[3];
				rawData[2]=(float)accelerometer_spi_in[5];
		osSemaphoreWait(all_raw_axes, osWaitForever);

		// the spi_in data are such: [(-53,55.5)(-58.5,56)(-63,52.5)}, calibrate accordingly
		osSemaphoreWait(all_axes, osWaitForever);
				axisParameters[0] = ((float)accelerometer_spi_in[1]-1.25f)/54.25f;
				axisParameters[1] = ((float)accelerometer_spi_in[3]+1.25f)/57.25f;
				axisParameters[2] = ((float)accelerometer_spi_in[5]+5.25f)/57.75f;
				filter_accelerations();
		osSemaphoreWait(all_axes, osWaitForever);
		
		update_angles();  // updates roll and pitch, protected by semaphore
		
		osSignalSet(tid_display_UI, accSampled);		
	}
}
/*************************************************************************************************************************/
/*  ACCELEROMETER THREAD START FUNCTION */
/*************************************************************************************************************************/
osThreadId accelerometer_start_thread(void){
	return osThreadCreate(osThread(accelerometer_thread), NULL);
}

/*************************************************************************************************************************/
/*  GET ACCELEROMETER DATA THROUGH DMA */
/*************************************************************************************************************************/

void DMA_get_accelerometer_data(){
	 	
		/* Set the CS_GPIO_PORT low */
		GPIO_ResetBits(LIS302DL_SPI_CS_GPIO_PORT, LIS302DL_SPI_CS_PIN);
	
			// receiving
		DMA2_Stream0->NDTR = 7;
		DMA2_Stream0->M0AR = (uint32_t)&(accelerometer_spi_in[0]); // char type. Is that equivalent to byte??
		DMA_Cmd(DMA2_Stream0, ENABLE);
	
		// sending part
		DMA2_Stream3->NDTR = 7;
		DMA2_Stream3->M0AR = (uint32_t)&(accelerometer_spi_out[0]); // char
		DMA_Cmd(DMA2_Stream3, ENABLE);				
}


/*************************************************************************************************************************/
/*  MOVING AVERAGE FILTER. NOT USED IN LAB 4 */
/*************************************************************************************************************************/
void filter_accelerations(void)
{
	axisParameters[0] = moving_average(&ma_x, axisParameters[0]); // x
	axisParameters[1] = moving_average(&ma_y, axisParameters[1]); // y
	axisParameters[2] = moving_average(&ma_z, axisParameters[2]); // z
}


// get values of roll and pitch from axis acceleration values
/*************************************************************************************************************************/
/*  UPDATE ANGLES: PUTS FINAL ROLL AND PITCH IN ANGLES ARRAY. PROTECTED BY SEMAPHORE */
/*************************************************************************************************************************/
void update_angles(){ // 57.2957795131 = 180/pi, used to convert radians to degrees
	osSemaphoreWait(all_angles, osWaitForever);
		angles[0] = 57.2957795131*atan(axisParameters[0]/sqrt(axisParameters[1]*axisParameters[1]+axisParameters[2]*axisParameters[2]));//roll
		angles[1] = 57.2957795131*atan(axisParameters[1]/sqrt(axisParameters[0]*axisParameters[0]+axisParameters[2]*axisParameters[2]));//pitch
	osSemaphoreRelease(all_angles);
}

/*************************************************************************************************************************/
/*  DMA IRQ HANDLER. SENDS SIGNAL WHEN DMA TRANSMISSION IS DONE */
/*************************************************************************************************************************/
void DMA2_Stream0_IRQHandler(void) {
	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0)) {

		/* Set CS high */
		GPIO_SetBits(LIS302DL_SPI_CS_GPIO_PORT, LIS302DL_SPI_CS_PIN);

		/* Clear all the flags */
		DMA_ClearFlag(DMA2_Stream0, DMA_FLAG_TCIF0);
		DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3);
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);

		/* Disable DMA */
		//DMA2_Stream0->CR |= 0;
		//DMA2_Stream3->CR |= 0;
		DMA_Cmd(DMA2_Stream0, DISABLE);
		DMA_Cmd(DMA2_Stream3, DISABLE);
		
		osSignalSet(tid_accelerometer_thread, accelerometer_data_ready);
	}
}

/*************************************************************************************************************************/
/*  Disable Accelerometer High-Pass Filter (for measuring tilt) */
/*************************************************************************************************************************/
void highPassReset(){
	/* disable embedded high pass filter */
	uint8_t ctrl = 0x00;
	LIS302DL_Write(&ctrl, LIS302DL_CTRL_REG2_ADDR, 1);  
}

/*************************************************************************************************************************/
/*  Enable Accelerometer High-Pass Filter (to filter gravity when measuring acceleration/movement) */
/*************************************************************************************************************************/
void highPassSet(){
	/* enable embedded high pass filter */
	uint8_t ctrl = 0x10;
	LIS302DL_Write(&ctrl, LIS302DL_CTRL_REG2_ADDR, 1);  
}
