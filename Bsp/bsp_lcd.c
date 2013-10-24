#include "stm32f4xx.h"
#include "bsp_lcd.h"



#define LCD_BYTES_X  			30 			//��ʾ�����
#define TEXT_HOME_ADDRESS 		0x0000 		//�ı���ʾ���׵�ַ
#define GRAPHIC_HOME_ADDRESS 	0x01E0 		//ͼ����ʾ���׵�ַ

#define	GPIO_SET(x)		{LCD_##x##_PORT -> BSRRL = LCD_##x##_PIN;}
#define	GPIO_RESET(x)	{LCD_##x##_PORT -> BSRRH = LCD_##x##_PIN;}

#define	LCD_WRITE_DATA(data)	LCD_DATA1_PORT -> BSRRH = 0x00FF;LCD_DATA1_PORT -> BSRRL = (data) ;		//д����
#define	LCD_SET_DATA_OUT()		LCD_DATA1_PORT->MODER |= 0x00005555; 									//����������Ϊ���ģʽ
#define	LCD_SET_DATA_IN()	    LCD_DATA1_PORT->MODER &= 0xffff0000; 									//����������Ϊ����ģʽ

#define SdCmd	LCD_WriteCommand
#define SdData	LCD_WriteData

#define	RdData


static u8 m_Color = 0xff;
static u8 DispBuff[128][120];


/****************************************************************************
* ��	�ƣ�u8 fLCD_ReadLEDstate(void)
* ��	�ܣ�������״̬
* ��ڲ�����
* ���ڲ�����u8	1:�����Ѵ򿪣�	0�������ѹر�
* ˵	������
****************************************************************************/
u8 fLCD_ReadLEDstate(void)
{
	return GPIO_ReadOutputDataBit(LCD_LED_PORT , LCD_LED_PIN);
}
/****************************************************************************
* ��	�ƣ�void fLCD_LEDON(void)
* ��	�ܣ���LCD����
* ��ڲ�����
* ���ڲ�����
* ˵	������
****************************************************************************/
void fLCD_LEDON(void)
{
	GPIO_SET(LED);
}
/****************************************************************************
* ��	�ƣ�void fLCD_LEDOFF(void)
* ��	�ܣ��ر�LCD����
* ��ڲ�����
* ���ڲ�����
* ˵	������
****************************************************************************/
void fLCD_LEDOFF(void)
{
	GPIO_RESET(LED);
}


static void WhileDly(u32 nDly)
{
	while(nDly--);
}

static void LCD_PortInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);		//��������ʱ��G
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);		//��������ʱ��G

 	//���ö�д����Ϊ�������
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;  
	GPIO_InitStructure.GPIO_Pin   = LCD_WR_PIN|LCD_RD_PIN|LCD_CE_PIN|LCD_CD_PIN|LCD_RST_PIN|LCD_FS_PIN|LCD_LED_PIN/
									LCD_DATA1_PIN|LCD_DATA2_PIN|LCD_DATA3_PIN|LCD_DATA4_PIN|LCD_DATA5_PIN|LCD_DATA6_PIN|LCD_DATA7_PIN|LCD_DATA8_PIN;		//LCD_WR
	GPIO_Init(LCD_WR_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin   = LCD_AD_CTRL_PIN ;					
	GPIO_Init(LCD_AD_CTRL_PORT, &GPIO_InitStructure);	
					
	GPIO_RESET(RST);
	
	GPIO_SET(CE);
	GPIO_SET(WR);
	GPIO_SET(AD_CTRL);
	GPIO_SET(RD);
	GPIO_SET(CD);
	GPIO_RESET(FS);		//ʹ��8*8ģʽ
	GPIO_SET(LED);		//�򿪱���
}

