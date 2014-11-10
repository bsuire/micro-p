#ifndef MODEHANDLER_H
#define MODEHANDLER_H
#include "cmsis_os.h"


void modeHandler_thread(void const *argument);
void display_edge(void); // float roll, float pitch
void display_direction(void); //float x, float y, float z, float roll, float pitch

osThreadId modeHandler_start_thread(void);


#endif
