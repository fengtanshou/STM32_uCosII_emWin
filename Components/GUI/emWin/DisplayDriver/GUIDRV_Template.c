/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2012  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.16 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDRV_Template.c
Purpose     : Template driver, should be used as starting point
              for new display drivers.
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>
#include "stm32f4xx.h"
#include "LCD_Private.h"
#include "GUI_Private.h"
#include "LCD_SIM.h"
#include "LCD_ConfDefaults.h"

#include "bsp_lcd.h"

#define	USE_QUICK	0

#define LCD_NUM_COLORS (1L<<16)

#define emWin_Optimize   1   /* used for emWin optimize */

/*
   				   s_ucRA8875BusyNow

   (1)此标志是资源共享标志，用于解决FSMC共享的问题，因为触摸和
      TFT屏的操作都是使用的FSMC.
   (2)将标志放在emWin的接口函数里面处理比较方便，要是放到相应的
      函数里面，容易出现遗漏。
   (3)此标志在RA8875的使用过程中，不是必须的，如果采用
      GUI_Delay（1） //或其它的延时刷新函数。
	  GUI_TOUCH_Exec();
	  这里需要使用此标志，应为需要延时官方的DEMO。
  （4）如果有系统支持的话，直接使用信号量来解决就可以。
*/

__IO uint8_t s_ucRA8875BusyNow = 0;
/*
********************************************************************
*
*       Defines
*
********************************************************************
*/

/*
********************************************************************
*
*       Macros for MIRROR_, SWAP_ and LUT_
*
********************************************************************
*/
#if (!defined (LCD_LUT_COM) && !defined(LCD_LUT_SEG))
  #if   (!LCD_MIRROR_X && !LCD_MIRROR_Y && !LCD_SWAP_XY) 
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) y
  #elif (!LCD_MIRROR_X && !LCD_MIRROR_Y &&  LCD_SWAP_XY) 
    #define LOG2PHYS_X(x, y) y
    #define LOG2PHYS_Y(x, y) x
  #elif (!LCD_MIRROR_X &&  LCD_MIRROR_Y && !LCD_SWAP_XY) 
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) LCD_YSIZE - 1 - (y)
  #elif (!LCD_MIRROR_X &&  LCD_MIRROR_Y &&  LCD_SWAP_XY) 
    #define LOG2PHYS_X(x, y) y
    #define LOG2PHYS_Y(x, y) LCD_XSIZE - 1 - (x)
  #elif ( LCD_MIRROR_X && !LCD_MIRROR_Y && !LCD_SWAP_XY) 
    #define LOG2PHYS_X(x, y) LCD_XSIZE - 1 - (x)
    #define LOG2PHYS_Y(x, y) y
  #elif ( LCD_MIRROR_X && !LCD_MIRROR_Y &&  LCD_SWAP_XY) 
    #define LOG2PHYS_X(x, y) LCD_YSIZE - 1 - (y)
    #define LOG2PHYS_Y(x, y) x
  #elif ( LCD_MIRROR_X &&  LCD_MIRROR_Y && !LCD_SWAP_XY) 
    #define LOG2PHYS_X(x, y) LCD_XSIZE - 1 - (x)
    #define LOG2PHYS_Y(x, y) LCD_YSIZE - 1 - (y)
  #elif ( LCD_MIRROR_X &&  LCD_MIRROR_Y &&  LCD_SWAP_XY) 
    #define LOG2PHYS_X(x, y) LCD_YSIZE - 1 - (y)
    #define LOG2PHYS_Y(x, y) LCD_XSIZE - 1 - (x)
  #endif
#else
  #if   ( defined (LCD_LUT_COM) && !defined(LCD_LUT_SEG))
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) LCD__aLine2Com0[y]
  #elif (!defined (LCD_LUT_COM) &&  defined(LCD_LUT_SEG))
    #define LOG2PHYS_X(x, y) LCD__aCol2Seg0[x]
    #define LOG2PHYS_Y(x, y) y
  #elif ( defined (LCD_LUT_COM) &&  defined(LCD_LUT_SEG))
    #define LOG2PHYS_X(x, y) LCD__aCol2Seg0[x]
    #define LOG2PHYS_Y(x, y) LCD__aLine2Com0[y]
  #endif
#endif

/*
**********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  U32 VRAMAddr;
  int xSize, ySize;
  int vxSize, vySize;
  int vxSizePhys;
  int BitsPerPixel;
} DRIVER_CONTEXT;

/*
**********************************************************************
*
*       Static functions
*
**********************************************************************
*/

