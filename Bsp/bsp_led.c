#include "stm32f4xx.h"
#include "bsp_led.h"



/****************************************************************************
* 名	称：void fIO_Init(void)
* 功	能：初始化IO
* 入口参数：无
* 出口参数：无
* 说	明：
****************************************************************************/	
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//配置输出管脚
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);	 	//输入输出引脚时钟	
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;  
	GPIO_InitStructure.GPIO_Pin   = LED4_PIN;
	GPIO_Init(LED4_PORT, &GPIO_InitStructure);	


	LED4_OFF;

}

void LED_Tog(void)
{
  	GPIO_WriteBit(LED4_PORT, LED4_PIN, !GPIO_ReadOutputDataBit(LED4_PORT, LED4_PIN));
}


