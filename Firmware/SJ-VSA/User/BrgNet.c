/****************************************Copyright (c)**************************************************
**                              
**                                
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: BrgNet.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��4��7��
**����޸�����: 2007��4��7��
**��        ��: 120.11ͨ��Э������������Э��
********************************************************************************************************/
#define		_BRG_NET_C_

//ͷ�ļ�
#include "includes.h"
#include "BrgNet.h"


//���²�Э�麯���ӿ�


//���ݶ���
//#define Flash_ID_Mode
#ifdef Flash_ID_Mode								 
uint16	code 		g_FlashBrgID  _at_ 0x0400;
#endif

//ָ������ID��
uint16 idata 		g_nCollectorLocalID = 56;

//���յ���ID��
uint16  		g_nNetID;			//�����ܵ�ַ
uint16  		g_nRelayID;			//��ת����ַ
uint16  		g_nCollectorID;		//�ɼ�����ַ
uint8 	 		g_nOutChannelID;	//�ⲿͨ����
uint8 idata		g_nChannelID;		//ͨ���ŵ�ַ
		  														   
/****************************************************************************
* ��	�ƣ�GetCollectorLocalID()
* ��	�ܣ���ȡ�ɼ������ص�ַ
* ��ڲ�������
* ���ڲ������ɼ���ID
* ˵	������
****************************************************************************/
#ifdef	_GET_LOCAL_ID_
uint16 GetCollectorLocalID() reentrant
{
#ifdef Flash_ID_Mode
	g_nCollectorLocalID = g_FlashBrgID;	
#endif
	
	return g_nCollectorLocalID;
}
#endif

/****************************************************************************
* ��	�ƣ�GetInnerChannel()
* ��	�ܣ���ȡ�ڲ�ͨ����
* ��ڲ������ⲿͨ����1 ~ 4
* ���ڲ������ڲ�ͨ����0 ~ 3
* ˵	������
****************************************************************************/
BOOL GetInnerChannel(INT8U nChannelNum) reentrant
{
	if(0xFF == nChannelNum)	return 0;	//�����0xFF���򷵻�0
	return (nChannelNum -1);
}


/****************************************************************************
* ��	�ƣ�BrgNetRecv()
* ��	�ܣ�102.11��������Э��
* ��ڲ�����pBuff  �����յ�����
			nLen   ���ݳ���
* ���ڲ�����0   ���ݳ���
			��0 ��������
* ˵	������
****************************************************************************/
#ifdef	_BRG_NET_RECV_
BOOL BrgNetRecv(uint8 *pBuff,uint16 nLen) reentrant
{
	//��ʱ����
	uint8 i;

#ifdef Flash_ID_Mode
	g_nCollectorLocalID = g_FlashBrgID;	
#endif

	//��ȡ��������
	WirelessReadData(pBuff, nLen+BRG_ADRC_LEN);

	//CRCУ�飬��������򷵻�false
	if(CRC16(pBuff,nLen+BRG_ADRC_LEN) != 0) 
	{
		_nop_();
		return FALSE;
	}

	//��ȡID	
	g_nNetID = ntohs(*((uint16*)(pBuff)));
	g_nRelayID = g_nNetID & RELAY_NET_ID;
	g_nCollectorID = g_nNetID & COLLECTOR_NET_ID;
	g_nOutChannelID = pBuff[BRG_CHANNEL_OFFSET];
	g_nChannelID = GetInnerChannel(g_nOutChannelID);

	//����Ƿ���б�����������ǣ�����false
	if((g_nCollectorID == g_nCollectorLocalID) || (g_nCollectorID == COLLECTOR_NET_ID))
	{
		//�������ݵ�Ӧ�ò�
		for(i = 0; i < nLen; i++)
		{
			pBuff[i] = pBuff[i + BRG_ADDR_LEN];
		}
		return TRUE;
	}
	return FALSE;
}
#endif


/****************************************************************************
* ��	�ƣ�BrgNetSend()
* ��	�ܣ�102.11����㷢��Э��
* ��ڲ�����pBuff  �����յ�����
			nLen   ���ݳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	_BRG_NET_SEND_
void BrgNetSend(uint8 *pBuff,uint16 nLen) reentrant
{ 
#ifdef Flash_ID_Mode
	g_nCollectorLocalID = g_FlashBrgID;	
#endif

	//�ƶ�����ָ�뵽֡ͷ��
	pBuff -= BRG_ADDR_LEN;

	//���ID��Ϣ
	*((uint16*)(pBuff)) = htons(g_nCollectorLocalID | g_nRelayID);
	pBuff[BRG_CHANNEL_OFFSET] = g_nOutChannelID;

	nLen += BRG_ADDR_LEN;

	//���CRCУ����
	*((uint16*)(pBuff+nLen)) = htons(CRC16(pBuff,nLen));
	nLen += BRG_CRC_LEN;
	
	//��������
	WirelessSend(pBuff,nLen);
}
#endif

#ifdef	_SET_SEND_CHANNEL_ID_
void SetSendChannelID(uint8 nChannelID) reentrant
{
	g_nOutChannelID = nChannelID;
}
#endif

