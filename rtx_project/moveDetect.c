#include "moveDetect.h"
#include "math.h"
#include "led.h"
#include "accelerometer.h"
#include "moving_average.h"
#include "Display_UI.h"

extern uint_fast8_t accelerometer_tick; // WARNING, changed this to signal
extern uint_fast8_t accSampled;
int read;
static float accSquare;
float axisAcc[3];

/*************************************************************************************************************************/
/*  Movement Display */ // LEDs light up to display the direction of movement along x, y or z axes only (not diagonally)
// Note: +z axis indicated by two opposide leds being simultaneously on
// 			 -z axis indicated by all four LEDS being turned on
// 			 For display along x or y axis, the corresponding LED lights up
/*************************************************************************************************************************/
void moveDetect(){
	get_rawAxes(axisAcc); // now use this instead of individual gets. I added a semaphore so it's protected.
	accSquare=axisAcc[0]*axisAcc[0]+axisAcc[1]*axisAcc[1]+axisAcc[2]*axisAcc[2];
	if (accSquare<100){
		all_leds_off();
	}else{
		if (axisAcc[0]<-8){
		//orangeOn();
		blueOn();
			
		}else if (axisAcc[0]>8){
			//blueOn();
			orangeOn();
			
		}
		if(axisAcc[1]<-8){
			//greenOn();
			redOn();
			
		}else if (axisAcc[1]>8){
			//redOn();
			greenOn();
			//osDelay(200);
		}
		
		// go down
		if(axisAcc[2]<-8){
			all_leds_on();
			
		}else if (axisAcc[2]>8){
			blueOn();
			orangeOn();
			
		}
		osDelay(400);	//osDelay(400);	
	}
}
