/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2012     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/
#include "GUI.h"
#include "GUIDRV_Template.h"
#include "stm32f4xx.h"
#include "bsp_lcd.h"


/*
********************************************************************
*
*       Layer configuration (to be modified)
*
********************************************************************
*/
// Physical display size
//
#define XSIZE_PHYS  			240
#define YSIZE_PHYS  			128
#define VXSIZE_PHYS 			240
#define VYSIZE_PHYS 			128

#define COLOR_CONVERSION  		GUICC_1
#define LCD_CONTROLLER       	8875 
#define LCD_BITSPERPIXEL       	1
#define LCD_USE_PARALLEL_16     0
//
// Color conversion
//

//
// Display driver
//
#define DISPLAY_DRIVER &GUIDRV_Template_API
//
// Orientation
//
#define DISPLAY_ORIENTATION  GUI_SWAP_XY | GUI_MIRROR_X | GUI_MIRROR_Y
//
// Configures touch screen module
//
#define GUI_TOUCH_AD_LEFT 	 35
#define GUI_TOUCH_AD_RIGHT 	 980
#define GUI_TOUCH_AD_TOP 	 915
#define GUI_TOUCH_AD_BOTTOM  90
/*
**********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif

#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif

#ifndef   VXSIZE_PHYS
  #define VXSIZE_PHYS XSIZE_PHYS
#endif

#ifndef   VYSIZE_PHYS
  #define VYSIZE_PHYS YSIZE_PHYS
#endif

#ifndef   VRAM_ADDR
  #define VRAM_ADDR 0
#endif

#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif

#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif


/*
****************************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
****************************************************************************   
*/

void LCD_X_Config(void) 
{		
// 	/* Set display driver and color conversion for 1st layer */
    GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);

//     if (g_ChipID == IC_8875)
// 	{
// 		if (g_ucGPIX == 1)	/* 	GPIX = 1  4.3  480x272 */
// 		{
//			/* Display driver configuration */
//			LCD_SetSizeEx    (0, 480, 272);
//			LCD_SetVSizeEx   (0, 480, 272);
//			
//			/* Touch calibration */ 
//			GUI_TOUCH_Calibrate(GUI_COORD_X, 0, 479, 985,  50);
//			GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, 271, 920,  100);
// 		}
// 		else 	/* 	GPIX = 0  7  800*480 */
// 		{
// 			/* Display driver configuration */
// 			LCD_SetSizeEx    (0, XSIZE_PHYS, YSIZE_PHYS);
// 			LCD_SetVSizeEx   (0, VXSIZE_PHYS, VYSIZE_PHYS);
// 		//	LCD_SetVRAMAddrEx(0, (void *)VRAM_ADDR);
// 			
// 			/* Touch calibration */
// 		//	GUI_TOUCH_SetOrientation(0);  
// 			GUI_TOUCH_Calibrate(GUI_COORD_X, 0, XSIZE_PHYS - 1, GUI_TOUCH_AD_LEFT,  GUI_TOUCH_AD_RIGHT);
// 			GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, YSIZE_PHYS - 1, GUI_TOUCH_AD_TOP,  GUI_TOUCH_AD_BOTTOM);		
// 		}
// 	}
// 	else
// 	{
 		LCD_SetSizeEx    (0, 240, 128);
 		LCD_SetVSizeEx   (0, 240, 128);
 		
 		/* Touch calibration */ 
 		GUI_TOUCH_Calibrate(GUI_COORD_X, 0, 399, 300, 3600);
 		GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, 239, 273,  3671);	
// 	}

}

/*
*********************************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if 
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*******************************************************************************
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) 
{
	int r;
	switch (Cmd) 
	{
		case LCD_X_INITCONTROLLER: 
		{
			//
			// Called during the initialization process in order to set up the
			// display controller and put it into operation. If the display
			// controller is not initialized by any external routine this needs
			// to be adapted by the customer...
			//
			// ...
			//  已经在前面初始化了，这里不再初始化
			LCD_Initilation();
			return 0;
		}
		case LCD_X_SETVRAMADDR: 
		{
			//
			// Required for setting the address of the video RAM for drivers
			// with memory mapped video RAM which is passed in the 'pVRAM' element of p
			//
			LCD_X_SETVRAMADDR_INFO * p;
			(void)p;
			p = (LCD_X_SETVRAMADDR_INFO *)pData;
			//...
			return 0;
		}
		case LCD_X_SETORG: 
		{
			//
			// Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
			//
			LCD_X_SETORG_INFO * p;
			(void)p;
			p = (LCD_X_SETORG_INFO *)pData;

			//...
			return 0;
		}
		case LCD_X_SHOWBUFFER: 
		{
			//
			// Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
			//
			LCD_X_SHOWBUFFER_INFO * p;
			(void)p;
			p = (LCD_X_SHOWBUFFER_INFO *)pData;
			//...
			return 0;
		}
		case LCD_X_SETLUTENTRY: 
		{
			//
			// Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
			//
			LCD_X_SETLUTENTRY_INFO * p;
			(void)p;
			p = (LCD_X_SETLUTENTRY_INFO *)pData;
			//...
			return 0;
		}
		case LCD_X_ON: 
		{
			//
			// Required if the display controller should support switching on and off
			//
			return 0;
		}
		case LCD_X_OFF: 
		{
			//
			// Required if the display controller should support switching on and off
			//
			// ...
			return 0;
		}
		default:
		r = -1;
	}
	return r;
}
/*************************** End of file ****************************/