/*
*************************************************************************
*
*       _SetPixelIndex
*
* Purpose:
*   Sets the index of the given pixel. The upper layers
*   calling this routine make sure that the coordinates are in range, so
*   that no check on the parameters needs to be performed.
*
*************************************************************************
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int PixelIndex) {
  #ifdef WIN32
    LCDSIM_SetPixelIndex(x, y, PixelIndex, pDevice->LayerIndex);
  #else
    //
    // Convert logical into physical coordinates (Dep. on LCDConf.h)
    //
    #if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
      int xPhys, yPhys;

      xPhys = LOG2PHYS_X(x, y);
      yPhys = LOG2PHYS_Y(x, y);
    #else
      #define xPhys x
      #define yPhys y
    #endif
    GUI_USE_PARA(pDevice);
    GUI_USE_PARA(x);
    GUI_USE_PARA(y);
    GUI_USE_PARA(PixelIndex);
    {
		LCD_SetPoint(x, y, PixelIndex);
    }
    #if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
      #undef xPhys
      #undef yPhys
    #endif
  #endif
}

/*
*************************************************************************
*
*       _GetPixelIndex
*
* Purpose:
*   Returns the index of the given pixel. The upper layers
*   calling this routine make sure that the coordinates are in range, so
*   that no check on the parameters needs to be performed.
*
*************************************************************************
*/
static unsigned int _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y) {
  unsigned int PixelIndex;
	
  #ifdef WIN32
    PixelIndex = LCDSIM_GetPixelIndex(x, y, pDevice->LayerIndex);
  #else
    // Convert logical into physical coordinates (Dep. on LCDConf.h)
    #if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
      int xPhys, yPhys;

      xPhys = LOG2PHYS_X(x, y);
      yPhys = LOG2PHYS_Y(x, y);
    #else
      #define xPhys x
      #define yPhys y
    #endif
    GUI_USE_PARA(pDevice);
    GUI_USE_PARA(x);
    GUI_USE_PARA(y);
    {
		
    }
    #if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
      #undef xPhys
      #undef yPhys
    #endif
  #endif
  return PixelIndex;
}

/*
********************************************************************
*
*       _XorPixel
*
********************************************************************
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {

//	LCD_PIXELINDEX Index = LCD_GetPoint(x,y);	
//	LCD_SetPoint(x, y, LCD_NUM_COLORS-1-Index);
}

/*
********************************************************************
*
*       _DrawHLine
*
*******************************************************************
*/
static void _DrawHLine  (GUI_DEVICE * pDevice, int x0, int y,  int x1) {
	LCD_PIXELINDEX ColorIndex;

    ColorIndex = LCD__GetColorIndex();	
#if USE_QUICK	
	LCD_DrawLine(x0, y, x1, y, ColorIndex);
#else	
	if (ColorIndex) {
		SetColor(1);
	} else {
		SetColor(0);
	}
	Draw_LineToDispBuff(x0, y, x1-x0+1, 1);
	LCD_SendBuffDC(x0, y, x1-x0+1, 1);
#endif
}

/*
*************************************************************************
*
*       _DrawVLine, not optimized
*
*************************************************************************
*/
static void _DrawVLine  (GUI_DEVICE * pDevice, int x, int y0,  int y1) {  
	LCD_PIXELINDEX ColorIndex;
	
    ColorIndex = LCD__GetColorIndex();
#if USE_QUICK		
	LCD_DrawLine(x, y0, x, y1, ColorIndex);
#else	
	if (ColorIndex) {
		SetColor(1);
	} else {
		SetColor(0);
	}
	Draw_LineToDispBuff(x, y0, 1, y1-y0+1);
	     LCD_SendBuffDC(x, y0, 1, y1-y0+1);
#endif	
}

