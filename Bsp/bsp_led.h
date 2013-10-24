#ifndef _LED_H_	
#define _LED_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

	
//LED管脚定义	
#define LED4_PORT		GPIOD
#define LED4_PIN		GPIO_Pin_14	


	
	

//引脚上下拉

#define LED4_ON 				GPIO_ResetBits(LED4_PORT, LED4_PIN)			
#define LED4_OFF 				  GPIO_SetBits(LED4_PORT, LED4_PIN)	
#define	ReadLED4State()			(GPIO_ReadOutputDataBit(LED4_PORT,LED4_PIN))
	
	
	
	
//初始化
extern void LED_Init(void);												//设置
extern void LED_Tog(void);

	
	
	
	

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif
