/****************************************Copyright (c)**************************************************
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: KeyDriver.c
**创   建   人: 杨承凯、魏宇、张娣
**创 建 日  期: 2007年7月31日
**最后修改日期: 2007年8月3日
**描        述: 键盘处理程序

**改		编: 杨承凯
**改 编  日 期: 2008年4月5日 
********************************************************************************************************/
#define _KEY_DRIVER_C_

#include "..\BSP\bsp.h"
#include "KeyDriver.h"

//状态定义
#define		FIRST_MAKE		0
#define		SECOND_MAKE		1
#define		THIRD_MAKE		2
#define		FIRST_BREAK		3
#define		SECOND_BREAK	4
#define		THIRD_BREAK		5



//数据定义
extern uint8 code VkTable[136][3];
extern uint8 code E0Table[128];
static uint8 		xdata nScanType;		//记录扫描码类型
static KEYINFO 		xdata KeyInfo;			//按键记录值
static uint8		xdata nCharHigher;		//按键的大小写
static uint8 		xdata nKeyValidate;		//记录键盘是否有动作
static uint8		xdata Caps_cnt;			//记录caps按键中0x58的次数
static uint8		xdata Pause_cnt;		//记录Pause按键中0xF0的次数

BOOL	E0_flag;							//接收到第二类按键的0xE0时的标志位
BOOL	E1_flag;							//接收到第三类按键的0xE1时的标志位
BOOL	Pause_flag;							//接收完Pause按键时的标志位
BOOL	Caps_status;						//caps lock按键的状态标志位

