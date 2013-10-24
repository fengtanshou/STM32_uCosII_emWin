#include "stm32f4xx.h"
#include "bsp_led.h"



/****************************************************************************
* ��	�ƣ�void fIO_Init(void)
* ��	�ܣ���ʼ��IO
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/	
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//��������ܽ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);	 	//�����������ʱ��	
	
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


