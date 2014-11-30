/****************************************Copyright (c)************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------
**��   ��   ��: nrf905.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2006��6��25��
**����޸�����: 2007��5��7��
**��        ��: nrf905Դ�ļ�
******************************************************************************/
#define _NRF905_C_

#include "bsp.h"
#include "spi.h"
#include "nrf905.h"

//defines---------------------------------------
#define 	NOP		_nop_();_nop_();_nop_()

//�������
//#define 	USE_PA

//****************���������**************************************************
#define 	R_TX_ADDR	 	0x23		//��Զ�̵�ַ
#define  	W_TX_ADDR		0x22		//дԶ�̵�ַ
#define  	R_TX_DATA_F 	0x21		//������������
#define  	W_TX_DATA_F 	0x20		//д����������
#define  	R_CFG_F		 	0x10		//�����üĴ���
#define  	W_CFG_F			0x00		//д���üĴ���
#define  	RECEIVE_DATA 	0x24		//���յ�������

//*********************�ź�������*********************************************
//����ź�=====================================================
//nrf905Ƭѡ
#define CSN_PORT		PORT(2)
#define CSN				BIT(5)

//�ϵ�
#define POWER_UP_PORT	PORT(2)
#define POWER_UP		BIT(1)

//���ͻ����ѡ���ź�
#define TX_EN_PORT		PORT(1)
#define TX_EN			BIT(7)


//���ͻ����ʹ���ź�
#define TRX_CE_PORT		PORT(2)
#define TRX_CE			BIT(0)


//���ſ�������-------------
#ifdef USE_PA
//���ſ���
#define PA_PORT			PORT(1)
#define PA				BIT(0)

//��Ƶ���ؿ���
#define STX_PORT		PORT(2)
#define STX				BIT(7)

void NrfDelay(uint16 nTime);
#endif

//�����ź�======================================================
//�ز����
//#define CD_PORT			PORT(2)
//#define CD				BIT(1)

//��ַƥ��
//#define AM_PORT			PORT(3)
//#define AM				BIT(7)

//���ݷ�����ϻ������ȷ
//#define DR_PORT			PORT(2)
//#define DR				BIT(3)

#define DRINT_PORT      PORT(0)    //���ռ���ж�����
#define DRINT           BIT(1)


//���ݶ���*******************************************************************
#define	DEBUG_SPI	1

//�����ֻ�����
static INT8U xdata RfCfg[10];

//���͵�ַ������
static INT8U xdata TxAddr[4];

static OS_EVENT 	*pNrf905Event;						    	//�¼����ƿ�
static uint16 		nNrf905TimeOut;								//���ճ�ʱʱ��
static uint8 		nNrf905Err;									//�����־
//**********************��������*********************************************

