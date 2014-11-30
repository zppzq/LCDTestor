/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------文 件 信 息--------------------------------------------------------**
**文   件   名：															 **
**创   建   人：															 **
**创 建  时 间：2007.6.1													 **
**最后修改时间：													 		 **
**描        述：														 	 **
******************************************************************************/

//--------Includes-------------------------------------------------------------
#include "std\global.h"
#include "410DA.h"


/******************************************************************************
**名	称：																 
**功	能：																 
**入口参数：																 
**出口参数：															 
**说	明：															 
******************************************************************************/
#ifdef DA_INIT
void DAInit(void)
{
	IDA0CN = 0xF7;						// IDA0使能、写IDA0H更新、右对齐、2mA
	P0SKIP |= 0x01;

	IDA1CN = 0xF7;						// IDA1使能、写IDA1H更新、右对齐、2mA

	//REF0CN &= 0x7F;						// IDAMRG = 0; IDA1连到P0.1
	//P0SKIP |= 0x02;
	REF0CN |= 0x80;						// IDAMRG = 0; IDA1连到P0.1

}
#endif

/******************************************************************************
**名	称：																 
**功	能：																 
**入口参数：																 
**出口参数：															 
**说	明：															 
******************************************************************************/
#ifdef DA_OUT_0
void DAOut0(unsigned int DAValue)
{

	IDA0L = DAValue & 0x00FF;			// 写IDA0低8位
	IDA0H = (DAValue & 0xFF00) >> 8;	// 写IDA0高8位并更新输出

}
#endif

/******************************************************************************
**名	称：																 
**功	能：																 
**入口参数：																 
**出口参数：															 
**说	明：															 
******************************************************************************/
#ifdef DA_OUT_1
void DAOut1(unsigned int DAValue)
{
	IDA1L = DAValue & 0x00FF;			// 写IDA1低8位
	IDA1H = (DAValue & 0xFF00) >> 8;	// 写IDA1高8位并更新输出
}
#endif

/******************************************************************************
**名	称：CalDAValueAtom()																 
**功	能：计算原子化电流对应的DA值																 
**入口参数：原子化电流										 
**出口参数：对应的DA值													 
**说	明：无
******************************************************************************/
#ifdef CAL_DA_VALUE_ATOM
unsigned int CalDAValueAtom(float fValue)
{
	unsigned int nDAValue;
	unsigned char temp;
	//nDAValue = fValue/MAX_CURRENT * (0x0FA0 - 0x06AA) + 0x06AA;

	//限制电流
	if(fValue>20.0)
	{
		fValue = 20.0;
	}	
	temp = (unsigned char)(fValue/1);
	nDAValue = (fValue-DivCurrentValue[temp])/1.0 * (DivImitateValue[temp+1] - DivImitateValue[temp]) 
														+ DivImitateValue[temp];
	if(nDAValue <= DivImitateValue[0])
	{
		nDAValue = 0x0000;
	}
	if(nDAValue > 0x0FFF)
	{
		nDAValue = 0x0FFF;
	}
	return nDAValue;
}
#endif

/******************************************************************************
**名	称：																 
**功	能：																 
**入口参数：																 
**出口参数：															 
**说	明：															 
******************************************************************************/
#ifdef OUT_CURRENT_ATOM
void OutCurrentAtom(float fValue)
{
	unsigned int nDAValue;	
	
	nDAValue = CalDAValueAtom(fValue);
	DA_Out0(nDAValue);
	
	//--------处理指示灯-------------------------------------------------------
	if(nDAValue <= DivImitateValue[0])
	{
		LightAtomOff();
	}
	else
	{
		LightAtomOn();
	}
}
#endif
