/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: YhComm.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��09��18��
**����޸�����: 2007��09��18��
**��        ��: ������ͨ��Э��ͷ�ļ�
*****************************************************************************************************************/
#define 	_YHCOMM_C_

//includes--------------------------------------------------------------------------------------------------------
#include "..\sdk\global.h"
#include "YhComm.h"

static char YhSendDataBuff[32];
static char YhRecvDataBuff[64];

/****************************************************************************
* ��	�ƣ�YhCheck()
* ��	�ܣ�ͨ��֡У�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�YhSend()
* ��	�ܣ��򴫸�����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�YhRecv()
* ��	�ܣ���ȡ����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
	
	//��������
	nLen = ReceiveComm1();

	if(nLen > 0)
	{
		//��ȡ����
   		ReadComm1Data(YhRecvDataBuff, nLen);

	    //��������
		nTemper =  *(int16*)(YhRecvDataBuff + 20);
		nMesure =  *(int16*)(YhRecvDataBuff + 22);
		nOffset =  *(int16*)(YhRecvDataBuff + 24);
		nFreq =  *(int16*)(YhRecvDataBuff + 26);
		nFreqBack =  *(int16*)(YhRecvDataBuff + 28);
	}

	return nMesure;
}

/****************************************************************************
* ��	�ƣ�YhAsk()
* ��	�ܣ���ȡ����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
uint16 YhAsk() reentrant
{
	YhSend();
	return YhRecv();
}




