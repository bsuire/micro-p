#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"


// Note we need sampling rate of 100Hz

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
__IO uint16_t CCR1_Val = 0; // output an event when reached zero
//static uint16_t PrescalerValue = 0;

/* Private function prototypes -----------------------------------------------*/


// Timer initialization function
void timer_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//

  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;   // ads tim3 to NVIC so it can recognize it
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // highest priority level. Precise sampling is more important than tap (priority 2)
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // enables interrupt
  NVIC_Init(&NVIC_InitStructure);

	SystemCoreClockUpdate();// adde by Ben March 5th
	
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 500; //start down-counter at 500
  TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;//maximum value for prescaler//SystemCoreClock (uint16_t) ((SystemCoreClock / 2) / 50000) - 1;;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;// down counter (recommended for tim3)

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);// passes configuration params to TIM3 struct

  /* TIM Interrupts enable */
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);

}