/****************************************************************************
* ��	�ƣ�Nrf905PortInit()
* ��	�ܣ�nrf905�˿ڳ�ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _NRF905_INIT_
void Nrf905PortInit() reentrant
{
	//---------------------
	MakePushPull(CSN);
	MakePushPull(POWER_UP);
	MakePushPull(TX_EN);
	MakePushPull(TRX_CE);
	//---------------------
	//MakeOpenDrain(CD);
	//MakeOpenDrain(AM);
	//MakeOpenDrain(DR);
	//---------------------
	SetLo(POWER_UP);
	SetLo(TRX_CE);
	SetLo(TX_EN);
	SetHi(CSN);

	//���ſ���
#ifdef USE_PA
	MakePushPull(PA);
	MakePushPull(STX);
	SetLo(PA);
	SetHi(STX);	
#endif	

	//�жϳ�ʼ��
	IT0 = 1;
	IT01CF &= 0xF0;
	IT01CF |= 0x08;
	IT01CF	|= GetBitValue(DRINT);
	MakeOpenDrain(DRINT);

}
#endif

/****************************************************************************
* ��	�ƣ�Nrf905VariInit()
* ��	�ܣ�nrf905ȫ�ֱ�����ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _NRF905_INIT_
void Nrf905VariInit() reentrant
{
	pNrf905Event = NULL;
	nNrf905TimeOut = 0;
}
#endif

/****************************************************************************
* ��	�ƣ�Nrf905SetDcb()
* ��	�ܣ������豸���Ʋ���
* ��ڲ�����pBrfDcb���豸���Ʋ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _NRF905_INIT_
void Nrf905SetDcb(NRF905DCB *pBrfDcb)  reentrant
{
	unsigned char tmp;

	//����Ƶ�ʿ�����
	if(pBrfDcb->nTransFreq<4224) pBrfDcb->nTransFreq = 4224;
	else if(pBrfDcb->nTransFreq>4735) pBrfDcb->nTransFreq = 4735;
	
	tmp = ((pBrfDcb->nTransFreq-4224)>>8) & 0x00FF;
	RfCfg[1] = 0x0c;
	RfCfg[1] &= ~0x01;
	RfCfg[1] |= tmp;
	tmp = (pBrfDcb->nTransFreq-4224) & 0x00FF;
	RfCfg[0] = tmp;
	
	//����DNS����
	RfCfg[2] = 0x44;
	
	//���ñ���DNS
	*((uint32*)(RfCfg+5)) = pBrfDcb->LocalDNS;

	//����Զ��DNS
	*((uint32*)TxAddr) = pBrfDcb->RemoteDNS;

	//���ý������ݳ���
	RfCfg[3] = pBrfDcb->nRxLen;

	//���÷������ݳ���
	RfCfg[4] = pBrfDcb->nTxLen;

	//���ù����Լ�У��λ
	RfCfg[9] = 0x1B;
	
	//
	//����
	//
	nrf905_rw(W_CFG_F,RfCfg,10,0);
	nrf905_rw(W_TX_ADDR,TxAddr,4,0);

	//�����ź���
	if(pNrf905Event == NULL) 
	{
		pNrf905Event = OSSemCreate(0);
	}

#ifdef	DEBUG_SPI
	for(tmp=0;tmp<10;tmp++) RfCfg[tmp] = 0;	
	nrf905_rw(R_CFG_F,RfCfg,10,1);
#endif

}
#endif

/****************************************************************************
* ��	�ƣ�Nrf905SetRecvTimeOut()
* ��	�ܣ����ý��ճ�ʱʱ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	_NRF905_SET_RECV_TIME_OUT_
void Nrf905SetRecvTimeOut(uint16 nTimeOut) reentrant
{
	if(nTimeOut == 0)
	{
		nNrf905TimeOut = 0;
		return;
	}
	nNrf905TimeOut = (nTimeOut - 1) / 10 + 1;
}
#endif


/****************************************************************************
* ��	�ƣ�OpenNrf905()
* ��	�ܣ���905ͨ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _NRF905_INIT_
void OpenNrf905()  reentrant
{
	SetHi(POWER_UP);
}
#endif


/****************************************************************************
* ��	�ƣ�CloseNrf905()
* ��	�ܣ��ر�905ͨ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _NRF905_CLOSE_
void CloseNrf905() reentrant
{
	SetLo(POWER_UP);
}
#endif

/****************************************************************************
* ��	�ƣ�nrf905_IsFresh()
* ��	�ܣ�nrf905�ж��ŵ��Ƿ����
* ��ڲ�������
* ���ڲ�����ָʾ�ŵ��Ƿ���У�1�����У�2��������
* ˵	������
****************************************************************************/
#ifdef _NRF905_SET_IS_FRESH_
BOOL Nrf905IsFresh() reentrant
{
	return(!GetSignal(CD));
}
#endif

/****************************************************************************
* ��	�ƣ�nrf905_rw()
* ��	�ܣ�nrf905��д����
* ��ڲ�����ctr ������; pd ����д���ݵ�ָ��; nd ����д���ݵĳ���
* ���ڲ�������
* ˵	����ctr ���������Ѷ���
****************************************************************************/
#ifdef _NRF905_INIT_
static void nrf905_rw(uint8 ctr, uint8 *pd, uint16 nd, uint8 bNeedRead) reentrant
{
	uint16 i;
	uint8  trst;
	
	SetLo(CSN);
	NOP;
	spi_rw(ctr);
	for(i=0;i<nd;i++)
	{
		trst = spi_rw(pd[i]);
		if(bNeedRead)
		{
			pd[i] = trst;
		}
		NOP;
	}
	SetHi(CSN);
	NOP;
}
#endif