/****************************************************************************
* ��	�ƣ�	void LCD_WriteCommand(uint8 nCmd)
* ��	�ܣ�	дLCD����
* ��ڲ�����u8	����
* ���ڲ�������
* ˵	����	��
****************************************************************************/
void LCD_WriteCommand(unsigned char nCmd)
{
	LCD_SET_DATA_OUT()				/* �������������Ϊ�����ʽ */
	GPIO_RESET(CE);
	GPIO_RESET(CD);
	GPIO_RESET(WR);
	LCD_WRITE_DATA(nCmd);			//��MCUд�����ݣ�׼����LCD�������
	WhileDly(2);
	GPIO_SET(WR);
	GPIO_SET(CE);
	LCD_SET_DATA_IN();				/* �������������Ϊ���뷽ʽ */
}

void LCD_WriteData(unsigned char nData)
{
	LCD_SET_DATA_OUT();				/* �������������Ϊ�����ʽ */
	GPIO_RESET(CE);					//GPIO_RESET(CE);                               
	GPIO_SET(CD);                   //GPIO_RESET(CD);                   
	GPIO_RESET(WR);                 //GPIO_RESET(WR);                        
	LCD_WRITE_DATA(nData);		    //LCD_WRITE_DATA(nCmd);	
	WhileDly(2);         			//WhileDly(NS100_DLY(4));                          
	GPIO_SET(WR);                   //GPIO_SET(WR);
	GPIO_SET(CE);
	LCD_SET_DATA_IN();				/* �������������Ϊ���뷽ʽ */
}


unsigned char Rd_Data(void)  
{
	unsigned char Ddata;
	
	LCD_SET_DATA_IN();
	GPIO_SET(CD); 
	GPIO_RESET(CE);
	GPIO_RESET(RD);
	Ddata = GPIO_ReadInputData(GPIOG);
	GPIO_SET(RD);
	GPIO_SET(CE);
	
    return (Ddata);
}

void LCD_SetCursor(u32 x, u32 y)
{
	int m;
	
	m=x/3;
	SdCmd(0xf4); SdCmd(m);                        // ���ò���������߽�
	SdCmd(0xf5); SdCmd(y);                        // ���ò��������ϱ߽�
	SdCmd(0xf6); SdCmd(m);                        // ���ò��������ұ߽�
	SdCmd(0xf7); SdCmd(y);                        // ���ò��������±߽�
	SdCmd(0xf8);			  	         			 // ���ô��ڲ���ʹ��
	SdCmd(0xd6);					 				 // ��������λΪ16λ
	SdCmd(m&0x0f); SdCmd(0x10|(m>>4));            // ������ʼ�е�ַ
	SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));     // ������ʼ�е�ַ
}



void LCDRAM(uint16_t dp)
{

}

//-----��㺯��--------------------------
// ����(x,y)��xΪˮƽ�������ص��У�yΪ��ֱ�������ص���
void Draw_Dot(int x, int y)
{
	int k1,k2,m;
	
	LCD_SetCursor(x, y);
	k1 = Rd_Data();                                  // �ն�
	k1 = Rd_Data();                                  // ������
	k2 = Rd_Data();                                  // ������
	
	m=x%3;
	switch (m) {
		case 0:   SdData(k1|0xf8);SdData(k2);		break;   // д��
		case 1:   SdData(k1|0x07);SdData(k2|0xe0);	break;
		case 2:   SdData(k1);SdData(k2|0x1f);		break;
	}
	SdCmd(0xd5);					  				// �ָ���������λΪ12λ
}

void Draw_ClrDot(int x, int y)
// ����(x,y)��xΪˮƽ�������ص��У�yΪ��ֱ�������ص���
{
	int k1,k2,m;
	
	LCD_SetCursor(x, y);
	k1 = Rd_Data();                                  // �ն�
	k1 = Rd_Data();                                  // ������
	k2 = Rd_Data();                                  // ������
	
	m=x%3;
	switch (m) {
		case 0:   SdData(k1&0x07);SdData(k2);		break;   // д��
		case 1:   SdData(k1&0xF8);SdData(k2&0x1F);	break;
		case 2:   SdData(k1);SdData(k2&0xe0);		break;
	}
	SdCmd(0xd5);					  // �ָ���������λΪ12λ
}

void LCD_SetPoint(u32 x, u32 y, u32 Index1)
{
	if (Index1) {
		Draw_ClrDot(x, y);
	} else {
		Draw_Dot(x, y);
	}
}

