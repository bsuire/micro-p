#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "math.h"
#include "accelerometer.h"
#include "temperature_sensor.h"
#include "cmsis_os.h"
#include "led.h"
#include "displayTemperature.h"
#include "displayEdge.h"
#include "Display_UI.h"
#include "moveDetect.h"
#include "display_enum.h"

extern display_state_type display_state;
extern int32_t accSampled;
static uint8_t state;

osThreadDef(display_UI_thread, osPriorityNormal, 1, 0);
/*************************************************************************************************************************/
/*  DISPLAY UI THREAD */ // calls the function corresponding to the current display mode
// 													Note: activated every 100Hz (once accelerometer data is ready)
/*************************************************************************************************************************/

void display_UI_thread (void const *argument){
	while(1){
		osSignalWait(accSampled,osWaitForever);
		// TIMER TICK -- increment flashing prescaler
		//osSignalWait(led_flash_tick, 0);
		//if(led_flash_tick) // increment "prescaler" counter each timer tick

		if(display_state == TEMPERATURE_DISPLAY)
		{
			displayTemperature();
		}
		else if(display_state == TEMPERATURE_FLASH)
		{
			leds_flash();
		}
		else if(display_state == ACCELEROMETER_EDGE)
		{
			displayEdge();
			// reset high pass filter when first come into this state
			if(state!=3){
				highPassReset();
			}
			state=3;
		}
		else if(display_state == ACCELEROMETER_DIRECTION)
		{
			moveDetect(); 
			if(state!=4){
				highPassSet();
			}
			state=4;
		}
			else if(display_state == LEDS_ON)
		{
			all_leds_on();
		}
			else if(display_state == LEDS_OFF)
		{
			all_leds_off();
		}
		else
		{
			all_leds_on();
		}
		
		//osDelay(10); // 10 ms // 100Hz
	}
}
/*************************************************************************************************************************/
/* DISPLAY UI THREAD START FUNCTION */
/*************************************************************************************************************************/
osThreadId display_UI_start_thread(void){
	return osThreadCreate(osThread(display_UI_thread), NULL);
}