/*
**************************************************************************
*
*       _FillRect
*
**************************************************************************
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) 
{
#if USE_QUICK		
	for (; y0 <= y1; y0++) {
		_DrawHLine(pDevice, x0, y0, x1);
	} 
	
#else	
	LCD_PIXELINDEX ColorIndex;
	
    ColorIndex = LCD__GetColorIndex();	
	if (ColorIndex) {
		SetColor(1);
	} else {
		SetColor(0);
	}
	Draw_LineToDispBuff(x0, y0, x1-x0+1, y1-y0+1);
	     LCD_SendBuffDC(x0, y0, x1-x0+1, y1-y0+1);
#endif	
}

/*
********************************************************************
*
*       Draw Bitmap 1 BPP
*
********************************************************************
*/
static void _DrawBitLine1BPP(GUI_DEVICE * pDevice, unsigned x, unsigned y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX pixels;
  LCD_PIXELINDEX Index0 = *(pTrans+0);
  LCD_PIXELINDEX Index1 = *(pTrans+1);
/*
// Jump to right entry point
*/
  pixels = *p;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS|LCD_DRAWMODE_XOR)) {
  case 0:
    #if defined (SETNEXTPIXEL)   /* Optimization ! */
      x+=Diff;
      LCD_SetCursor(x,y);
    #endif
    switch (Diff&7) {
    case 0:   
      goto WriteBit0;
    case 1:   
      goto WriteBit1;
    case 2:
      goto WriteBit2;
    case 3:
      goto WriteBit3;
    case 4:
      goto WriteBit4;
    case 5:   
      goto WriteBit5;
    case 6:   
      goto WriteBit6;
    case 7:   
      goto WriteBit7;
    }
    break;
  case LCD_DRAWMODE_TRANS:
    switch (Diff&7) {
    case 0:
      goto WriteTBit0;
    case 1:
      goto WriteTBit1;
    case 2:
      goto WriteTBit2;
    case 3:
      goto WriteTBit3;
    case 4:
      goto WriteTBit4;
    case 5:   
      goto WriteTBit5;
    case 6:   
      goto WriteTBit6;
    case 7:   
      goto WriteTBit7;
    }
    break;
  case LCD_DRAWMODE_XOR:
    switch (Diff&7) {
    case 0:   
      goto WriteXBit0;
    case 1:   
      goto WriteXBit1;
    case 2:
      goto WriteXBit2;
    case 3:
      goto WriteXBit3;
    case 4:
      goto WriteXBit4;
    case 5:   
      goto WriteXBit5;
    case 6:   
      goto WriteXBit6;
    case 7:   
      goto WriteXBit7;
    }
  }
/*
        Write with transparency
*/

  WriteTBit0:
   if (pixels&(1<<7)) LCD_SetPoint(x+0, y, Index1);
    if (!--xsize)
      return;
  WriteTBit1:
    if (pixels&(1<<6)) LCD_SetPoint(x+1, y, Index1);
    if (!--xsize)
      return;
  WriteTBit2:
    if (pixels&(1<<5)) LCD_SetPoint(x+2, y, Index1);
    if (!--xsize)
      return;
  WriteTBit3:
    if (pixels&(1<<4)) LCD_SetPoint(x+3, y, Index1);
    if (!--xsize)
      return;
  WriteTBit4:
    if (pixels&(1<<3)) LCD_SetPoint(x+4, y, Index1);
    if (!--xsize)
      return;
  WriteTBit5:
    if (pixels&(1<<2)) LCD_SetPoint(x+5, y, Index1);
    if (!--xsize)
      return;
  WriteTBit6:
    if (pixels&(1<<1)) LCD_SetPoint(x+6, y, Index1);
    if (!--xsize)
      return;
  WriteTBit7:
    if (pixels&(1<<0)) LCD_SetPoint(x+7, y, Index1);
    if (!--xsize)
      return;
    x+=8;
    pixels = *(++p);
    goto WriteTBit0;

/*
        Write without transparency
*/

  WriteBit0:
    LCD_SetPoint(x+0, y, (pixels&(1<<7)) ? Index1 : Index0);
    if (!--xsize)
      return;
  WriteBit1:
    LCD_SetPoint(x+1, y, (pixels&(1<<6)) ? Index1 : Index0);
    if (!--xsize)
      return;
  WriteBit2:
    LCD_SetPoint(x+2, y, (pixels&(1<<5)) ? Index1 : Index0);
    if (!--xsize)
      return;
  WriteBit3:
    LCD_SetPoint(x+3, y, (pixels&(1<<4)) ? Index1 : Index0);
    if (!--xsize)
      return;
  WriteBit4:
    LCD_SetPoint(x+4, y, (pixels&(1<<3)) ? Index1 : Index0);
    if (!--xsize)
      return;
  WriteBit5:
    LCD_SetPoint(x+5, y, (pixels&(1<<2)) ? Index1 : Index0);
    if (!--xsize)
      return;
  WriteBit6:
    LCD_SetPoint(x+6, y, (pixels&(1<<1)) ? Index1 : Index0);
    if (!--xsize)
      return;
  WriteBit7:
    LCD_SetPoint(x+7, y, (pixels&(1<<0)) ? Index1 : Index0);
    if (!--xsize)
      return;
    x+=8;
    pixels = *(++p);
    goto WriteBit0;

