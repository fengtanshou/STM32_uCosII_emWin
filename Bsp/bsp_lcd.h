
#ifndef _LCD_H_
#define _LCD_H_

#define NS100_DLY(x) 		((unsigned int)(20*x))
#define LCD_Delay(x) 		WhileDly(x)


#define LCD_DATA1_PORT		GPIOG
#define LCD_DATA1_PIN		GPIO_Pin_0
#define LCD_DATA2_PORT		GPIOG
#define LCD_DATA2_PIN		GPIO_Pin_1
#define LCD_DATA3_PORT		GPIOG
#define LCD_DATA3_PIN		GPIO_Pin_2
#define LCD_DATA4_PORT		GPIOG
#define LCD_DATA4_PIN		GPIO_Pin_3
#define LCD_DATA5_PORT		GPIOG
#define LCD_DATA5_PIN		GPIO_Pin_4
#define LCD_DATA6_PORT		GPIOG
#define LCD_DATA6_PIN		GPIO_Pin_5
#define LCD_DATA7_PORT		GPIOG
#define LCD_DATA7_PIN		GPIO_Pin_6
#define LCD_DATA8_PORT		GPIOG
#define LCD_DATA8_PIN		GPIO_Pin_7

#define LCD_WR_PORT			GPIOG
#define LCD_WR_PIN			GPIO_Pin_8
#define LCD_RD_PORT			GPIOG
#define LCD_RD_PIN			GPIO_Pin_9
#define LCD_CE_PORT			GPIOG
#define LCD_CE_PIN			GPIO_Pin_10
#define LCD_CD_PORT			GPIOG
#define LCD_CD_PIN			GPIO_Pin_11
#define LCD_RST_PORT		GPIOG
#define LCD_RST_PIN			GPIO_Pin_12
#define LCD_FS_PORT			GPIOG
#define LCD_FS_PIN			GPIO_Pin_13
#define LCD_LED_PORT		GPIOG
#define LCD_LED_PIN			GPIO_Pin_15
#define LCD_AD_CTRL_PORT	GPIOC
#define LCD_AD_CTRL_PIN		GPIO_Pin_8


#define MAKE_COLOR(x)	((x) ^ m_Color)


/****************************************************************************
* ��	�ƣ�
* ��	�ܣ�GUI�õ��ĺ���
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/
void LCD_WriteCommand(unsigned char nCmd);
void LCD_WriteData(unsigned char nData);
void LCD_Initilation(void);
void SetAddr(unsigned int x, unsigned int y, unsigned int width, unsigned int high);
void Draw_Dot(int x,int y);
void Draw_ClrDot(int x, int y);

void LCD_ShowBMP(int x, int y, int width, int high, const unsigned char bmp[]);

void LCD_DrawLine(int x1,int y1,int x2,int y2, int Index1);
void LCD_SetPoint(u32 x, u32 y, u32 Index1);
void LCDRAM(uint16_t dp);
void LCD_SetCursor(u32 x, u32 y);

void LCD_SendBuffDC(int x, int y, int xl, int yl);
void Draw_LineToDispBuff(int xpos, int ypos, int xlen, int ylen);
void SetColor(u32 nColor);

/*
����ΪLCM��������,��Ҫ������T6963C�ĸ�������,�ṩ������ʾ��ַ�ȹ���  
�ڷ�������ǰ������״̬��  
����������ķ���ģʽ:   �Ȳ���->������;  
            ����ģʽ:   ������->�����;  
  
T6963C����Ķ���  
********************************************************************/   
//ָ������ָ��    
#define LCD_CUR_POS 0X21    //���λ������(ֻ�����õ���Ч��ʾ��ַ������ʾ�ſ���),˫����(D1ˮƽλ��,D2��ֱλ��)    
#define LCD_CGR_POS 0X22    //CGRAMƫ�õ�ַ����(���������Լ��ķ���),˫����(D1��5λ��Ч,D2=00)    
#define LCD_ADR_POS 0x24    //��ַָ��λ��(���ö�д��������)    

