/****************************************Copyright (c)**************************************************
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: KeyDriver.h
**创   建   人: 杨承凯、魏宇、张娣
**创 建 日  期: 2007年7月31日
**最后修改日期: 2007年8月3日
**描        述:

**改		编: 杨承凯
**改 编  日 期: 2008年4月5日 
********************************************************************************************************/
#ifndef _KEY_DRIVER_H_
#define _KEY_DRIVER_H_

#ifdef	_KEY_DRIVER_C_
#define	KEY_DRIVER_EXT
#else
#define	KEY_DRIVER_EXT extern
#endif

//includes
#include "..\BSP\PS2.h"

//虚建定义
#define		VK_F1		0x70
#define		VK_F2		0x71
#define		VK_F3		0x72
#define		VK_F4		0x73
#define		VK_F5		0x74
#define		VK_F6		0x75
#define		VK_F7		0x76
#define		VK_F8		0x77
#define		VK_F9		0x78
#define		VK_F10		0x79
#define		VK_F11		0x7A
#define		VK_F12		0x7B
#define		VK_LEFT		0x25
#define		VK_UP		0x26
#define		VK_RIGHT	0x27
#define		VK_DOWN		0x28
#define		VK_DELETE	0x2E
#define		VK_PGUP		0x01
#define		VK_PGDOWN	0x02
#define		VK_END		0x23
#define		VK_HOME		0x24
#define		VK_INSERT	0x2D
#define		VK_RWIN		0x5C
#define		VK_LWIN		0x5B
#define		VK_TAB		0x09
#define		VK_ENTER	0x0A
#define		VK_BACK		0x08
#define		VK_ESCAPE	0x1B
#define     VK_PRINT	0x03
#define 	VK_SCROLL	0x04
	


//状态定义
#define		KEY_UP		0x01
#define		KEY_SHIFT	0x02
#define		KEY_CTRL	0x04
#define		KEY_ALT		0x08	
#define		KEY_ASC		0x10
	
//数据结构定义
typedef struct KEYINFO
{
	uint8 nKeyType;
	uint8 nVkValue;
}KEYINFO;
/****************************************************************************
* 名	称：KeyInit()
* 功	能：初始化键盘
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
KEY_DRIVER_EXT void KeyInit() reentrant;

/****************************************************************************
* 名	称：GetKeyValue()
* 功	能：获取键值
* 入口参数：&g_KeyValue
* 出口参数：无
* 说	明：
****************************************************************************/
KEY_DRIVER_EXT BOOL GetKeyValue(KEYINFO* pKeyValue) reentrant;



#endif