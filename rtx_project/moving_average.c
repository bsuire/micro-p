#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "math.h"
#include "moving_average.h"

// initialization fuction

void moving_average_init(ma_state *my_ma){
	my_ma->average = 0;
	my_ma->index = 0;
	
	for(int i =0; i<MOVING_AVERAGE_SIZE_INT; i++)
		{
			my_ma-> window[i] = 0;
		}
}

// moving average using block parameter passing (circular queue) + new value
// returns an averaged value

float moving_average(ma_state *my_ma, float new_value){
	//ignore any new value that deviates more than 3 degree from the current average, unless it's just initialized
  if(fabs(new_value-my_ma->average)<3){
  // calculate new average value using recursive moving average formula
	my_ma->average = my_ma->average + (new_value - my_ma->window[my_ma->index])/MOVING_AVERAGE_SIZE;
	
  my_ma->window[my_ma->index] = new_value; // overwrite odlest value in array with newest
		
	my_ma->index += 1;  // increment index to point to the next oldest value in array
  
	// index wrap around check for implementing a circular queue
	if(my_ma->index == (MOVING_AVERAGE_SIZE_INT)) // check if end of array eached									
		{
			my_ma->index = 0; // if so, return to first position in the array																
		} 
	
	}else{
		// when it's just initialized, take in data
		if(my_ma->window[my_ma->index]==0){
			my_ma->average = my_ma->average + (new_value - my_ma->window[my_ma->index])/MOVING_AVERAGE_SIZE;
	
			my_ma->window[my_ma->index] = new_value; // overwrite odlest value in array with newest
		
			my_ma->index += 1;  // increment index to point to the next oldest value in array
  
			// index wrap around check for implementing a circular queue
		if(my_ma->index == (MOVING_AVERAGE_SIZE_INT)) // check if end of array eached									
		{
			my_ma->index = 0; // if so, return to first position in the array																
		} 
	
	}
}
return  my_ma->average;	
}

// exponential moving average, used for accelerometer
float moving_average_exp(float avg, float new_value){
	// Avg=(1-a)*Avg+a*newValue;
	avg=(1.0f-ALPHA)*avg+ALPHA*new_value;
	return  avg;
}
	

