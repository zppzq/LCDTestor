/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: YhComm.c
**创   建   人: 杨承凯
**创 建 日  期: 2007年09月18日
**最后修改日期: 2007年09月18日
**描        述: 湘银河通信协议头文件
*****************************************************************************************************************/
#define 	_YHCOMM_C_

//includes--------------------------------------------------------------------------------------------------------
#include "..\sdk\global.h"
#include "YhComm.h"

static char YhSendDataBuff[32];
static char YhRecvDataBuff[64];

/****************************************************************************
* 名	称：YhCheck()
* 功	能：通信帧校验程序
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
uint8 YhCheck(uint8* pData, uint16 nLen)
{
	int i = 0; 
	uint8 nRst = pData[0];

	for(i = 1; i < nLen; i++)
	{
	   nRst = nRst^pData[i];
	}
	return nRst;
}

/****************************************************************************
* 名	称：YhSend()
* 功	能：向传感器发送数据
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void YhSend() reentrant
{
	YhSendDataBuff[0] = 0xAA;
	YhSendDataBuff[1] = 0x75;
	YhSendDataBuff[2] = 0x10;
	YhSendDataBuff[3] = 0;
	YhSendDataBuff[4] = 0x0E;
	YhSendDataBuff[5] = 0;

	YhSendDataBuff[6] = 0;
	YhSendDataBuff[7] = 0;
	YhSendDataBuff[8] = 0;
	YhSendDataBuff[9] = 0;
	YhSendDataBuff[10] = 0;
	YhSendDataBuff[11] = 0;
	YhSendDataBuff[12] = 0;
	YhSendDataBuff[13] = 0;

	YhSendDataBuff[14] = 0x08;
	YhSendDataBuff[15] = 0x10;
	YhSendDataBuff[16] = 0x12;
	YhSendDataBuff[17] = 0x01;
	YhSendDataBuff[18] = 0x01;
	YhSendDataBuff[19] = 0x01;
	YhSendDataBuff[20] = YhCheck(YhSendDataBuff, 20); 
	
	SendComm1(YhSendDataBuff,21); 

	while(IsComm1SendEnd() == FALSE);

}


/****************************************************************************
* 名	称：YhRecv()
* 功	能：读取传感器数据
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
static int16 nTemper;
static float fMesure;
static int16 nOffset;
static int16 nOffset;
static int16 nFreq;
static int16 nFreqBack; 
static int16 nMesure = 0;
uint16 YhRecv() reentrant
{
	uint16 nLen;
	
	//接收数据
	nLen = ReceiveComm1();

	if(nLen > 0)
	{
		//读取数据
   		ReadComm1Data(YhRecvDataBuff, nLen);

	    //解析数据
		nTemper =  *(int16*)(YhRecvDataBuff + 20);
		nMesure =  *(int16*)(YhRecvDataBuff + 22);
		nOffset =  *(int16*)(YhRecvDataBuff + 24);
		nFreq =  *(int16*)(YhRecvDataBuff + 26);
		nFreqBack =  *(int16*)(YhRecvDataBuff + 28);
	}

	return nMesure;
}

/****************************************************************************
* 名	称：YhAsk()
* 功	能：获取传感器数据
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
uint16 YhAsk() reentrant
{
	YhSend();
	return YhRecv();
}




