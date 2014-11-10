#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "led.h"
#include "cmsis_os.h"


static uint_fast8_t led_prescaler = 0; // a prescaler for stepping down the flahsing frequency from 100Hz down to 1Hz

//	static int press;
//	static int tap;

/*************************************************************************************************************************/
/*  LED Init Function */
/*************************************************************************************************************************/
void led_init()
{
GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Configure all 4 LEDs in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = LED4GREEN | LED3ORANGE| LED5RED| LED6BLUE;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}
/*************************************************************************************************************************/
/*  FLASHING */ // LEDs toggle every 0.5s, assuming a calling frequency of 100Hz
/*************************************************************************************************************************/
void leds_flash()
{		
		// leds toggle signal = 1/100 of tim3 freq = 1Hz
	led_prescaler++; // increment prescaler
	if(led_prescaler == 50)
	{
		all_leds_on();
	}
	else if(led_prescaler >= 100)
	{
		all_leds_off();
		led_prescaler = 0; // reset prescaler
	}
}

/*************************************************************************************************************************/
/*  LEDs ON/OFF functions */
/*************************************************************************************************************************/

// turns all 4 LEDs on
void all_leds_on(){
	blueOn();
	greenOn();
	redOn();
	orangeOn();
}

// turns all 4 LEDs off
void all_leds_off(){
	blueOff();
	greenOff();
	redOff();
	orangeOff();
}
// turns green LED on
void greenOn(){
	// Green LED on
  GPIO_SetBits(GPIOD, LED4GREEN);
	
}
// turns orange LED on
void orangeOn(){
	// Orange LED on
  GPIO_SetBits(GPIOD, LED3ORANGE); 
}
// turns red LED on
void redOn(){
	// Red LED on
  GPIO_SetBits(GPIOD, LED5RED); 
}
// turns blue LED on
void blueOn(){
	// Blue LED on
	GPIO_SetBits(GPIOD, LED6BLUE);	
}
// turns green LED off
void greenOff(){
	// Green LED off
  GPIO_ResetBits(GPIOD, LED4GREEN);	
}
// turns orange LED off
void orangeOff(){
	// Orange LED off
  GPIO_ResetBits(GPIOD, LED3ORANGE); 
}
// turns red LED off
void redOff(){
	// Red LED off
  GPIO_ResetBits(GPIOD, LED5RED); 
}
// turns blue LED off
void blueOff(){
	// Blue LED off
	GPIO_ResetBits(GPIOD, LED6BLUE);
}
// toggles all LEDs (use with a timer / delay  to flash)
void leds_toggle()
{	
	GPIO_ToggleBits(GPIOD, LED4GREEN|LED3ORANGE|LED5RED|LED6BLUE);
}