//��ʾ��������ָ��    
#define LCD_TXT_STP 0X40    //�ı�����ַ(�Ӵ˵�ַ��ʼ����Ļ���Ͻ���ʾ�ַ�)    
#define LCD_TXT_WID 0X41    //�ı������(������ʾ���,N/6ʽN/8,����NΪX��ĵ���)    
#define LCD_GRH_STP 0X42    //ͼ������ַ(�Ӵ˵�ַ��ʼ����Ļ���Ͻ���ʾ��,((�еȽ�һ��ȷ��))Ϊ�ı���ʽ��ʾʱ,��ʾΪ�ı���ʾ����    
#define LCD_GRH_WID 0X43    //ͼ�������(������ʾ���,N/6ʽN/8,����NΪX��ĵ���)    

//��ʾ��ʽ����ָ��,��CG(D3)Ϊ0����CGROM    
#define LCD_MOD_OR  0X80    //��ʾ��ʽ:�߼���    
#define LCD_MOD_XOR 0X81    //��ʾ��ʽ:�߼����    
#define LCD_MOD_AND 0x83    //��ʾ��ʽ:�߼���    
#define LCD_MOD_TCH 0x84    //��ʾ��ʽ:�ı�    

//��ʾ��������ָ��,(D0,D1,D2,D3)=1/0  ==========>  (�����˸,�����ʾ,�ı���ʾ,ͼ����ʾ)����/����    
#define LCD_DIS_SW  0x90    //��ʾ��������
#define LCD_DIS_OFF	0xAC	//����ʾ	UC1698
#define LCD_DIS_GON	0xAD	//��ͼ��	UC1698
#define LCD_DIS_TON	0x94	//���ı�    

//��꿪ͷ����ָ��    
#define LCD_CUR_SHP 0xA0    //�������״ѡ��:0xA0-0xA7������ռ������    

//�Զ���д����ָ��    
#define LCD_AUT_WR  0xB0    //�Զ�д����    
#define LCD_AUT_RD  0xB1    //�Զ�������    
#define LCD_AUT_OVR 0xB2    //�Զ���д����    

//����һ�ζ�д����ָ��    
#define LCD_INC_WR  0xC0    //����һ��д,��ַ��1    
#define LCD_INC_RD  0xC1    //����һ�ζ�,��ַ��1    
#define LCD_DEC_WR  0xC2    //����һ��д,��ַ��1    
#define LCD_DEC_RD  0xC3    //����һ�ζ�,��ַ��1    
#define LCD_ZER_WR  0xc4    //����һ��д,��ַ����    
#define LCE_ZER_RD  0xc5    //����һ�ζ�,��ַ����    

//��������ָ��    
#define LCD_SCN_RD  0xE0    //����,����ֻ����ͼ����ʾ����;��ָ���ǰ����ϵ�����ȡ������Ϊ����ʹ��.    

//������    
#define LCD_SCN_CP  0xE8    //������,��ָ����ϵ�ǰ��ַָ��(ͼ��)�������һ�кϳ���ʾ���ݿ��������Ӧ��ͼ����ʾ����һ����Ԫ    

//λ����ָ��    
#define LCD_BIT_OP  0xF0    //��ָ��ɽ���ʾ���������޵�ĳλ�������λ.D0-D1---����D0-D7λ;D3--1Ϊ��λ,D3--0Ϊ��λ   



/****************************************************************************
* ��	�ƣ�u8 fLCD_ReadLEDstate(void)
* ��	�ܣ���ȡ��ǰLCD�����״̬
* ��ڲ�������
* ���ڲ�����0	�ر�
						1	����
* ˵	����
****************************************************************************/
unsigned char fLCD_ReadLEDstate(void);

/****************************************************************************
* ��	�ƣ�void fLCD_LEDON(void)
* ��	�ܣ�����LCD����
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/
void fLCD_LEDON(void);

/****************************************************************************
* ��	�ƣ�void fLCD_LEDOFF(void)
* ��	�ܣ��ر�LCD����
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/
void fLCD_LEDOFF(void);
/****************************************************************************
* ��	�ƣ�void ClearScreen(void)
* ��	�ܣ������Ļ
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/
void ClearScreen(void);

#endif
