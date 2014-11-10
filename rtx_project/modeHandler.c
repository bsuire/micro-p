#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "modeHandler.h"
#include "math.h"
#include "accelerometer.h"
#include "temperature_sensor.h"
#include "cmsis_os.h"
#include "led.h"
#include "displayTemperature.h"
#include "displayEdge.h"
#include "moveDetect.h"
#include "display_enum.h"


// Global Variables
extern osThreadId tid_modeHandler;				// modeHandler thread	
extern int32_t mode_handler_wake_up;			// mode handler signal, generated by tap or press
extern display_state_type display_state;	// current display state (flash, temeprature, angle/edge, movement/acceleration

// Local variables
// flags 
static uint32_t press = 0; 					// button
static uint32_t tap = 0; 						// acc tap detection

/*************************************************************************************************************************/
/*  MODE HANDLER THREAD */ 	/* Manages  display mode switches based on tap or button press detection */ 
/*************************************************************************************************************************/
osThreadDef(modeHandler_thread, osPriorityNormal, 1, 0);

void modeHandler_thread (void const *argument){

	while(1){
		
		osSignalWait(mode_handler_wake_up, osWaitForever);
		
		// TAP DETECTION -- Toggle Temperature/Accelerometer		
		// BUTTON PRESS -- toggle Flash/Normal Display or Edge/Direction
		
		if(press)
		{
			// button press may debounce in <200ms
			osDelay(200);
			press = 0;
			tap=0;
			switch (display_state) // Toggling from flash<->temperature or  edge<->movement
			{ 
			case TEMPERATURE_FLASH : 
						display_state = TEMPERATURE_DISPLAY;
						break;
			case TEMPERATURE_DISPLAY: 
						display_state = TEMPERATURE_FLASH;
						break;
			case ACCELEROMETER_EDGE: 
						display_state = ACCELEROMETER_DIRECTION;
						break;
			case ACCELEROMETER_DIRECTION: 
						display_state = ACCELEROMETER_EDGE;
						break;
			default: display_state = ACCELEROMETER_DIRECTION;
			}
		}else if(tap)
		{
			tap = 0; // toggling from flash/temperature to edge or edge/movement to flash
			if(display_state == TEMPERATURE_FLASH || display_state == TEMPERATURE_DISPLAY)
			{
				display_state = ACCELEROMETER_EDGE;
			}
			else
			{
				display_state = TEMPERATURE_FLASH;
			}
		}
	}	
}

/*************************************************************************************************************************/
/*  MODE HANDLER THREAD START FUNCTION */
/*************************************************************************************************************************/
osThreadId modeHandler_start_thread(void){
	return osThreadCreate(osThread(modeHandler_thread), NULL);
}

/*************************************************************************************************************************/
/*  User Button IRQ Handler  */ // generates a signal for modeHandler and sets a flag for press
/*************************************************************************************************************************/
 void EXTI0_IRQHandler(void) // button interrupt
{
	osSignalSet(tid_modeHandler, mode_handler_wake_up);
	press = 1; // button detected
   /* Clear the EXTI line 0 pending bit */
   EXTI_ClearITPendingBit(EXTI_Line0);
}

/*************************************************************************************************************************/
/*  Tap Detection IRQ Handler  */ // generates a signal for modeHandler and sets a flag for tap
/*************************************************************************************************************************/
void EXTI1_IRQHandler(void) // tap interrupt
{
	osSignalSet(tid_modeHandler, mode_handler_wake_up);
	tap = 1; // tap detected
  /* Clear the EXTI line I pending bit */
  EXTI_ClearITPendingBit(EXTI_Line1);
}
