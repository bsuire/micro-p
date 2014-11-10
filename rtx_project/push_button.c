#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "push_button.h"

EXTI_InitTypeDef   EXTI_InitStructure_button;

void button_init()
{
GPIO_InitTypeDef  GPIO_InitStructure_button;
NVIC_InitTypeDef  NVIC_InitStructure_button; // for the interrupt

/*Enable GPIOE clock*/
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
/* Enable SYSCFG clock */
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Configure PA0 in input pull down mode */
  GPIO_InitStructure_button.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure_button.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure_button.GPIO_OType = GPIO_OType_PP; // type?
  GPIO_InitStructure_button.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure_button.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOD, &GPIO_InitStructure_button);
	
	/* Connect EXTI Line0 to PE0 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0); // need to connect external interrupt to pin

  /* Configure EXTI Line0 */
  EXTI_InitStructure_button.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure_button.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure_button.EXTI_Trigger = EXTI_Trigger_Rising; // interrupt will be triggered on rising edge 
  EXTI_InitStructure_button.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure_button);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitStructure_button.NVIC_IRQChannel = EXTI0_IRQn; // add EXTI0_IRQn to NVIC   
  NVIC_InitStructure_button.NVIC_IRQChannelPreemptionPriority = 0x02; // set priority level
  NVIC_InitStructure_button.NVIC_IRQChannelSubPriority = 0x02;
  NVIC_InitStructure_button.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure_button);
}

button_state_type button_read(void)
{
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
	{			
		return BUTTON_DOWN;
	}
	else
	{
		return BUTTON_UP;
	}
}
