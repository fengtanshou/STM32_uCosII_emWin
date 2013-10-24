#ifndef _LED_H_	
#define _LED_H_

#ifdef __cplusplus		   //�����CPP����C���� //��ʼ����
extern "C" {
#endif

	
//LED�ܽŶ���	
#define LED4_PORT		GPIOD
#define LED4_PIN		GPIO_Pin_14	


	
	

//����������

#define LED4_ON 				GPIO_ResetBits(LED4_PORT, LED4_PIN)			
#define LED4_OFF 				  GPIO_SetBits(LED4_PORT, LED4_PIN)	
#define	ReadLED4State()			(GPIO_ReadOutputDataBit(LED4_PORT,LED4_PIN))
	
	
	
	
//��ʼ��
extern void LED_Init(void);												//����
extern void LED_Tog(void);

	
	
	
	

#ifdef __cplusplus		   //�����CPP����C���� //��������
}
#endif

#endif