void LCD_DrawLine(int x1,int y1,int x2,int y2, int Index1)
//  xΪˮƽ���������У�yΪ��ֱ�������ص���
//  ����(x1,y1)Ϊ����ʼ��ַ���ꣻ����(x2,y2)Ϊ����ֹ��ַ���ꡣ
{
  u32 temp;
  int dalt_x,dalt_y,err=0;
	
	if (y1>y2)
    {
		temp=x1;
		x1=x2;
		x2=temp;
		temp=y1;
		y1=y2;
		y2=temp;
    }
	LCD_SetPoint(x1,y1,Index1);
	dalt_x=x2-x1;
	dalt_y=y2-y1;
	if(dalt_x>=0)
     {
      if(dalt_y>dalt_x)//k>1
        {
         while(y1<y2)
         {
          if(err<0)
          {
           x1=x1+1;
           y1=y1+1;
           err=err+dalt_y-dalt_x;
           }
           else
           {
           y1=y1+1;
           err=err-dalt_x;
           }
          LCD_SetPoint(x1,y1,Index1);
         }
       }
     else  // 0<=k=<1
      {
       if (dalt_y==0)
          y1=y1-1;
       while(x1<x2)
         {
          if(err<0)
          {
           x1=x1+1;
           err=err+dalt_y;
           }
           else
           {
           y1=y1+1;
           x1=x1+1;
           err=err+dalt_y-dalt_x;
           }
          LCD_SetPoint(x1,y1,Index1);
         }
     }
     
  }
else
  { 
   dalt_x=x1-x2;  
    if(dalt_y>dalt_x)//k<-1
     {
      while(y1<y2)
       {
        if(err<0)
         {
          x1=x1-1;
          y1=y1+1;
          err=err+dalt_y-dalt_x;
         }
        else
         {
          y1=y1+1;
          err=err-dalt_x;
         }
        LCD_SetPoint(x1,y1,Index1);
      }
    }
   else       //0>k>=-1
    {
      if (dalt_y==0)
          y1=y1-1; 
    while(x1>x2)
      {
       if(err<0)
        {
         x1=x1-1;
         err=err+dalt_y;
        }
       else
        {
         x1=x1-1;
         y1=y1+1;
         err=err+dalt_y-dalt_x;
        }
       LCD_SetPoint(x1,y1,Index1);
      }
    } 
  }
     
}

void SetColor(u32 nColor)
{
	m_Color = (nColor == 0) ? 0 : 0xFF;
}
/*
 *	ˢ��ָ�����ڻ���
 */
void LCD_SendBuffDC(int x, int y, int xl, int yl)
{
	int i, j;
	int sx;
	int ex, ey;
	int mod, len;	
	
	mod = x % 6;				/* ���������һ��6������ַ�ľ��� */
	x -= mod;					/* ���������һ��6������ַ�ľ��� */
	sx = x / 3;					/* ����Һ�����ڵĵ�ַ */
	xl += mod;					/* �������� */
	ex = (xl + x) - 1;			/* ����ʼ����ͳ���ת��Ϊ��������x */
	mod = ex % 6;				/* ���������ַ������ */
	ex = ex + (6 - mod) - 1;	/* ���x�Ľ�����ַ��������ַ��6������ */
	len = (ex + 1) / 2;			/* ����ʼ��ַһֱˢ��������ַ */		
 	ey = (yl + y) - 1;			/* ����y��Ľ������� */

	SdCmd(0xf4); SdCmd(sx);                   		// ���ò���������߽�
	SdCmd(0xf5); SdCmd(y);                   		// ���ò��������ϱ߽� 
	SdCmd(0xf6); SdCmd(ex/3);             			// ���ò��������ұ߽�
	SdCmd(0xf7); SdCmd(ey);              			// ���ò��������±߽�
	SdCmd(0xf8);		                    		// ���ô��ڲ���ʹ��
	SdCmd(sx&0x0f);SdCmd(0x10|(sx>>4));        		// ������ʼ�е�ַ
	SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));		// ������ʼ�е�ַ	
	
	for (i = y; i <= ey; i++) {
		for (j = x/2; j < len; j++) {
			SdData(DispBuff[i][j]);
		}
	}	
}