/*
        Write XOR mode
*/

  WriteXBit0:
    if (pixels&(1<<7))
      _XorPixel(pDevice, x+0, y);
    if (!--xsize)
      return;
  WriteXBit1:
    if (pixels&(1<<6))
      _XorPixel(pDevice, x+0, y);
    if (!--xsize)
      return;
  WriteXBit2:
    if (pixels&(1<<5))
      _XorPixel(pDevice, x+0, y);
    if (!--xsize)
      return;
  WriteXBit3:
    if (pixels&(1<<4))
      _XorPixel(pDevice, x+0, y);
    if (!--xsize)
      return;
  WriteXBit4:
    if (pixels&(1<<3))
      _XorPixel(pDevice, x+0, y);
    if (!--xsize)
      return;
  WriteXBit5:
    if (pixels&(1<<2))
      _XorPixel(pDevice, x+0, y);
    if (!--xsize)
      return;
  WriteXBit6:
    if (pixels&(1<<1))
      _XorPixel(pDevice, x+0, y);
    if (!--xsize)
      return;
  WriteXBit7:
    if (pixels&(1<<0))
      _XorPixel(pDevice, x+0, y);
    if (!--xsize)
      return;
    x+=8;
    pixels = *(++p);
    goto WriteXBit0;
}

/*
**************************************************************************
*
*       Draw Bitmap 2 BPP
*
**************************************************************************
*/
static void  _DrawBitLine2BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX pixels;
/*
// Jump to right entry point
*/
  pixels = *p;
  if (pTrans) {
    /*
      with palette
    */
    if (GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) switch (Diff&3) {
    case 0:
      goto WriteTBit0;
    case 1:
      goto WriteTBit1;
    case 2:
      goto WriteTBit2;
    default:
      goto WriteTBit3;
    } else switch (Diff&3) {
    case 0:
      goto WriteBit0;
    case 1:
      goto WriteBit1;
    case 2:
      goto WriteBit2;
    default:
      goto WriteBit3;
    }
  /*
          Write without transparency
  */
  WriteBit0:
    LCD_SetPoint(x+0, y, *(pTrans+(pixels>>6)));
    if (!--xsize)
      return;
  WriteBit1:
    LCD_SetPoint(x+1, y, *(pTrans+(3&(pixels>>4))));
    if (!--xsize)
      return;
  WriteBit2:
    LCD_SetPoint(x+2, y, *(pTrans+(3&(pixels>>2))));
    if (!--xsize)
      return;
  WriteBit3:
    LCD_SetPoint(x+3, y, *(pTrans+(3&(pixels))));
    if (!--xsize)
      return;
    pixels = *(++p);
    x+=4;
    goto WriteBit0;
  /*
          Write with transparency
  */
  WriteTBit0:
    if (pixels&(3<<6))
      LCD_SetPoint(x+0, y, *(pTrans+(pixels>>6)));
    if (!--xsize)
      return;
  WriteTBit1:
    if (pixels&(3<<4))
      LCD_SetPoint(x+1, y, *(pTrans+(3&(pixels>>4))));
    if (!--xsize)
      return;
  WriteTBit2:
    if (pixels&(3<<2))
      LCD_SetPoint(x+2, y, *(pTrans+(3&(pixels>>2))));
    if (!--xsize)
      return;
  WriteTBit3:
    if (pixels&(3<<0))
      LCD_SetPoint(x+3, y, *(pTrans+(3&(pixels))));
    if (!--xsize)
      return;
    pixels = *(++p);
    x+=4;
    goto WriteTBit0;
  } else { 
    /* 
      without palette 
    */
    if (GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) switch (Diff&3) {
    case 0:
      goto WriteDDPTBit0;
    case 1:
      goto WriteDDPTBit1;
    case 2:
      goto WriteDDPTBit2;
    default:
      goto WriteDDPTBit3;
    } else switch (Diff&3) {
    case 0:
      goto WriteDDPBit0;
    case 1:
      goto WriteDDPBit1;
    case 2:
      goto WriteDDPBit2;
    default:
      goto WriteDDPBit3;
    }
  /*
          Write without transparency
  */
  WriteDDPBit0:
    LCD_SetPoint(x+0, y, (pixels>>6));
    if (!--xsize)
      return;
  WriteDDPBit1:
    LCD_SetPoint(x+1, y, (3&(pixels>>4)));
    if (!--xsize)
      return;
  WriteDDPBit2:
    LCD_SetPoint(x+2, y, (3&(pixels>>2)));
    if (!--xsize)
      return;
  WriteDDPBit3:
    LCD_SetPoint(x+3, y, (3&(pixels)));
    if (!--xsize)
      return;
    pixels = *(++p);
    x+=4;
    goto WriteDDPBit0;
  /*
          Write with transparency
  */
  WriteDDPTBit0:
    if (pixels&(3<<6))
      LCD_SetPoint(x+0, y, (pixels>>6));
    if (!--xsize)
      return;
  WriteDDPTBit1:
    if (pixels&(3<<4))
      LCD_SetPoint(x+1, y, (3&(pixels>>4)));
    if (!--xsize)
      return;
  WriteDDPTBit2:
    if (pixels&(3<<2))
      LCD_SetPoint(x+2, y, (3&(pixels>>2)));
    if (!--xsize)
      return;
  WriteDDPTBit3:
    if (pixels&(3<<0))
      LCD_SetPoint(x+3, y, (3&(pixels)));
    if (!--xsize)
      return;
    pixels = *(++p);
    x+=4;
    goto WriteDDPTBit0;
  }
}

