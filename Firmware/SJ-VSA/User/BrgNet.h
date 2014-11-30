/****************************************Copyright (c)**************************************************
**                              
**                               
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: BrgNet.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��4��7��
**����޸�����: 2007��4��7��
**��        ��: 120.11ͨ��Э������������Э��
********************************************************************************************************/
#ifndef		_BRG_NET_H_
#define		_BRG_NET_H_

#ifdef 		_BRG_NET_C_
#define		BRG_NET_EXT
#else
#define		BRG_NET_EXT	extern
#endif

//�����������
#define	_BRG_NET_RECV_
#define	_BRG_NET_SEND_
#define	_GET_LOCAL_ID_
#define	_SET_SEND_CHANNEL_ID_


#define	BRG_NET_DEBUG		0

//ȫ�ֱ�������
extern uint8 idata	g_nChannelID;		//ͨ���ŵ�ַ

BRG_NET_EXT uint16 GetCollectorLocalID(void) reentrant;
BRG_NET_EXT BOOL BrgNetRecv(uint8 *pBuff,uint16 nLen) reentrant;
BRG_NET_EXT void BrgNetSend(uint8 *pBuff,uint16 nLen) reentrant;
void SetSendChannelID(uint8 nChannelID) reentrant;

//**************************************************************************************************
#endif
