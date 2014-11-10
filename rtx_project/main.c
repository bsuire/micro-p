#include "arm_math.h"
#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "modeHandler.h" 
#include "led.h"
#include "accelerometer.h"
#include "temperature_sensor.h"
#include "push_button.h"
#include "timer.h"
#include "moving_average.h"
#include "moveDetect.h"
#include "display_UI.h"
#include "display_enum.h"

/************ VARIABLE DECLARATIONS **************/
// GLOBAL VARIABLES
// moving average states
ma_state ma_x;
ma_state ma_y;
ma_state ma_z;
ma_state ma_temperature;

// thread IDs
osThreadId tid_accelerometer_thread;
osThreadId tid_temp_sensor;
osThreadId tid_modeHandler;
osThreadId tid_display_UI;

// display state (flash, temperature display, edge display, direction of movement display)
display_state_type display_state = TEMPERATURE_FLASH;

// Signals
// accelerometer thread
int32_t accelerometer_tick = 0x1;   		// timer 
int32_t accelerometer_data_ready = 0x2; // DMA data ready
int32_t accSampled=0x4;
// temp sensor thread
int32_t temperature_sensor_tick = 0x1; 	// timer
// UI thread (mode handler)
int32_t mode_handler_wake_up = 0x1; // press or tap



// LOCAL VARIABLES
// Timer prescaler for temp sensor
static uint_fast8_t temp_sensor_prescaler = 0;

 
void thread(void const * argument);

//! Thread structure for above thread
osThreadDef(thread, osPriorityNormal, 1, 0);

int main (void) {
	// Initializations of peripherals
	accelerometer_init(); // accelerometer
  temperature_sensor_init();     
	led_init();
	button_init();
	timer_init();         // timer interrupt
	
	// Filters initializations
	moving_average_init(&ma_x); // initialize moving average fitlers
	moving_average_init(&ma_y);
	moving_average_init(&ma_z);
	moving_average_init(&ma_temperature);

	// Start threads
  tid_accelerometer_thread = accelerometer_start_thread();
	tid_temp_sensor = temperature_sensor_start_thread();
	tid_modeHandler = modeHandler_start_thread();
	tid_display_UI = display_UI_start_thread();

	// The below doesn't really need to be in a loop
	while(1){
		osDelay(osWaitForever);
	}
}

void thread (void const *argument) {
		
			while (1)
  {			
		osDelay(100);
	}
}
/*************************************************************************************************************************/
/*  Timer IRQ Handler */ // controls the sampling rates and flashing LEDs timer (via accelerometer sampling rate)
/*************************************************************************************************************************/

void TIM3_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // clear interrupt
	
	// accelerometer sampling signal: = freq of tim3 timer = 100Hz
	osSignalSet(tid_accelerometer_thread, accelerometer_tick);
	
	// temp sensor sempling signal = 1/5 of tim3 freq = 20Hz
	temp_sensor_prescaler++; // update counter every tick
	if(temp_sensor_prescaler == 5) // 20Hz = 1/5 of 100Hz, where 100Hz is the frequency at which tim3 is set
	{
		osSignalSet(tid_temp_sensor, temperature_sensor_tick);
		temp_sensor_prescaler = 0; // reset temp sensor prescaler
	}
}

