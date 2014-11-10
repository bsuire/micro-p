#ifndef LED_H
#define LED_H
void led_init(void);
void leds_flash(void); // uint_fast8_t timer_tick
void leds_toggle(void);
void all_leds_on(void);
void all_leds_off(void);


void blueOn(void);
void redOn(void);
void orangeOn(void);
void greenOn(void);
void blueOff(void);
void redOff(void);
void orangeOff(void);
void greenOff(void);

#define LED4GREEN  GPIO_Pin_12
#define LED3ORANGE  GPIO_Pin_13
#define LED5RED  GPIO_Pin_14
#define LED6BLUE  GPIO_Pin_15

#endif