void Draw_LineToDispBuff(int xpos, int ypos, int xlen, int ylen)
{
	int i, j;
	int sx;
	int odd,num;
	unsigned char buff;
	unsigned char sbu;		
	
	sx = xpos;
	buff = 0xff;
	for (i = ypos; i < (ypos + ylen); i++) {			
		for (j = xpos; j < (xpos + xlen); j++) {
			if (((j - sx) % 8) == 0) {
				buff = 0xFF;
			}	
			num = (j / 2);
			odd = (j % 2);
			if (odd == 0) {											/* �������Ϊ0��ʾ���ֽڵĸ߰��ֽ� */
				sbu = DispBuff[i][num] & 0x0F;	
				if (buff & 0x80) {
					DispBuff[i][num] = (MAKE_COLOR(DispBuff[i][num] | 0xF0) & 0xF0) | (sbu);
				} else {
					DispBuff[i][num] = (MAKE_COLOR(DispBuff[i][num] & 0x0F) & 0xF0) | (sbu);
				}	
			} else {												/* �������Ϊ1��ʾ���ֽڵĵͰ��ֽ� */
				sbu = DispBuff[i][num] & 0xF0;
				if (buff & 0x80) {
					DispBuff[i][num] = (MAKE_COLOR(DispBuff[i][num] | 0x0F) & 0x0F) | (sbu);
				} else {
					DispBuff[i][num] = (MAKE_COLOR(DispBuff[i][num] & 0xF0) & 0x0F) | (sbu);
				}			
			}	
			buff <<= 1;
		}
	}	
}
/****************************************************************************
* ��	�ƣ�void LCD_Init(void)
* ��	�ܣ���ʼ��LCD
* ��ڲ�������
* ���ڲ�����
* ˵	������
****************************************************************************/
void LCD_Initilation(void)
{
	//LCD�ܽų�ʼ��
	LCD_PortInit();	

    GPIO_RESET(RST); LCD_Delay(NS100_DLY(200)); 	//Ӳ����λ
    GPIO_SET(RST); 	LCD_Delay(NS100_DLY(500000)); 	//��λ���ӳ�800ms����ʱ��

    LCD_WriteCommand(0x25);                           	// �����¶Ȳ���ϵ��-0.05%/C
    LCD_WriteCommand(0x2b);                           	// �ڲ�DC-DC
	LCD_WriteCommand(0xc4);                          	// LCDӳ��MY=1��MX=0��LC0=0  c4
	LCD_WriteCommand(0xa3);                        		// ������ɨ��Ƶ��  fr=76.34hz a1
	LCD_WriteCommand(0xd1);                          	// ��ɫ���ݸ�ʽR-G-B
	LCD_WriteCommand(0xd5);								// ��������λΪ12λRRRR-GGGG-BBBB
	LCD_WriteCommand(0xc8);	LCD_WriteCommand(0x3f);    	// ����M�ź�Ϊ19�з�ת 11
	LCD_WriteCommand(0xec);                            	// ����ƫѹ��1/12
	LCD_WriteCommand(0xa6);                            	// ������ʾ
	LCD_WriteCommand(0xa4);                            	// ������ʾ
	LCD_WriteCommand(0x81);	LCD_WriteCommand(0x4f);   	//���öԱȶ�bf
	LCD_WriteCommand(0xd8);                            	// ����ɨ��ģʽd8

    LCD_WriteCommand(0xf1);	LCD_WriteCommand(0x9f);    	//������CEN,159
    LCD_WriteCommand(0xf2);	LCD_WriteCommand(0x20);    	//��ʼDST
    LCD_WriteCommand(0xf3);	LCD_WriteCommand(0x9f);    	//����DEN
	LCD_WriteCommand(0x84); 
	LCD_WriteCommand(0xa9); 
	
	LCD_Delay(NS100_DLY(800));// ����ʾad
	
}

