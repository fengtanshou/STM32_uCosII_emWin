#include "stm32f4xx.h"
#include "string.h"

#include "gui.h"
#include "WM.h"
#include "MULTIEDIT.h"
#include "widget.h"
#include "DIALOG.h"


#include "bsp_led.h"
#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>
#include  <os.h>
#include  <bsp.h>
#include  "app_cfg.h"
#include  "os_cfg_app.h"


extern void MainTask(void);								    
extern int g_iRunTime;


static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   AppTaskUpdateTCB;
static  CPU_STK  AppTaskUpdateStk[APP_CFG_TASK_UPDATE_STK_SIZE];

static  OS_TCB   AppTaskCOMTCB;
static  CPU_STK  AppTaskCOMStk[APP_CFG_TASK_COM_STK_SIZE];

static  OS_TCB   AppTaskUserIFTCB;
static  CPU_STK  AppTaskUserIFStk[APP_CFG_TASK_USER_IF_STK_SIZE];

static  OS_TCB   AppTaskGUITCB;
static  CPU_STK  AppTaskGUIStk[APP_CFG_TASK_GUI_STK_SIZE];

static  OS_TCB   AppTaskKeyTCB;
static  CPU_STK  AppTaskKeyStk[APP_CFG_TASK_KEY_STK_SIZE];




static void AppTaskStart          (void     *p_arg);
static void AppTaskCreate         (void);
static void AppTaskUserIF         (void     *p_arg);
static void AppTaskGUI            (void     *p_arg);
static void AppTaskKeyPro     	(void     *p_arg);
static void AppTaskCOM			(void 	  *p_arg);






int main(void)
{
    OS_ERR  err;


    CPU_IntDis();                                            	/* Disable all interrupts.                              */
    
    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )AppTaskStart, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
    
    (void)&err;
    
    return (0);
}

