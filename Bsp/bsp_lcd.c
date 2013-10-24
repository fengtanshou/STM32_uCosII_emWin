#include "stm32f4xx.h"
#include "bsp_lcd.h"



#define LCD_BYTES_X  			30 			//显示区宽度
#define TEXT_HOME_ADDRESS 		0x0000 		//文本显示区首地址
#define GRAPHIC_HOME_ADDRESS 	0x01E0 		//图形显示区首地址

#define	GPIO_SET(x)		{LCD_##x##_PORT -> BSRRL = LCD_##x##_PIN;}
#define	GPIO_RESET(x)	{LCD_##x##_PORT -> BSRRH = LCD_##x##_PIN;}

#define	LCD_WRITE_DATA(data)	LCD_DATA1_PORT -> BSRRH = 0x00FF;LCD_DATA1_PORT -> BSRRL = (data) ;		//写数据
#define	LCD_SET_DATA_OUT()		LCD_DATA1_PORT->MODER |= 0x00005555; 									//数据线设置为输出模式
#define	LCD_SET_DATA_IN()	    LCD_DATA1_PORT->MODER &= 0xffff0000; 									//数据线设置为输入模式

#define SdCmd	LCD_WriteCommand
#define SdData	LCD_WriteData

#define	RdData


static u8 m_Color = 0xff;
static u8 DispBuff[128][120];


/****************************************************************************
* 名	称：u8 fLCD_ReadLEDstate(void)
* 功	能：读背光状态
* 入口参数：
* 出口参数：u8	1:背光已打开；	0：背光已关闭
* 说	明：无
****************************************************************************/
u8 fLCD_ReadLEDstate(void)
{
	return GPIO_ReadOutputDataBit(LCD_LED_PORT , LCD_LED_PIN);
}
/****************************************************************************
* 名	称：void fLCD_LEDON(void)
* 功	能：打开LCD背光
* 入口参数：
* 出口参数：
* 说	明：无
****************************************************************************/
void fLCD_LEDON(void)
{
	GPIO_SET(LED);
}
/****************************************************************************
* 名	称：void fLCD_LEDOFF(void)
* 功	能：关闭LCD背光
* 入口参数：
* 出口参数：
* 说	明：无
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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);		//开启引脚时钟G
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);		//开启引脚时钟G

 	//配置读写控制为推挽输出
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
	GPIO_RESET(FS);		//使用8*8模式
	GPIO_SET(LED);		//打开背光
}

/****************************************************************************
* 名	称：	void LCD_WriteCommand(uint8 nCmd)
* 功	能：	写LCD命令
* 入口参数：u8	命令
* 出口参数：无
* 说	明：	无
****************************************************************************/
void LCD_WriteCommand(unsigned char nCmd)
{
	LCD_SET_DATA_OUT()				/* 将数据输出口设为输出方式 */
	GPIO_RESET(CE);
	GPIO_RESET(CD);
	GPIO_RESET(WR);
	LCD_WRITE_DATA(nCmd);			//向MCU写入数据，准备向LCD输出数据
	WhileDly(2);
	GPIO_SET(WR);
	GPIO_SET(CE);
	LCD_SET_DATA_IN();				/* 将数据输出口设为输入方式 */
}

void LCD_WriteData(unsigned char nData)
{
	LCD_SET_DATA_OUT();				/* 将数据输出口设为输出方式 */
	GPIO_RESET(CE);					//GPIO_RESET(CE);                               
	GPIO_SET(CD);                   //GPIO_RESET(CD);                   
	GPIO_RESET(WR);                 //GPIO_RESET(WR);                        
	LCD_WRITE_DATA(nData);		    //LCD_WRITE_DATA(nCmd);	
	WhileDly(2);         			//WhileDly(NS100_DLY(4));                          
	GPIO_SET(WR);                   //GPIO_SET(WR);
	GPIO_SET(CE);
	LCD_SET_DATA_IN();				/* 将数据输出口设为输入方式 */
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
	SdCmd(0xf4); SdCmd(m);                        // 设置操作窗口左边界
	SdCmd(0xf5); SdCmd(y);                        // 设置操作窗口上边界
	SdCmd(0xf6); SdCmd(m);                        // 设置操作窗口右边界
	SdCmd(0xf7); SdCmd(y);                        // 设置操作窗口下边界
	SdCmd(0xf8);			  	         			 // 设置窗口操作使能
	SdCmd(0xd6);					 				 // 设置数据位为16位
	SdCmd(m&0x0f); SdCmd(0x10|(m>>4));            // 设置起始列地址
	SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));     // 设置起始行地址
}



void LCDRAM(uint16_t dp)
{

}

//-----绘点函数--------------------------
// 坐标(x,y)，x为水平方向像素点列；y为垂直方向像素点行
void Draw_Dot(int x, int y)
{
	int k1,k2,m;
	
	LCD_SetCursor(x, y);
	k1 = Rd_Data();                                  // 空读
	k1 = Rd_Data();                                  // 读数据
	k2 = Rd_Data();                                  // 读数据
	
	m=x%3;
	switch (m) {
		case 0:   SdData(k1|0xf8);SdData(k2);		break;   // 写点
		case 1:   SdData(k1|0x07);SdData(k2|0xe0);	break;
		case 2:   SdData(k1);SdData(k2|0x1f);		break;
	}
	SdCmd(0xd5);					  				// 恢复设置数据位为12位
}