/*
**************************************************************************
*
*       Draw Bitmap 4 BPP
*
**************************************************************************
*/
static void  _DrawBitLine4BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX pixels;

  pixels = *p;
  if (pTrans)
  {
    if (GUI_Context.DrawMode & LCD_DRAWMODE_TRANS)
    {
      if ((Diff&1) ==0)
        goto WriteTBit0;
        goto WriteTBit1;
    }
    else
    {
      if ((Diff&1) ==0)
        goto WriteBit0;
        goto WriteBit1;
    }

  WriteBit0:
    LCD_SetPoint(x+0, y, *(pTrans+(pixels>>4)));
    if (!--xsize)
      return;
  WriteBit1:
    LCD_SetPoint(x+1, y, *(pTrans+(pixels&0xf)));
    if (!--xsize)
      return;
    x+=2;
    pixels = *(++p);
    goto WriteBit0;
  /*
          Write with transparency
  */
  WriteTBit0:
    if (pixels>>4)
      LCD_SetPoint(x+0, y, *(pTrans+(pixels>>4)));
    if (!--xsize)
      return;
  WriteTBit1:
    if (pixels&0xf)
      LCD_SetPoint(x+1, y, *(pTrans+(pixels&0xf)));
    if (!--xsize)
      return;
    x+=2;
    pixels = *(++p);
    goto WriteTBit0;
  } else {
    /*
      without palette
    */
    if (GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) {
      if ((Diff&1) ==0)
        goto WriteDDPTBit0;
      goto WriteDDPTBit1;
    } else {
      if ((Diff&1) ==0)
        goto WriteDDPBit0;
      goto WriteDDPBit1;
    }
  /*
          Write without transparency
  */
  WriteDDPBit0:
    LCD_SetPoint(x+0, y, (pixels>>4));
    if (!--xsize)
      return;
  WriteDDPBit1:
    LCD_SetPoint(x+1, y, (pixels&0xf));
    if (!--xsize)
      return;
    x+=2;
    pixels = *(++p);
    goto WriteDDPBit0;
  /*
          Write with transparency
  */
  WriteDDPTBit0:
    if (pixels>>4)
      LCD_SetPoint(x+0, y, (pixels>>4));
    if (!--xsize)
      return;
  WriteDDPTBit1:
    if (pixels&0xf)
      LCD_SetPoint(x+1, y, (pixels&0xf));
    if (!--xsize)
      return;
    x+=2;
    pixels = *(++p);
    goto WriteDDPTBit0;
  }
}

/*
**************************************************************************
*
*       Draw Bitmap 8 BPP
*
**************************************************************************
*/
static void  _DrawBitLine8BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX pixel;
  if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS)==0) {
    if (pTrans) {
      for (;xsize > 0; xsize--,x++,p++) {
        pixel = *p;
        LCD_SetPoint(x, y, *(pTrans+pixel));
      }
    } else {
      for (;xsize > 0; xsize--,x++,p++) {
        LCD_SetPoint(x, y, *p);
      }
    }
  } else {   /* Handle transparent bitmap */
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          LCD_SetPoint(x+0, y, *(pTrans+pixel));
        }
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          LCD_SetPoint(x+0, y, pixel);
        }
      }
    }
  }
}

