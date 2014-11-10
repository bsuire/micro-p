#include "led.h"
#include "temperature_sensor.h"
#include "displayTemperature.h"
static float  temperature_t;

extern osSemaphoreId temperature_lock;


/*************************************************************************************************************************/
/*  DISPLAY_TEMPERATURE */
/*************************************************************************************************************************/
void displayTemperature()
{
	osSemaphoreWait(temperature_lock, osWaitForever);
	temperature_t = get_temperature();
	osSemaphoreRelease(temperature_lock);
	
	temperature_t = (int)temperature_t%8; // 2 degrees apart, total 8 degrees
	
	
	if (temperature_t<2) // here using another variable
	{   
    greenOn();
	  orangeOff();
		redOff();
		blueOff();
	}
  else if(temperature_t<4)
	{
    greenOff();
	  orangeOn();
		redOff();
		blueOff();
	}
  else if(temperature_t<6)
	{
    greenOff();
	  orangeOff();
		redOn();
		blueOff();
	} 
	else
	{
    greenOff();
	  orangeOff();
		redOff();
		blueOn();
	}
}