static  void  AppTaskStart (void *p_arg)
{
	OS_ERR err;
	(void)p_arg;	 
	
 	bsp_Init();
	CPU_Init();


#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif
                                          
    AppTaskCreate();                                           
    while (1) {                                         
		OSTimeDly(100, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

static void AppTaskGUIUpdate(void *p_arg)
{
	OS_ERR err;
	(void)p_arg;	 
		  
	while(1) {	
		OSTimeDly(100, OS_OPT_TIME_HMSM_STRICT, &err);							  	 	       											  
	}   
}

static void AppTaskCOM(void *p_arg)
{	
	OS_ERR err;
	(void)p_arg;	 
	 
	while(1) {	
		OSTimeDly(100, OS_OPT_TIME_HMSM_STRICT, &err);	
	}
	 						  	 	       											    
}

static void AppTaskUserIF(void *p_arg)
{
	OS_ERR err;
	(void)p_arg;	 

	while (1) {   		
        OSTimeDly(100, OS_OPT_TIME_HMSM_STRICT, &err);	     
	}
}



//
#define MSG_CHANGE_TEXT WM_USER+0
extern WM_HWIN  WinDialog(WM_HWIN hParent);


static WM_CALLBACK* _cbBkWindowOld;

static WM_HWIN _hWindow1;
static WM_HWIN _hWindow2;
static WM_HWIN _hChild;

static char _acInfoText[40];

s32 g_iRunTime = 5;

static void _cbBkWindow(WM_MESSAGE* pMsg) {
	switch (pMsg->MsgId) {
		case MSG_CHANGE_TEXT:
			strcpy(_acInfoText, pMsg->Data.p);
	  
		case WM_PAINT:
			GUI_SetBkColor(GUI_BLACK);
			GUI_Clear();
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(GUI_FONT_20_ASCII);
			GUI_DispStringHCenterAt("WindowManager - Sample", 120, 0);
			GUI_SetFont(GUI_FONT_8_ASCII);
			GUI_DispStringAt(_acInfoText, 0, 21);
		break;
	default:
		WM_DefaultProc(pMsg);
	}
}
static void _cbWindow1(WM_MESSAGE* pMsg) {
	int x, y;
	
	switch (pMsg->MsgId) {
		case WM_PAINT:
			GUI_SetBkColor(GUI_WHITE);
			GUI_Clear();
			GUI_SetColor(GUI_BLACK);
			GUI_SetFont(&GUI_Font20_ASCII);
			x = WM_GetWindowSizeX(pMsg->hWin);
			y = WM_GetWindowSizeY(pMsg->hWin);
			GUI_DispStringHCenterAt("Window 1", x / 2, (y / 2)+5);
			GUI_SetFont(&GUI_Font8_ASCII);
			GUI_DispStringAt("WWujialing", 0, y-8);  
			GUI_DispDecAt(g_iRunTime, x-60, y-8, 10);
			break;
		default:
			WM_DefaultProc(pMsg);
	}
}

static void _cbWindow2(WM_MESSAGE* pMsg) {
	int x, y;
	
	switch (pMsg->MsgId) {
		case WM_PAINT:
			GUI_SetBkColor(GUI_WHITE);
			GUI_Clear();
			GUI_SetColor(GUI_BLACK);
			GUI_SetFont(GUI_FONT_8X15B_ASCII);
			x = WM_GetWindowSizeX(pMsg->hWin);
			y = WM_GetWindowSizeY(pMsg->hWin);
			GUI_DispStringHCenterAt("Window 2", x / 2, 0);
		
			//GUI_SetFont(GUI_FONT_8X15B_ASCII);
			GUI_DispDecAt(g_iRunTime, x-60, 0, 5);
			break;
		
		case WM_SIZE:
			WM_InvalidateWindow(_hChild);
			break;
		
		default:
			WM_DefaultProc(pMsg);
	}
}
static void _ChangeInfoText(char* pStr) {
	WM_MESSAGE Message;
	
	Message.MsgId  = MSG_CHANGE_TEXT;
	Message.Data.p = pStr;
	WM_SendMessage(WM_HBKWIN, &Message);
	WM_InvalidateWindow(WM_HBKWIN);
}

static void AppTaskGUI(void *p_arg)
{
	GUI_RECT xp;	
	(void)p_arg;	 
	
	GUI_Init();
	GUI_SetBkColor(GUI_WHITE);
	GUI_Clear(); 
	
	WIDGET_SetDefaultEffect(&WIDGET_Effect_Simple);
	
	_cbBkWindowOld = WM_SetCallback(WM_HBKWIN, _cbBkWindow);	/* Create windows */
	_ChangeInfoText("WM_CreateWindow()");
	
	_hWindow1 = WM_CreateWindow( 1,  33, 120, 70, WM_CF_SHOW, _cbWindow1, 0);
	_hWindow2 = WM_CreateWindow( 1,  112, 238, 15, WM_CF_SHOW, _cbWindow2, 0);
	
	WinDialog(_hWindow1);	/* 对话框显示在窗口1上 */
	
	xp.x0 = 180;
	xp.y0 = 0;
	xp.x1 = 238;
	xp.y1 = 14;
    while (1) {
		WM_MoveTo(_hWindow1,  g_iRunTime%50,  33);
//		WM_MoveTo(_hWindow2,  1,  112);
		WM_InvalidateWindow(_hWindow2);
		GUI_Delay(100);
    }					 
}

static void AppTaskKeyPro(void *p_arg)
{
	OS_ERR err;
	(void)p_arg;	/* 避免编译器报警 */
		
	while (1) {
		g_iRunTime++;
		LED_Tog();
		OSTimeDly(100, OS_OPT_TIME_HMSM_STRICT, &err);
	}
}


static  void  AppTaskCreate (void)
{
	OS_ERR      err;
	
	/**************创建UPDATE任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskUpdateTCB,             
                 (CPU_CHAR     *)"App Task Update",
                 (OS_TASK_PTR   )AppTaskGUIUpdate, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_UPDATE_PRIO,
                 (CPU_STK      *)&AppTaskUpdateStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_UPDATE_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_UPDATE_STK_SIZE,
                 (OS_MSG_QTY    )1,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

	/**************创建COM任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskCOMTCB,            
                 (CPU_CHAR     *)"App Task COM",
                 (OS_TASK_PTR   )AppTaskCOM, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_COM_PRIO,
                 (CPU_STK      *)&AppTaskCOMStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE,
                 (OS_MSG_QTY    )2,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/**************创建USER IF任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskUserIFTCB,             
                 (CPU_CHAR     *)"App Task UserIF",
                 (OS_TASK_PTR   )AppTaskUserIF, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_USER_IF_PRIO,
                 (CPU_STK      *)&AppTaskUserIFStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/**************创建GUI任务*********************/			 
	OSTaskCreate((OS_TCB       *)&AppTaskGUITCB,              
                 (CPU_CHAR     *)"App Task GUI",
                 (OS_TASK_PTR   )AppTaskGUI, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_GUI_PRIO,
                 (CPU_STK      *)&AppTaskGUIStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/**************创建按键处理任务*********************/			 
	OSTaskCreate((OS_TCB       *)&AppTaskKeyTCB,              
                (CPU_CHAR     *)"App Task GUIRefresh",
                (OS_TASK_PTR   )AppTaskKeyPro, 
                (void         *)0,
                (OS_PRIO       )APP_CFG_TASK_KEY_PRIO,
                (CPU_STK      *)&AppTaskKeyStk[0],
                (CPU_STK_SIZE  )APP_CFG_TASK_KEY_STK_SIZE / 10,
                (CPU_STK_SIZE  )APP_CFG_TASK_KEY_STK_SIZE,
                (OS_MSG_QTY    )0,
                (OS_TICK       )0,
                (void         *)0,
                (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR       *)&err);
				 
}