/****************************************************************************
* ��	�ƣ�Nrf905ReadData()
* ��	�ܣ�nrf905���������ݺ���
* ��ڲ�����pd �������ݵ�ָ��; nd �������ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _NRF905_READ_
void Nrf905ReadData(uint8 *pd,uint16 nd) reentrant
{
	nrf905_rw(RECEIVE_DATA,pd,nd,1);
}
#endif

/****************************************************************************
* ��	�ƣ�Nrf905Send()
* ��	�ܣ�nrf905�������ݺ���
* ��ڲ�����pd ��д���ݵ�ָ��; nd ��д���ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _NRF905_SEND_
void Nrf905Send(uint8 *pd,uint16 nd) reentrant
{
	//д�������ݵ����ͻ�����
	nrf905_rw(W_TX_DATA_F,pd,nd,0);

	//������
#ifdef USE_PA
	SetLo(STX);
	SetHi(PA);
	NrfDelay(200);	
#endif

	//����
	SetHi(TX_EN);
	SetHi(TRX_CE);

	//���ԭ���жϱ�־��ʹ���ж�
	IE0 = 0;
	EX0 = 1;

	//�޳�ʱ�ȴ��������
	OSSemPend(pNrf905Event, 0, &nNrf905Err);
	
	//�رշ���
	SetLo(TX_EN);
	//���жϴ��������ͨ�ű��ضϣ�����Ͳ���Ҫ��ͨ����
}
#endif

/****************************************************************************
* ��	�ƣ�Nrf905SendCarry()
* ��	�ܣ������ز�����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	_NRF905_SEND_CARRY_
void Nrf905SendCarry(uint16 nCycles) reentrant
{
	SetHi(TX_EN);
	SetHi(TRX_CE);

	//���������ز�	
	if(nCycles == 0) while(1);
	OSTimeDly(nCycles);

	SetLo(TX_EN);
	SetLo(TRX_CE);
}
#endif
/****************************************************************************
* ��	�ƣ�Nrf905Receive()
* ��	�ܣ�nrf905���ճ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	_NRF905_RECEIVE_
BOOL Nrf905Receive() reentrant
{
	//��������
	SetLo(TX_EN);
	SetHi(TRX_CE);
	
	//���ԭ���жϱ�־��ʹ���ж�
	IE0 = 0;
	EX0 = 1;

	//�ȴ��������
	OSSemPend(pNrf905Event, nNrf905TimeOut, &nNrf905Err);

	//���жϴ��������ͨ�ű��ضϣ�����Ͳ���Ҫ��ͨ����

	//
	//ע�⣺���������ݺ��ܹر�POWER_UP���������������ȫΪ0��
	//

	if(nNrf905Err == OS_NO_ERR) 
	{
		return 1;
	}
	return 0;
}
#endif

/*****************************************************************************************************************
* ��	��	Int0ISR()
* ��	�ܣ�����/�����жϳ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void Int0ISR() interrupt 0
{		 
	OSIntEnter();				//�����ж�
	EX0 = 0;					//��ֹ�ⲿ�ж�0�ж�

	SetLo(TRX_CE);				//�ر�905ͨ��

	//�رտ���
#ifdef USE_PA
	SetLo(PA);
	SetHi(STX);						   
#endif

	OSSemPost(pNrf905Event);	//���ͻ���ճ������ź���
	OSIntExit();				//�˳��ж�
}


/*****************************************************************************************************************
* ��	��	NrfDelay()
* ��	�ܣ���ʱ����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef USE_PA
static void NrfDelay(uint16 nTime)
{
	while(nTime--);
}	
#endif