/****************************************************************************
* 名	称：KeyInit()
* 功	能：初始化键盘
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void KeyInit() reentrant
{
	KeyInfo.nKeyType = 0x00;
	KeyInfo.nVkValue = 0x00;

	nScanType = FIRST_MAKE;				//扫描类型初始化为第一类按键的通码
	Caps_status = 0;					//caps lock按键初始化为0	
	Caps_cnt = 0;

	Pause_flag = 0;
	E1_flag = 0;
	E0_flag = 0;
	Pause_cnt = 0;
}

/****************************************************************************
* 名	称：KeyScan()
* 功	能：键盘扫描
* 入口参数：无
* 出口参数：无
* 说	明：
****************************************************************************/
uint8 KeyScan() reentrant
{
	uint8 TmpChar;					//存放接收到的一个字节数据
	
	TmpChar = Ps2GetData();			//接收中断收到的字节数据

	
	//判断扫描码类型
	if(TmpChar == 0xE1)				//第三类按键标志
	{
		E1_flag = 1;
		nScanType = THIRD_MAKE;
	}
	
	else if(TmpChar == 0xE0)		//第二类按键标志
	{
		nScanType = SECOND_MAKE;
		E0_flag = 1;
	}

	else if(TmpChar == 0xF0)		//第一、二类断码标志
	{
		nScanType = FIRST_BREAK;
		if(E0_flag == 1)			//如果收到E0，表明是第二类断码
		{
			E0_flag = 0;
			nScanType = SECOND_BREAK;
		}
		if(E1_flag == 1)			//如果是在收到E1的情况下的0xF0
		{
			Pause_cnt ++;
			nScanType = THIRD_MAKE;
			if(Pause_cnt == 2)		//如果收到了两个0xF0，表明pause按键还有一个字节
			{
				Pause_cnt = 0;
				nScanType = THIRD_BREAK;
				E1_flag = 0;
			}
		}
	}
	else							//扫描码数值
	{
		switch(nScanType)
		{	
			case FIRST_MAKE:		//第一类按键的通码
			{	
				
				KeyInfo.nKeyType &= ~KEY_UP;
				if(Caps_status == (((KeyInfo.nKeyType & KEY_SHIFT)>0)?1:0))
				{						//如果caps和shift的状态相同
					nCharHigher = 0;
				}
				else
				{
					nCharHigher = 1;
				}

				KeyInfo.nKeyType &= ~KEY_ASC;
				KeyInfo.nVkValue = VkTable[TmpChar][nCharHigher];
				KeyInfo.nKeyType |= VkTable[TmpChar][2];
							
				nKeyValidate = 1;
				break;
			}
			case FIRST_BREAK:			//第一类按键的断码
			{
				KeyInfo.nKeyType |= KEY_UP;
				if(Caps_status == (((KeyInfo.nKeyType & KEY_SHIFT)>0)?1:0))
				{
					nCharHigher = 0;
				}
				else
				{
					nCharHigher = 1;
				}
				KeyInfo.nKeyType &= ~KEY_ASC;
				KeyInfo.nVkValue = VkTable[TmpChar][nCharHigher];
				KeyInfo.nKeyType |= VkTable[TmpChar][2];

				nKeyValidate = 1;
				nScanType = FIRST_MAKE;
				break;
			}	
			case SECOND_MAKE:		//第二类按键的通码
			{
				KeyInfo.nKeyType &= ~KEY_ASC;
				KeyInfo.nKeyType &= ~KEY_UP;
				KeyInfo.nVkValue = E0Table[TmpChar];
				nKeyValidate = 1;
				break;
			}	
			case SECOND_BREAK:		//第二类按键的断码
			{
				KeyInfo.nKeyType &= ~KEY_ASC;
				KeyInfo.nKeyType |= KEY_UP;
				KeyInfo.nVkValue = E0Table[TmpChar];
				nKeyValidate = 1;
				nScanType = FIRST_MAKE;
				break;
			}
			case THIRD_MAKE:		//第三类按键的通码
			{
				KeyInfo.nKeyType &= ~KEY_ASC;
				KeyInfo.nKeyType &= ~KEY_UP;
				nKeyValidate = 0;
				break;	
			}
			case THIRD_BREAK:		//第三类按键的断码
			{	
				KeyInfo.nKeyType &= ~KEY_ASC;
				KeyInfo.nKeyType |= KEY_UP;
				KeyInfo.nVkValue =0x14;		//返回值为0x14
				nKeyValidate = 1;
				nScanType = FIRST_MAKE;
				Pause_flag = 1;		//当接收完pause按键数据时将Pause_flag置1，避免和下面的num按键的状态冲突
				break;
			}
			default: break;
		}
	}
	if(E1_flag != 1 && Pause_flag != 1)//E1_flag != 1是为了避免和case 0x14:冲突，因为pause数据中有0x14
	{								//Pause_flag != 1是为了避免和case 0x77:冲突，因为pause数据中有0x77
		switch(TmpChar)
		{
			case 0x12:		//按下shift
			case 0x59:
			{
				if((KeyInfo.nKeyType & KEY_UP) == KEY_UP) 
				{
					KeyInfo.nKeyType &= ~KEY_SHIFT;
				}
				else 
				{
					KeyInfo.nKeyType |= KEY_SHIFT;
				}
				nKeyValidate = 0;
				break;
			}
		
			case 0x11:		//按下alt
			{
				if((KeyInfo.nKeyType & KEY_UP) == KEY_UP) 
				{
					KeyInfo.nKeyType &= ~KEY_ALT;
				}
				else
				{ 
					KeyInfo.nKeyType |= KEY_ALT;
				}
				nKeyValidate = 0;
				break;
			}
		
			case 0x14:		//按下ctrl
			{
				if((KeyInfo.nKeyType & KEY_UP) == KEY_UP) 
				{
					KeyInfo.nKeyType &= ~KEY_CTRL;
				}
				else
				{ 
					KeyInfo.nKeyType |= KEY_CTRL;
				}
				nKeyValidate = 0;
				break;
			}
			case 0x58:		//caps lock
			{
				Caps_cnt ++;		
				if(Caps_cnt == 2)
				{
					Caps_cnt =0;
					Caps_status = !Caps_status;	
				}
				nKeyValidate = 0;
				break;
			}
			default: break;
		}
	}
	
	Pause_flag = 0;			//前面如果Pause_flag==1,在这儿将其置0
	return 1;
}

/****************************************************************************
* 名	称：GetKeyValue()
* 功	能：获取键值
* 入口参数：&g_KeyValue
* 出口参数：无
* 说	明：
****************************************************************************/

BOOL GetKeyValue(KEYINFO* pKeyValue) reentrant
{
	Ps2ClearInt();				//清除外部中断
	Ps2OpenInt();				//开启外部中断

	//等待接收到一个完整的按键
	while(nKeyValidate <= 0)
	{
		KeyScan();
	}

	Ps2CloseInt();			//关闭外部中断

	//装载键值
	*pKeyValue = KeyInfo;
	
	//清除，返回
	nKeyValidate = 0;
	
	return TRUE;

}
/****************************************************************************
* 名	称：uint8 code VkTable[136][3] 
* 功	能：第一类按键的键值表
* 说	明：
****************************************************************************/