/*
**************************************************************************
*
*       Draw Bitmap 16 BPP, not optimized
*
**************************************************************************
*/
static void _DrawBitLine16BPP(GUI_DEVICE * pDevice, int x, int y, U16 const GUI_UNI_PTR * p, int xsize) 
{
	LCD_PIXELINDEX Index;

	if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS)==0) {
		LCD_SetCursor(x, y);
		for (;xsize > 0; xsize--)	{
			LCDRAM((uint16_t)(*p++));
		}
	} else {   /* Handle transparent bitmap */
		for (; xsize > 0; xsize--, x++, p++) {
			Index = *p;
			if (Index) {
				LCD_SetPoint(x+0, y, Index);
			}
		}
	}
}

/*
**************************************************************************
*
*       Draw Bitmap 32 BPP, not optimized
*
**************************************************************************
*/
static void _DrawBitLine32BPP(GUI_DEVICE * pDevice, int x, int y, U32 const GUI_UNI_PTR * p, int xsize) 
{
	LCD_PIXELINDEX Index;

	if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS)==0) {
		LCD_SetCursor(x, y);
		for (;xsize > 0; xsize--)	{
			LCDRAM(*p++);
		}
	} else {   /* Handle transparent bitmap */
		for (; xsize > 0; xsize--, x++, p++) {
			Index = *p;
			if (Index) {
				LCD_SetPoint(x+0, y, Index);
			}
		}
	}
}

/*
**************************************************************************
*
*       _DrawBitmap
*
**************************************************************************
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX* pTrans) {
  int i;

  switch (BitsPerPixel) {
  case 1:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine1BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 2:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine2BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 4:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine4BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 8:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine8BPP(pDevice, x0, i + y0, pData, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 16:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine16BPP(pDevice, x0, i + y0, (const U16 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  case 32:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine32BPP(pDevice, x0, i + y0, (const U32 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  }
}

/*
********************************************************************
*
*       _InitOnce
*
* Purpose:
*   Allocates a fixed block for the context of the driver
*
* Return value:
*   0 on success, 1 on error
*
********************************************************************
*/
static int _InitOnce(GUI_DEVICE * pDevice) {
  DRIVER_CONTEXT * pContext;

  if (pDevice->u.pContext == NULL) {
    pDevice->u.pContext = GUI_ALLOC_GetFixedBlock(sizeof(DRIVER_CONTEXT));
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->BitsPerPixel = LCD__GetBPP(pDevice->pColorConvAPI->pfGetIndexMask());
  }
  return pDevice->u.pContext ? 0 : 1;
}

/*
**************************************************************************
*
*       _GetDevProp
*
**************************************************************************
*/
static I32 _GetDevProp(GUI_DEVICE * pDevice, int Index) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  switch (Index) {
  case LCD_DEVCAP_XSIZE:
    return pContext->xSize;
  case LCD_DEVCAP_YSIZE:
    return pContext->ySize;
  case LCD_DEVCAP_VXSIZE:
    return pContext->vxSize;
  case LCD_DEVCAP_VYSIZE:
    return pContext->vySize;
  case LCD_DEVCAP_BITSPERPIXEL:
    return pContext->BitsPerPixel;
  case LCD_DEVCAP_NUMCOLORS:
    return 0;
  case LCD_DEVCAP_XMAG:
    return 1;
  case LCD_DEVCAP_YMAG:
    return 1;
  case LCD_DEVCAP_MIRROR_X:
    return 0;
  case LCD_DEVCAP_MIRROR_Y:
    return 0;
  case LCD_DEVCAP_SWAP_XY:
    return 0;
  }
  return -1;
}

