#include "displayEdge.h"
#include "led.h"
#include "accelerometer.h"

// Local variable
static float leds_angles[2];

/*************************************************************************************************************************/
/*  Edge Display */ // LEDs light up indicating the direction of the ground
/*************************************************************************************************************************/
void displayEdge(){

	get_angles(leds_angles); 

			
	if (leds_angles[0]<-8){ // roll
		orangeOn();
		blueOff();
	}else if (leds_angles[0]>8){ // roll
		blueOn();
		orangeOff();
	}else{
		blueOff();
		orangeOff();
	}
	
	if (leds_angles[1]<-8){ // pitch
		greenOn();
		redOff();
	}else if (leds_angles[1]>8){ // pitch
		redOn();
		greenOff();
	}else{
		greenOff();
		redOff();
	}
}
