#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H
#include "cmsis_os.h"


void display_UI_thread(void const *argument);

osThreadId display_UI_start_thread(void);


#endif
