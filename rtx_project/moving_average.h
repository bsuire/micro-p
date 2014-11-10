#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#define MOVING_AVERAGE_SIZE 10.0f
#define MOVING_AVERAGE_SIZE_INT 10
#define ALPHA 0.85f

typedef struct{
  float		average;
	float		window[MOVING_AVERAGE_SIZE_INT];
	uint8_t index;
}ma_state;

void moving_average_init(ma_state *my_moving_average) ;
float moving_average(ma_state *my_moving_average, float new_value);// should take a pointer to the state as a parameter
float moving_average_exp(float avg, float new_value);

#endif

