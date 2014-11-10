#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H
#include "cmsis_os.h"
void temperature_sensor_init(void);
void temperature_sensor_thread(void const *argument);
float bits_to_celsius(uint16_t reading);
float get_temperature(void);
osThreadId temperature_sensor_start_thread(void);
#endif
