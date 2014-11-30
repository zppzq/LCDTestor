/****************************************文件信息**************************************************                      
**
**文   件   名: LCDDriver.c
**创   建   人: 黄安源
**创 建 日  期: 2007年7月16日
**最后修改日期: 2007年8月 4日
**描        述: LCD驱动源文件

**改		编: 杨承凯
**改 编  日 期: 2008年4月4日
***************************************************************************************************/
#define	_LCDDriver_C_

#include "..\BSP\bsp.h"
#include "LCDDriver.h"

//LCD并行数据接口，8BIT
#define  LCD_DATA       P4

//RS数据指令选择信号 1:数据,0:指令
#define LCD_RS_PORT	    PORT(2)
#define LCD_RS			BIT(5)

//读写选择信号 1:读,0:写
#define LCD_RW_PORT	    PORT(2)
#define LCD_RW			BIT(0)

//读写使能信号 写操作E下降缘有效,读操作E高电平有效
#define LCD_EN_PORT	    PORT(2)
#define LCD_EN			BIT(4)

//LCD串并选择
#define	LCD_PSB_PORT	PORT(2)
#define	LCD_PSB			BIT(1)	


//数据定义
static INT8U xdata CharBuff[66];

/****************************************************************************
* 名	称：LCDDriverInit()
* 功	能：LCD端口初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef		LCDDRIVER_INIT
void LcdDriverInit() reentrant
{
	//端口配置
	PortPushPull(LCD_DATA);
	MakePushPull(LCD_RS);
	MakePushPull(LCD_RW);
	MakePushPull(LCD_EN);
	MakePushPull(LCD_PSB);

	//初始化电平
	SetHi(LCD_RS);
	SetHi(LCD_RW);
	SetLo(LCD_EN);
	SetHi(LCD_PSB);
}
#endif


/****************************************************************************
* 名	称：LcdReadState()
* 功	能：LCD读状态函数
* 入口参数：无
* 出口参数：TmpData ,读出LCD忙闲状态和地址
* 说	明：无
****************************************************************************/
#ifdef	LCDDRIVER_READ_STATE
INT8U LcdReadState() reentrant
{
	INT8U TmpData;
	
	//调整端口
	PortOpenDrain(LCD_DATA);
	LCD_DATA = 0xFF;

	//读端口
	SetLo(LCD_RS);
	SetHi(LCD_RW);
	SetHi(LCD_EN);	
	TmpData = LCD_DATA;
	SetLo(LCD_EN);

	//调整端口
	PortPushPull(LCD_DATA);

	return TmpData;
}
#endif

/****************************************************************************
* 名	称：LcdIsBusy()
* 功	能：LCD忙状态判断函数
* 入口参数：无
* 出口参数：LCD忙标志, 1:忙 0:闲
* 说	明：无
****************************************************************************/
#ifdef	LCDDRIVER_IS_BUSY
BOOL LcdIsBusy() reentrant
{
	INT8U Tmp;
	Tmp = LcdReadState();
	return ((Tmp & 0x80) > 0)?TRUE:FALSE;
}
#endif


/****************************************************************************
* 名	称：LcdReadAC()
* 功	能：LCD读位地址计数器AC的值函数
* 入口参数：无
* 出口参数：位地址计数器AC的值
* 说	明：无
****************************************************************************/
#ifdef	LCDDRIVER_READ_AC
INT8U LcdReadAC() reentrant
{
	INT8U Tmp;
    while(LcdIsBusy() == TRUE);
    Tmp = LcdReadState();
	return (Tmp & 0x7F);
}
#endif


/****************************************************************************
* 名	称：LcdWriteCmd()
* 功	能：LCD读位地址计数器AC的值函数
* 入口参数：cmd，指令
* 出口参数：无
* 说	 
****************************************************************************/
#ifdef	LCDDRIVER_WRITE_CMD
void LcdWriteCmd(INT8U cmd) reentrant
{
	while(LcdIsBusy() == TRUE);

	SetLo(LCD_RS);
	SetLo(LCD_RW);
	SetHi(LCD_EN);
	LCD_DATA = cmd;
	SetLo(LCD_EN);
}
#endif


/****************************************************************************
* 名	称：LcdWriteData()
* 功	能：LCD写数据函数
* 入口参数：dat，数据
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	LCDDRIVER_WRITE_DATA
void LcdWriteData(INT8U dat) reentrant
{
	while(LcdIsBusy() == true);

	SetHi(LCD_RS);
	SetLo(LCD_RW);
	SetHi(LCD_EN);
	LCD_DATA = dat;
	SetLo(LCD_EN);
}
#endif


/****************************************************************************
* 名	称：LcdReadData()
* 功	能：LCD读数据函数
* 入口参数：无
* 出口参数：pData ,读出数据 nLen 数据长度
* 说	明：无
****************************************************************************/
#ifdef	LCDDRIVER_READ_DATA
void LcdReadData( INT8U* pData, INT8U nLen) reentrant
{	
    INT8U i;

	//调整端口
	PortOpenDrain(LCD_DATA);
	LCD_DATA = 0xFF;
	
	//假读
	SetHi(LCD_RS);  
	SetHi(LCD_RW);
    LCD_DATA = 0xff;
	SetHi(LCD_EN);
	SetLo(LCD_EN);

	//真读
	for(i = 0; i < nLen; i++)
	{
		SetHi(LCD_EN);
		pData[i] = LCD_DATA;
		SetLo(LCD_EN);
	}

	//调整端口
	PortPushPull(LCD_DATA);
}
#endif




/****************************************************************************
* 名	称：LcdDelaynus()
* 功	能：延时 n us
* 入口参数：ntime 延时的时间
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef LCD_DELAY_NUS
void LcdDelaynus(INT16U ntime) reentrant
{
   INT16U i;
   i = 6 * ntime;
   while(i--); 
}
#endif
