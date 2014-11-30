/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : ColBusSlaveMac.c
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/
#include <string.h>
#include "ColBusDef.h"
#include "CrcCheck.h"
#include "nwk.h"
#include "ColBusSlaveMac.h"

//ColBus���ݻ���������
#define CB_BUFF_SEND_MAX	128
#define CB_BUFF_RECV_MAX	128

//#define FLASH_ID_MODE
#ifdef FLASH_ID_MODE
uint16 code g_FlashID _at_ 0x0400;
#endif

//ColBus��ַ
#define CB_ADDR_LEN			15
static const uint8 m_nAddrMacLen = 4;											//ColBus�����ַ����
static uint8 m_arrAddrMac[CB_ADDR_LEN] = {5, 1, 1, 5};							//���豸��ַΪ0x05010101

static uint8 m_nAddrNetLen = 4;													//ColBus�����ַ����
static uint8 m_arrAddrNet[CB_ADDR_LEN] = {0, 0, 0, 0};					//��ʼ�������ַΪ��

//ColBus���ݻ���������
static uint8 m_buffCBSend[CB_BUFF_SEND_MAX];
static uint8 m_buffCBRecv[CB_BUFF_RECV_MAX];
static uint8 m_nCBControl = 0xC4;		//ColBus������

//���յĵ�ַ����
static uint8 m_nRecvMacLen;	
static uint8 m_nRecvNetLen;	

//�ڲ���������
int16 CBSlaveMacSend(uint8* pData, int16 nLen) reentrant;

//��ʼ��
void CBMacInit() reentrant
{
#ifdef FLASH_ID_MODE
	uint16 nID = g_FlashID;
									
	m_arrAddrMac[0] = LOBYTE(nID);
	m_arrAddrMac[1] = HIBYTE(nID);	
#endif
}

//Mac״̬����
uint8 CBSlaveMacFsm() reentrant
{
	uint8* pData = m_buffCBRecv + m_nAddrMacLen + 1;
	uint8 nCmd = pData[0];

	//���������ַ
	if(nCmd == CB_SET_ADDR)
	{
		//pData[1]ΪAddrLevel
		m_nAddrNetLen = pData[2];
		m_nAddrNetLen = (m_nAddrNetLen < CB_ADDR_LEN) ? m_nAddrNetLen : CB_ADDR_LEN;
		memcpy(m_arrAddrNet, pData + 3, m_nAddrNetLen);

		//�ظ�ԭָ��
		CBSlaveMacSend(pData, 1);

		return 1;
	}

	return 0;
}

//ColBus���豸���պ���
int16 CBSlaveNetRecv(uint8* pData, int16 nLen) reentrant
{
	int16 nRecv;
	int8 i;
	
	nLen = nLen;

	//�������������
	nRecv = NwkRecv(m_buffCBRecv, CB_BUFF_RECV_MAX);
	if(nRecv < 4) return nRecv;		//һ��ColBus���ݰ����ٰ��������֣�ָ���CRC��������4�ֽ�����

	//��ȡ������
	m_nCBControl = m_buffCBRecv[0];

	//У������䷽��
	if((m_nCBControl & MTS_MASK) == MTS_UP) return 0;

	//У�������Ϣ
	if((m_nCBControl & CODM_MASK) == CODM_MAC)
	{
		//MAC�����

		//У���ַ
		m_nRecvMacLen = m_nCBControl & CB_ADDR_SIZE_MASK;
		if(m_nRecvMacLen > m_nAddrMacLen) return 0; 				//��ֹ��ַ�������
		for(i = 0; i < m_nRecvMacLen; i++)
		{
			if(m_buffCBRecv[1+i] != m_arrAddrMac[i]) return 0;  	//��ַУ��ʧ��
		}
	
		//У��CRC
		if(CRC16(m_buffCBRecv, nRecv) != 0) return 0; //CRCУ��ʧ��
	
		//��������
		if(CBSlaveMacFsm() == 0)
		{
			//MAC��û�д�����APS�㴦��(������ֱ�ӵ�ַ���ʵ�APS��)
			nRecv = nRecv - m_nAddrMacLen - 3/*�����ֺ�CRC*/;  //�������ݺ�ָ��ĳ���
			memcpy(pData, m_buffCBRecv + 1 + m_nAddrMacLen, nRecv);

			return nRecv;
		}
		else
		{
			//���MAC�Ѵ���������Ҫ����㴦��
			return 0;
		}
	}	

	if((m_nCBControl & CODM_MASK) == CODM_NET)
	{
		//��������

		//У���ַ
		m_nRecvNetLen = m_nCBControl & CB_ADDR_SIZE_MASK;
		if(m_nRecvNetLen > m_nAddrNetLen) return 0; //��ֹ��ַ�������
		for(i = 0; i < m_nRecvNetLen; i++)
		{
			if(m_buffCBRecv[1+i] != m_arrAddrNet[i]) return 0;  //��ַУ��ʧ��
		}
	
		//У��CRC
		if(CRC16(m_buffCBRecv, nRecv) != 0) return 0; //CRCУ��ʧ��
	
		//��������
		nRecv = nRecv - m_nRecvNetLen - 3/*�����ֺ�CRC*/;  //�������ݺ�ָ��ĳ���
		memcpy(pData, m_buffCBRecv + 1 + m_nRecvNetLen, nRecv);
		return nRecv;
	}

	return 0;
}

