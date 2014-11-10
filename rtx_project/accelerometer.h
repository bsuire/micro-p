/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ACCELEROMETER_H
#define __ACCELEROMETER_H

/* Includes ------------------------------------------------------------------*/
//#include "stm32f4_discovery.h"
//#include "stm32f4_discovery_lis302dl.h"
#include "math.h"
#include "cmsis_os.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DOUBLECLICK_Z                    ((uint8_t)0x60)
#define SINGLECLICK_Z                    ((uint8_t)0x50)

/* TIM2 Autoreload and Capture Compare register values */
//#define TIM_ARR                          ((uint16_t)1900)
//#define TIM_CCR                          ((uint16_t)1000)
/* Exported macro ------------------------------------------------------------*/
#define ABS(x)                           (x < 0) ? (-x) : x
#define MAX(a,b)                         (a < b) ? (b) : a
/* Exported functions ------------------------------------------------------- */
void accelerometer_init(void);
void accelerometer_thread (void const *argument);
void DMA_get_accelerometer_data(void);
void filter_accelerations(void);
void update_angles(void);
void get_angles(float*);
void get_axes(float*);
void get_rawAxes(float*);
float get_x(void);
float get_y(void);
float get_z(void);
float get_roll(void);
float get_pitch(void);
osThreadId accelerometer_start_thread(void);
//int isTap(void);
void DMA2_Stream0_IRQHandler(void);
void highPassReset(void);
void highPassSet(void);
#endif 

