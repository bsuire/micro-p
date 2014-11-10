#include "stm32f4xx_conf.h"
#include <stdio.h>
#include "temperature_sensor.h"
#include "moving_average.h"
#include "cmsis_os.h"
#define ADC1_DR_ADDRESS     ((uint32_t)0x4001224C)

// Global Variables
extern int32_t temperature_sensor_tick; 	// timer signal
extern ma_state ma_temperature;						// moving average state

// Local Variables
static float temp_t;			// temporary temperature value for processing
static float temperature; // final temperature value

// Semaphore for protecting "temperature" read/write operations
osSemaphoreId temperature_lock;
osSemaphoreDef(temeprature_lock);

/*************************************************************************************************************************/
/*  GET TEMPERATURE // Note: function call must be protected with the use of the semaphore "temprature_lock"*/ 
/*************************************************************************************************************************/
float get_temperature(){
		return temperature;
}

/*************************************************************************************************************************/
/*  TEMPERATURE SENSOR INIT */
/*************************************************************************************************************************/
void temperature_sensor_init(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;	

  /* Enable ADC1, DMA2 and GPIO clocks ****************************************/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	
  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC3 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

	ADC_TempSensorVrefintCmd(ENABLE);
  /* ADC1 regular channel12 configuration *************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_480Cycles);//480 cycles = max
	
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
}

/*************************************************************************************************************************/
/*  TEMPERATURE THREAD */ // Manages Sampling and processing of the temperature sensor // Semaphore protection for writing of temperature value
/*************************************************************************************************************************/
osThreadDef(temperature_sensor_thread, osPriorityNormal, 1, 0);

void temperature_sensor_thread (void const *argument){

	
	while(1){
		
		// wait for sampling signal
		osSignalWait(temperature_sensor_tick, osWaitForever);
		
		ADC_SoftwareStartConv(ADC1);
		
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);	
		
		// Protected write operation
		osSemaphoreWait(temperature_lock, osWaitForever);
		temperature = moving_average(&ma_temperature, bits_to_celsius(ADC_GetConversionValue(ADC1))); 
		osSemaphoreRelease(temperature_lock);
			
	}
}
/*************************************************************************************************************************/
/*  TEMPERATURE SENSOR THREAD START */
/*************************************************************************************************************************/
osThreadId temperature_sensor_start_thread(void){
	return osThreadCreate(osThread(temperature_sensor_thread), NULL);
}

/*************************************************************************************************************************/
/*  Converts Digital value to Degrees Celsius */
/*************************************************************************************************************************/
float bits_to_celsius(uint16_t reading){
	temp_t= (((0.00073260073f*((float)reading) - 0.76f)*400.0f) + 25.0f); //0.00073260073
	return temp_t;
	// ((Vsense-V25)/avg_slope)+25
	// V_sense: 0.00073260073*reading converts reading to a voltag value. The factor is 3/(2^12 -1), where 3 is the max ref voltage, while 2^12 - 1 is the max bit value
	// 0.76 is V25, and avg_slope is 2.5mV/C, so *400
}

