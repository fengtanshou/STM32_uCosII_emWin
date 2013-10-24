/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : 这个例子比较简单，主要是学习绘制用水平颜色梯度填充的矩形
*	修改记录 :
*		版本号    日期          作者                 说明
*		v1.0    2013-04-17    Eric2013      ST固件库版本 V1.0.2版本。
*
*	Copyright (C), 2013-2014
*   QQ超级群：216681322
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
  // 得到要显示的色调数目
  //
  NumColors = GUI_COUNTOF(_aBarData);
  for (i = NumBars = 0, NumBars = 0; i < NumColors; i++) {
    NumBars += _aBarData[i].NumBars;
  }

  yStep = (ySize - Y_START) / NumBars;

  //
  // 显示文字
  //
  Rect.x0 = 0;
  Rect.x1 = X_START - 1;
  Rect.y0 = Y_START;
  GUI_SetFont(&GUI_Font8x16);
  for (i = 0; i < NumColors; i++) {
    Rect.y1 = Rect.y0 + yStep * _aBarData[i].NumBars - 1;
	//在当前窗口指定的矩形区域内的指定位置处，使用当前字体显示作为参数的字符串
	//如果指定的矩形区域太小，文本会被裁剪。
    GUI_DispStringInRect(_aBarData[i].s, 	 //显示的字符串
	                      &Rect, 			 //要写入的客户端窗口矩形区域（单位：像素）。
						  GUI_TA_LEFT | GUI_TA_VCENTER); //对齐标记，可以通过“OR”操作进行组合
    Rect.y0 = Rect.y1 + 1;
  }
  //
  // 绘制色条
  //
  Rect.x0 = X_START;
  Rect.x1 = xSize - 1;
  Rect.y0 = Y_START;
  for (i = 0; i < NumColors; i++) {
    for (j = 0; j < _aBarData[i].NumBars; j++) {
      Rect.y1 = Rect.y0 + yStep - 1;
	  //绘制用水平颜色梯度填充的矩形
      GUI_DrawGradientH(Rect.x0,  //左上角X 位置。
	  Rect.y0, Rect.x1, 		  //左上角Y 位置。
	  Rect.y1, 					  //右下角X 位置。
	  _aColorStart[j], 			  //右下角Y 位置。
	  _aBarData[i].Color);		  //矩形最左侧要绘制的颜色。
      Rect.y0 = Rect.y1 + 1;	  //矩形最右侧要绘制的颜色
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
  //设置了背景颜色以后，记得加清除，这样才能起作用
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