uint8 code VkTable[136][3] = 
{
//	SHIFT未按下			SHIF按下		是否为char		扫描码索引
	{0,					0		,		0			},		//00
	{VK_F9,				VK_F9	,		0			},		//01	VK_F9
	{0,					0		,		0			},		//02
	{VK_F5,				VK_F5	,		0			},		//03	VK_F5
	{VK_F3,				VK_F3	,		0			},		//04	VK_F3
	{VK_F1,				VK_F1	,		0			},		//05	VK_F1
	{VK_F2,				VK_F2	,		0			},		//06	VK_F2
	{VK_F12,			VK_F12	,		0			},		//07	VK_F12
	{0,					0		,		0			},		//08
	{VK_F10,			VK_F10	,		0			},		//09	VK_F10
	{VK_F8,				VK_F8	,		0			},		//0A	VK_F8
	{VK_F6,				VK_F6	,		0			},		//0B	VK_F6
	{VK_F4,				VK_F4	,		0			},		//0C	VK_F4
	{VK_TAB,			VK_TAB	,		0x10		},		//0D	VK_TAB
	{'`',				'~'		,		0x10		},		//0E
	{0,					0		,		0			},		//0F
	{0,					0		,		0			},		//10
	{0,					0		,		0			},		//11
	{0,					0		,		0			},		//12
	{0,					0		,		0			},		//13
	{0,					0		,		0			},		//14
	{'q',				'Q'		,		0x10		},		//15
	{'1',				'!' 	,		0x10		},		//16
	{0,					0		,		0			},		//17
	{0,					0		,		0			},		//18
	{0,					0		,		0			},		//19
	{'z',				'Z'		,		0x10		},		//1A
	{'s',				'S'		,		0x10		},		//1B
	{'a',				'A'		,		0x10		},		//1C
	{'w',				'W'		,		0x10		},		//1D
	{'2',				'@'		,		0x10		},		//1E
	{0,					0		,		0			},		//1F
	{0,					0		,		0			},		//20
	{'c',				'C'		,		0x10		},		//21
	{'x',				'X'		,		0x10		},		//22
	{'d',				'D'		,		0x10		},		//23
	{'e',				'E'		,		0x10		},		//24
	{'4',				'$'		,		0x10		},		//25
	{'3',				'#'		,		0x10		},		//26
	{0,					0		,		0			},		//27
	{0,					0		,		0			},		//28
	{' ',				' '		,		0x10		},		//29   SPASE
	{'v',				'V'		,		0x10		},		//2A
	{'f',				'F'		,		0x10		},		//2B
	{'t',				'T'		,		0x10		},		//2C
	{'r',				'R'		,		0x10		},		//2D
	{'5',				'%'		,		0x10		},		//2E
	{0,					0		,		0			},		//2F
	{0,					0		,		0			},		//30
	{'n',				'N'		,		0x10		},		//31
	{'b',				'B'		,		0x10		},		//32
	{'h',				'H'		,		0x10		},		//33
	{'g',				'G'		,		0x10		},		//34
	{'y',				'Y'		,		0x10		},		//35
	{'6',				'^'		,		0x10		},		//36
	{0,					0		,		0			},		//37
	{0,					0		,		0			},		//38
	{0,					0		,		0			},		//39
	{'m',				'M'		,		0x10		},		//3A
	{'j',				'J'		,		0x10		},		//3B
	{'u',				'U'		,		0x10		},		//3C
	{'7',				'&'		,		0x10		},		//3D
	{'8',				'*'		,		0x10		},		//3E
	{0,					0		,		0			},		//3F
	{0,					0		,		0			},		//40
	{',',				'<'		,		0x10		},		//41
	{'k',				'K'		,		0x10		},		//42
	{'i',				'I'		,		0x10		},		//43
	{'o',				'O'		,		0x10		},		//44
	{'0',				')'		,		0x10		},		//45
	{'9',				'('		,		0x10		},		//46
	{0,					0		,		0			},		//47
	{0,					0		,		0			},		//48
	{'.',				'>'		,		0x10		},		//49
	{'/',				'?'		,		0x10		},		//4A
	{'l',				'L'		,		0x10		},		//4B
	{';',				':'		,		0x10		},		//4C
	{'p',				'P'		,		0x10		},		//4D
	{'-',				'_'		,		0x10		},		//4E
	{0,					0		,		0			},		//4F
	{0,					0		,		0			},		//50
	{0,					0		,		0			},		//51
	{'\'',		        0		,		0x10		},		//52	////
	{0,					0		,		0			},		//53
	{'[',				'{'		,		0x10		},		//54
	{'=',				'+'		,		0x10		},		//55
	{0,					0		,		0			},		//56
	{0,					0		,		0			},		//57
	{0,					0		,		0			},		//58
	{0,					0		,		0			},		//59
	{VK_ENTER,			VK_ENTER,		0x10		},		//5A	VK_ENTER
	{']',				'}'		,		0x10		},		//5B
	{0,					0		,		0			},		//5C
	{'\'',			    '|'		,		0x10		},		//5D	////
	{0,					0		,		0			},		//5E
	{0,					0		,		0			},		//5F
	{0,					0		,		0			},		//60
	{0,					0		,		0			},		//61
	{0,					0		,		0			},		//62
	{0,					0		,		0			},		//63
	{0,					0		,		0			},		//64
	{0,					0		,		0			},		//65
	{VK_BACK,			VK_BACK	,		0			},		//66	VK_BACK
	{0,					0		,		0			},		//67
	{0,					0		,		0			},		//68
	{'1',	     		35		,		0x10		},		//69
	{0,					0		,		0			},		//6A
	{'4',		   		37		,		0x10		},		//6B
	{'7',	    		0		,		0			},		//6C
	{0,					0		,		0			},		//6D
	{0,					0		,		0			},		//6E
	{0,					0		,		0			},		//6F
	{'0',		    	45		,		0x10		},		//70
	{'.', 				0		,		0			},		//71
	{'2',				40		,		0x10		},		//72
	{'5',				0		,		0x10		},		//73
	{'6',		    	39		,		0x10		},		//74
	{'8', 			    0   	,		0			},		//75
	{VK_ESCAPE,			VK_ESCAPE,		0x10		},		//76	VK_ESCAPE 
	{0,					0		,		0			},		//77
	{VK_F11,			VK_F11	,		0			},		//78	VK_F11
	{'+',				0		,		0			},		//79
	{'3',				34		,		0x10		},		//7A
	{'-',				0		,		0			},		//7B
	{'*',				0		,		0			},		//7C
	{'9',				0		,		0			},		//7D
	{VK_SCROLL,			VK_SCROLL,		0			},		//7E	scroll lock
	{0,					0		,		0			},		//7F
	{0,					0		,		0			},		//80
	{0,					0		,		0			},		//81
	{0,					0		,		0			},		//82
	{VK_F7,				VK_F7	,		0			},		//83	VK_F7
	{0,					0		,		0			}		//84


};
/****************************************************************************
* 名	称：uint8 code E0Table[128] 
* 功	能：第二类按键的键值表
* 说	明：
****************************************************************************/
uint8 code E0Table[128] = 
{
	0,			//00
	0,			//01
	0,			//02
	0,			//03
	0,			//04
	0,			//05
	0,			//06
	0,			//07
	0,			//08
	0,			//09
	0,			//0A
	0,			//0B
	0,			//0C
	0,			//0D
	0,			//0E
	0,			//0F
	0,			//10
	0,			//11
	0,			//12
	0,			//13
	0,			//14
	0,			//15
	0,			//16
	0,			//17
	0,			//18
	0,			//19
	0,			//1A
	0,			//1B
	0,			//1C
	0,			//1D
	0,			//1E
	VK_LWIN,	//1F	L GUI
	0,			//20
	0,			//21
	0,			//22
	0,			//23
	0,			//24
	0,			//25
	0,			//26
	VK_RWIN,	//27	R GUI
	0,			//28
	0,			//29
	0,			//2A
	0,			//2B
	0,			//2C
	0,			//2D
	0,			//2E
	0,			//2F
	0,			//30
	0,			//31
	0,			//32
	0,			//33
	0,			//34
	0,			//35
	0,			//36
	0,			//37
	0,			//38
	0,			//39
	0,			//3A
	0,			//3B		
	0,			//3C		
	0,			//3D		
	0,			//3E		
	0,			//3F
	0,			//40
	0,			//41
	0,			//42		
	0,			//43		
	0,			//44		
	0,			//45
	0,			//46		
	0,			//47
	0,			//48
	0,			//49
	0,			//4A
	0,			//4B		
	0,			//4C
	0,			//4D
	0,			//4E
	0,			//4F
	0,			//50
	0,			//51
	0,			//52
	0,			//53
	0,			//54
	0,			//55
	0,			//56
	0,			//57
	0,			//58
	0,			//59
	0,			//5A
	0,			//5B
	0,			//5C
	0,			//5D	////
	0,			//5E
	0,			//5F
	0,			//60
	0,			//61
	0,			//62
	0,			//63
	0,			//64
	0,			//65
	0,			//66
	0,			//67
	0,			//68
	VK_END,    	//69	END
	0,			//6A
	VK_LEFT,	//6B	LEFT ARROW
	VK_HOME,    //6C	HOME
	0,			//6D
	0,			//6E
	0,			//6F
	VK_INSERT,  //70	INSERT
	VK_DELETE,	//71	DELETE
	VK_DOWN,	//72	DOWN ARROW
	0,			//73
	VK_RIGHT,	//74	RIGH ARROW
	VK_UP, 		//75	UP ARROW
	0,			//76
	0,			//77
	0,			//78
	0,			//79
	VK_PGDOWN,	//7A	PAGE DOWN
	0,			//7B
	VK_PRINT,	//7C
	VK_PGUP,	//7D	PAGE UP
	0,			//7E
	0,			//7F
};