//ColBus���豸���ͺ���
int16 CBSlaveMacSend(uint8* pData, int16 nLen) reentrant
{
	uint16 nCrc16;
	int8 i;

	//��������
 	m_nCBControl &= ~CODM_MASK;
	m_nCBControl |= CODM_MAC;

	//���䷽��
	m_nCBControl &= ~MTS_MASK;
	m_nCBControl |= MTS_UP;

	//��ַ����
	m_nRecvMacLen = (m_nRecvMacLen < CB_ADDR_LEN) ? m_nRecvMacLen : CB_ADDR_LEN;	//��ֹ��ַ�������
 	m_nCBControl &= ~CB_ADDR_SIZE_MASK;
	m_nCBControl |= m_nRecvMacLen;

	//���ÿ�����
	m_buffCBSend[0] = m_nCBControl;
	
	//���õ�ַ
	for(i = 0; i < m_nRecvMacLen; i++)
	{
		m_buffCBSend[1+i] = m_arrAddrMac[i];
	}

	//�������(ע�⣬δ��鷢�����Ƿ�����)
	memcpy(m_buffCBSend + 1 + m_nRecvMacLen, pData, nLen);

	//����CRCУ���ֶ�
	nCrc16 = CRC16(m_buffCBSend, 1 + m_nRecvMacLen + nLen);
	m_buffCBSend[1 + m_nRecvMacLen + nLen] = (uint8)((nCrc16 >> 8) & 0xFF);
	m_buffCBSend[2 + m_nRecvMacLen + nLen] = (uint8)(nCrc16 & 0xFF);

	//�������������
	NwkSend(m_buffCBSend, 3/*��������CRC*/ + m_nRecvMacLen + nLen);

	return nLen;
}

//ColBus���豸���ͺ���
int16 CBSlaveNetSend(uint8* pData, int16 nLen) reentrant
{
	uint16 nCrc16;
	int8 i;

	//��������
 	m_nCBControl &= ~CODM_MASK;
	m_nCBControl |= CODM_NET;

	//���䷽��
	m_nCBControl &= ~MTS_MASK;
	m_nCBControl |= MTS_UP;

	//��ַ����
	m_nRecvNetLen = (m_nRecvNetLen < CB_ADDR_LEN) ? m_nRecvNetLen : CB_ADDR_LEN;	//��ֹ��ַ�������
	m_nRecvNetLen = (m_nRecvNetLen == 0) ? m_nAddrNetLen : m_nRecvNetLen;			//��ֹ�յ�ַ���㲥��ѡָ��ʱ

 	m_nCBControl &= ~CB_ADDR_SIZE_MASK;
	m_nCBControl |= m_nRecvNetLen;

	//���ÿ�����
	m_buffCBSend[0] = m_nCBControl;
	
	//���õ�ַ
	for(i = 0; i < m_nRecvNetLen; i++)
	{
		m_buffCBSend[1+i] = m_arrAddrNet[i];
	}

	//�������(ע�⣬δ��鷢�����Ƿ�����)
	memcpy(m_buffCBSend + 1 + m_nRecvNetLen, pData, nLen);

	//����CRCУ���ֶ�
	nCrc16 = CRC16(m_buffCBSend, 1 + m_nRecvNetLen + nLen);
	m_buffCBSend[1 + m_nRecvNetLen + nLen] = (uint8)((nCrc16 >> 8) & 0xFF);
	m_buffCBSend[2 + m_nRecvNetLen + nLen] = (uint8)(nCrc16 & 0xFF);

	//�������������
	NwkSend(m_buffCBSend, 3/*��������CRC*/ + m_nRecvNetLen + nLen);

	return nLen;
}

//У�������ַ
BOOL CheckAddrNet(uint8* arrAddr, uint8 nLen)
{
	uint8 i;

	if(nLen == 0) return FALSE;
	if(nLen != m_nAddrNetLen) return FALSE;

	for(i = 0; i < nLen; i++)
	{
		if(arrAddr[i] != m_arrAddrNet[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}