/*
**************************************************************************
*
*       _GetDevData
*
**************************************************************************
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  #if GUI_SUPPORT_MEMDEV
    switch (Index) {
    case LCD_DEVDATA_MEMDEV:
	//
	// TBD: Has to be adapted to the right memory device depending on the used color depth!
    // 
	  return (void *)&GUI_MEMDEV_DEVICE_16; 
    }
  #else
    GUI_USE_PARA(Index);
  #endif
  return NULL;
}

/*
**************************************************************************
*
*       _GetRect
*
**************************************************************************
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pRect->x0 = 0;
  pRect->y0 = 0;
  pRect->x1 = pContext->vxSize - 1;
  pRect->y1 = pContext->vySize - 1;
}

/*
**************************************************************************
*
*       _SetOrg
*
**************************************************************************
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  LCD_X_SETORG_INFO Data = {0};

  Data.xPos = x;
  Data.yPos = y;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETORG, (void *)&Data);
}

/*
**********************************************************************
*
*       Static code: Functions available by _GetDevFunc()
*
**********************************************************************
*/

/*
*********************************************************************
*
*       _SetVRAMAddr
*
*********************************************************************
*/
static void _SetVRAMAddr(GUI_DEVICE * pDevice, void * pVRAM) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETVRAMADDR_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->VRAMAddr = (U32)pVRAM;
    Data.pVRAM = pVRAM;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETVRAMADDR, (void *)&Data);
  }
}

/*
********************************************************************
*
*       _SetVSize
*
********************************************************************
*/
static void _SetVSize(GUI_DEVICE * pDevice, int xSize, int ySize) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->vxSize = xSize;
    pContext->vySize = ySize;
    pContext->vxSizePhys = xSize;
  }
}

/*
********************************************************************
*
*       _SetSize
*
********************************************************************
*/
static void _SetSize(GUI_DEVICE * pDevice, int xSize, int ySize) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETSIZE_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->vxSizePhys = (pContext->vxSizePhys == 0) ? xSize : pContext->vxSizePhys;
    pContext->xSize = xSize;
    pContext->ySize = ySize;
    Data.xSize = xSize;
    Data.ySize = ySize;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETSIZE, (void *)&Data);
  }
}

/*
*********************************************************************
*
*       _Init
*
*********************************************************************
*/
static int  _Init(GUI_DEVICE * pDevice) {
  int r;

  r = _InitOnce(pDevice);
  r |= LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_INITCONTROLLER, NULL);
  return r;
}

/*
*********************************************************************
*
*       _On
*
*********************************************************************
*/
static void _On (GUI_DEVICE * pDevice) {
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_ON, NULL);
}

/*
*********************************************************************
*
*       _Off
*
*********************************************************************
*/
static void _Off (GUI_DEVICE * pDevice) {
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_OFF, NULL);
}

/*
*********************************************************************
*
*       _SetLUTEntry
*
*********************************************************************
*/
static void _SetLUTEntry(GUI_DEVICE * pDevice, U8 Pos, LCD_COLOR Color) {
  LCD_X_SETLUTENTRY_INFO Data = {0};

  Data.Pos   = Pos;
  Data.Color = Color;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETLUTENTRY, (void *)&Data);
}

/*
*********************************************************************
*
*       _GetDevFunc
*
*********************************************************************
*/
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  GUI_USE_PARA(ppDevice);
  switch (Index) {
  case LCD_DEVFUNC_SET_VRAM_ADDR:
    return (void (*)(void))_SetVRAMAddr;
  case LCD_DEVFUNC_SET_VSIZE:
    return (void (*)(void))_SetVSize;
  case LCD_DEVFUNC_SET_SIZE:
    return (void (*)(void))_SetSize;
  case LCD_DEVFUNC_INIT:
    return (void (*)(void))_Init;
  case LCD_DEVFUNC_ON:
    return (void (*)(void))_On;
  case LCD_DEVFUNC_OFF:
    return (void (*)(void))_Off;
  case LCD_DEVFUNC_SETLUTENTRY:
    return (void (*)(void))_SetLUTEntry;
  }
  return NULL;
}

/*
**********************************************************************
*
*       Public data
*
**********************************************************************
*/

/*
***********************************************************************
*
*       GUI_DEVICE_API structure
*
***********************************************************************
*/
const GUI_DEVICE_API GUIDRV_Template_API = {
  //
  // Data
  //
  DEVICE_CLASS_DRIVER,
  //
  // Drawing functions
  //
  _DrawBitmap,
  _DrawHLine,
  _DrawVLine,
  _FillRect,
  _GetPixelIndex,
  _SetPixelIndex,
  _XorPixel,
  //
  // Set origin
  //
  _SetOrg,
  //
  // Request information
  //
  _GetDevFunc,
  _GetDevProp,
  _GetDevData,
  _GetRect,
};

/*************************** End of file ****************************/