void Draw_ClrDot(int x, int y)
// 坐标(x,y)，x为水平方向像素点列；y为垂直方向像素点行
{
	int k1,k2,m;
	
	LCD_SetCursor(x, y);
	k1 = Rd_Data();                                  // 空读
	k1 = Rd_Data();                                  // 读数据
	k2 = Rd_Data();                                  // 读数据
	
	m=x%3;
	switch (m) {
		case 0:   SdData(k1&0x07);SdData(k2);		break;   // 写点
		case 1:   SdData(k1&0xF8);SdData(k2&0x1F);	break;
		case 2:   SdData(k1);SdData(k2&0xe0);		break;
	}
	SdCmd(0xd5);					  // 恢复设置数据位为12位
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
//  x为水平方向像素列；y为垂直方向像素点行
//  坐标(x1,y1)为线起始地址坐标；坐标(x2,y2)为线终止地址坐标。
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
 *	刷新指定窗口缓存
 */
void LCD_SendBuffDC(int x, int y, int xl, int yl)
{
	int i, j;
	int sx;
	int ex, ey;
	int mod, len;	
	
	mod = x % 6;				/* 计算距离上一个6倍数地址的距离 */
	x -= mod;					/* 计算距离上一个6倍数地址的距离 */
	sx = x / 3;					/* 计算液晶屏内的地址 */
	xl += mod;					/* 长度增加 */
	ex = (xl + x) - 1;			/* 将起始坐标和长度转换为结束坐标x */
	mod = ex % 6;				/* 计算结束地址的余数 */
	ex = ex + (6 - mod) - 1;	/* 算出x的结束地址，结束地址是6个倍数 */
	len = (ex + 1) / 2;			/* 从起始地址一直刷到结束地址 */		
 	ey = (yl + y) - 1;			/* 计算y轴的结束坐标 */

	SdCmd(0xf4); SdCmd(sx);                   		// 设置操作窗口左边界
	SdCmd(0xf5); SdCmd(y);                   		// 设置操作窗口上边界 
	SdCmd(0xf6); SdCmd(ex/3);             			// 设置操作窗口右边界
	SdCmd(0xf7); SdCmd(ey);              			// 设置操作窗口下边界
	SdCmd(0xf8);		                    		// 设置窗口操作使能
	SdCmd(sx&0x0f);SdCmd(0x10|(sx>>4));        		// 设置起始列地址
	SdCmd(0x60+(y&0x0f)); SdCmd(0x70+(y>>4));		// 设置起始行地址	
	
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
			if (odd == 0) {											/* 如果余数为0表示在字节的高半字节 */
				sbu = DispBuff[i][num] & 0x0F;	
				if (buff & 0x80) {
					DispBuff[i][num] = (MAKE_COLOR(DispBuff[i][num] | 0xF0) & 0xF0) | (sbu);
				} else {
					DispBuff[i][num] = (MAKE_COLOR(DispBuff[i][num] & 0x0F) & 0xF0) | (sbu);
				}	
			} else {												/* 如果余数为1表示在字节的低半字节 */
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
* 名	称：void LCD_Init(void)
* 功	能：初始化LCD
* 入口参数：无
* 出口参数：
* 说	明：无
****************************************************************************/
void LCD_Initilation(void)
{
	//LCD管脚初始化
	LCD_PortInit();	

    GPIO_RESET(RST); LCD_Delay(NS100_DLY(200)); 	//硬件复位
    GPIO_SET(RST); 	LCD_Delay(NS100_DLY(500000)); 	//复位后延迟800ms以上时间

    LCD_WriteCommand(0x25);                           	// 设置温度补偿系数-0.05%/C
    LCD_WriteCommand(0x2b);                           	// 内部DC-DC
	LCD_WriteCommand(0xc4);                          	// LCD映像MY=1，MX=0，LC0=0  c4
	LCD_WriteCommand(0xa3);                        		// 设置行扫描频率  fr=76.34hz a1
	LCD_WriteCommand(0xd1);                          	// 彩色数据格式R-G-B
	LCD_WriteCommand(0xd5);								// 设置数据位为12位RRRR-GGGG-BBBB
	LCD_WriteCommand(0xc8);	LCD_WriteCommand(0x3f);    	// 设置M信号为19行翻转 11
	LCD_WriteCommand(0xec);                            	// 设置偏压比1/12
	LCD_WriteCommand(0xa6);                            	// 正性显示
	LCD_WriteCommand(0xa4);                            	// 正常显示
	LCD_WriteCommand(0x81);	LCD_WriteCommand(0x4f);   	//设置对比度bf
	LCD_WriteCommand(0xd8);                            	// 设置扫描模式d8

    LCD_WriteCommand(0xf1);	LCD_WriteCommand(0x9f);    	//结束行CEN,159
    LCD_WriteCommand(0xf2);	LCD_WriteCommand(0x20);    	//起始DST
    LCD_WriteCommand(0xf3);	LCD_WriteCommand(0x9f);    	//结束DEN
	LCD_WriteCommand(0x84); 
	LCD_WriteCommand(0xa9); 
	
	LCD_Delay(NS100_DLY(800));// 开显示ad
	
}

