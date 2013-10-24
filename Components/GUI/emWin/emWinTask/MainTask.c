/*
*********************************************************************************************************
*	                                  
*	ģ������ : GUI����������
*	�ļ����� : MainTask.c
*	��    �� : V1.0
*	˵    �� : ������ӱȽϼ򵥣���Ҫ��ѧϰ������ˮƽ��ɫ�ݶ����ľ���
*	�޸ļ�¼ :
*		�汾��    ����          ����                 ˵��
*		v1.0    2013-04-17    Eric2013      ST�̼���汾 V1.0.2�汾��
*
*	Copyright (C), 2013-2014
*   QQ����Ⱥ��216681322
*   BLOG: http://blog.sina.com.cn/u/2565749395 
*********************************************************************************************************
*/

#include "includes.h"
#include "MainTask.h"


/*******************************************************************
*
*       Defines
*
********************************************************************
*/
#define X_START 60
#define Y_START 40

/*******************************************************************
*
*       Types
*
********************************************************************
*/
typedef struct {
  int NumBars;
  GUI_COLOR Color;
  const char * s;
} BAR_DATA;

/*******************************************************************
*
*       Static data
*
********************************************************************
*/
static const BAR_DATA _aBarData[] = {
  { 2, GUI_RED    , "Red" },
  { 2, GUI_GREEN  , "Green" },
  { 2, GUI_BLUE   , "Blue" },
  { 1, GUI_WHITE  , "Grey" },
  { 2, GUI_YELLOW , "Yellow" },
  { 2, GUI_CYAN   , "Cyan" },
  { 2, GUI_MAGENTA, "Magenta" },
};

static const GUI_COLOR _aColorStart[] = { GUI_BLACK, GUI_WHITE };

/*******************************************************************
*
*       Static code
*
********************************************************************
*/
/*******************************************************************
*
*       _DemoShowColorBar
*/
static void _DemoShowColorBar(void) {
  int yStep, i, j, xSize, ySize, NumBars, NumColors;
  GUI_RECT Rect;

  //
  // Get display size
  //
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();

  //
  // Get number of bars
  // �õ�Ҫ��ʾ��ɫ����Ŀ
  //
  NumColors = GUI_COUNTOF(_aBarData);
  for (i = NumBars = 0, NumBars = 0; i < NumColors; i++) {
    NumBars += _aBarData[i].NumBars;
  }

  yStep = (ySize - Y_START) / NumBars;

  //
  // ��ʾ����
  //
  Rect.x0 = 0;
  Rect.x1 = X_START - 1;
  Rect.y0 = Y_START;
  GUI_SetFont(&GUI_Font8x16);
  for (i = 0; i < NumColors; i++) {
    Rect.y1 = Rect.y0 + yStep * _aBarData[i].NumBars - 1;
	//�ڵ�ǰ����ָ���ľ��������ڵ�ָ��λ�ô���ʹ�õ�ǰ������ʾ��Ϊ�������ַ���
	//���ָ���ľ�������̫С���ı��ᱻ�ü���
    GUI_DispStringInRect(_aBarData[i].s, 	 //��ʾ���ַ���
	                      &Rect, 			 //Ҫд��Ŀͻ��˴��ھ������򣨵�λ�����أ���
						  GUI_TA_LEFT | GUI_TA_VCENTER); //�����ǣ�����ͨ����OR�������������
    Rect.y0 = Rect.y1 + 1;
  }
  //
  // ����ɫ��
  //
  Rect.x0 = X_START;
  Rect.x1 = xSize - 1;
  Rect.y0 = Y_START;
  for (i = 0; i < NumColors; i++) {
    for (j = 0; j < _aBarData[i].NumBars; j++) {
      Rect.y1 = Rect.y0 + yStep - 1;
	  //������ˮƽ��ɫ�ݶ����ľ���
      GUI_DrawGradientH(Rect.x0,  //���Ͻ�X λ�á�
	  Rect.y0, Rect.x1, 		  //���Ͻ�Y λ�á�
	  Rect.y1, 					  //���½�X λ�á�
	  _aColorStart[j], 			  //���½�Y λ�á�
	  _aBarData[i].Color);		  //���������Ҫ���Ƶ���ɫ��
      Rect.y0 = Rect.y1 + 1;	  //�������Ҳ�Ҫ���Ƶ���ɫ
    }
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  GUI_Init();
  //�����˱�����ɫ�Ժ󣬼ǵü��������������������
  GUI_SetBkColor(GUI_BLACK);
  GUI_Clear();
  GUI_CURSOR_Show();
	
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(&GUI_Font24_ASCII);
  GUI_DispStringHCenterAt("COLOR_ShowColorBar - Sample", 160, 5);
  _DemoShowColorBar();
  while(1)
    GUI_Delay(100);
}

/*************************** End of file ****************************/


