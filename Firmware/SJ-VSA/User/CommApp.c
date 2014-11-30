/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: CommApp.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��09��18��
**����޸�����: 2007��09��18��
**��        ��: ͨ��Ӧ��Դ�ļ�
*****************************************************************************************************************/
#define _USERCOMM_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "DataManage.h"
#include "CommApp.h"
#include "DataAcq.h"
#include "ColBusSlaveMac.h"
#include "Bsp\LowPower.h"
#include "SyncTimer.h"


//defines
#define COLLECTOR_MODEL			201

#define USE_STATIC_PREPARE

#define ENABLE_LACAL_ZEROED		1					//���ص���ʹ��
#define ENABLE_CARRY_SEND		1					//�ز�ʹ��

//��ʱʱ�䶨��
#define SLEEP_RECV_TIMEOUT		500					//����ʱ���ճ�ʱʱ�䣬��λΪms
#define NORMAL_RECV_TIMEOUT		5000				//��̬�½��ճ�ʱʱ�䣬��λΪms


//����ʱ����ƣ��˲�������NORMAL_RECV_TIMEOUTΪ����ǰʱ��============================
#define	SLEEP_POINT			60

//-----------------------------------------------------------------------------------	
//ȫ�ֱ���===========================================================================
uint16 g_nSleepPoint = SLEEP_POINT;
uint16 g_nSleepTicketMax = 1;


//4432������Ϣ
#ifdef _SI4432_H_

//�����豸������
WIRELESSDCB *pWirelessDCB;							//�����豸ָ��

//Ƶ�ʲ���
uint16 nDefaultFreq;								//Ĭ��Ƶ��
uint16 nCurFreq;									//��ǰƵ��
uint16 nCarryFreq;									//��ǰ�ز�Ƶ��
uint16 nCarrySendCycles;							//�ز���������
#endif


//�ⲿ��������==================================

extern uint16	g_nSleepPoint;					    //���֮������(��λΪ5S)
extern uint16	g_nSleepTicketMax;					//����ʱ��Ϊ���(��λΪ5S)

extern uint32 g_nStoreSectStart[DATA_ACQ_COUNT];				//��ǰ����εĿ�ʼ����
extern uint32 g_nStoreSectLen[DATA_ACQ_COUNT];					//��ǰ����εĳ���

extern float idata g_fMesureZeroVol[DATA_ACQ_COUNT];			//�洢���洢���еĵ����ѹ
extern DT_STORE idata g_sMesureZero[DATA_ACQ_COUNT];			//�������ݣ����ڼ���Ĳ���


//���߿���
uint16 nSleepRecvTimout = SLEEP_RECV_TIMEOUT;		//����ʱ���ճ�ʱʱ�䣬��λΪms
uint16 nNormalRecvTimeout = NORMAL_RECV_TIMEOUT;	//��̬�½��ճ�ʱʱ�䣬��λΪms





/****************************************************************************
* ��	�ƣ�SetNormalRecvTimeOut()
* ��	�ܣ����������Ľ��ճ�ʱʱ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SetNormalRecvTimeOut() reentrant
{
	//ע���������ʱ����ɼ�������ʱ���й�
#ifdef _SI4432_H_
	WirelessSetRecvTimeOut(nNormalRecvTimeout);
#endif
	
#ifdef _USE_COMM2_
	SetCommRecvTimeOut(2, nNormalRecvTimeout);
#endif
}

/****************************************************************************
* ��	�ƣ�SetSleepRecvTimeOut()
* ��	�ܣ�����˯��ʱ�Ľ��ճ�ʱʱ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SetSleepRecvTimeOut() reentrant
{
	//ע���������ʱ����ɼ�������ʱ���й�
#ifdef _SI4432_H_
	WirelessSetRecvTimeOut(nSleepRecvTimout);
#endif

#ifdef _USE_COMM2_
	SetCommRecvTimeOut(2, nSleepRecvTimout);
#endif

}

/****************************************************************************
* ��	�ƣ�SetFreqToDefault()
* ��	�ܣ�Ƶ������ΪĬ��Ƶ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_H_
void SetFreqToDefault() reentrant
{
	//����ΪĬ��Ƶ��
	WirelessSetNeedFreq(nDefaultFreq);
}
#endif

/****************************************************************************
* ��	�ƣ�SetFreqToCur()
* ��	�ܣ�Ƶ������Ϊ��ǰƵ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_H_
void SetFreqToCur() reentrant
{
	//Ƶ������
	WirelessSetNeedFreq(nCurFreq);
}
#endif

/***************************************************************************
* ��	�ƣ�UserCommInit()
* ��	�ܣ�ͨ��Ӧ�ò��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/  
void UserCommInit() reentrant
{
	//����ͨ�Ų���
#ifdef _SI4432_H_
	pWirelessDCB = Si4432GetDCB();
	nDefaultFreq = FREQ_DEVICE_DEFAULT;		//����Ĭ��Ƶ��
	nCurFreq = nDefaultFreq;				//��ǰΪĬ��Ƶ��
	pWirelessDCB->nFreq = nDefaultFreq;		//ͨ��Ƶ������
#endif

	//���ó��泬ʱʱ��
	SetNormalRecvTimeOut();

	//ColBus��ʼ��
	CBSlaveApsInit();
}


/***************************************************************************
* ��	�ƣ�CommProcess()
* ��	�ܣ�ͨ��������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
//���߻��ѿ���
int16	nWakeCount = 0;
BOOL	bSleep = FALSE;

//���߿���
uint16 g_nSleepIndex;
void CommProcess(void)
{
	int16 nRecvLen;

	//������������
	nRecvLen = CBSlaveApsFsm();

	//����յ����ݣ�ִ��ͨ��Э��
	if(nRecvLen > 0)
	{
		//�ź�ָʾ
		PostLightOn(1);

		//�������δ�������ݵĴ���
		nWakeCount = 0;

		//�ر�˯�߱�־
		if(bSleep == TRUE)
		{
			SetNormalRecvTimeOut();
			bSleep = FALSE;
		}

		//���õ���ǰƵ��
		#ifdef _SI4432_H_
		SetFreqToCur();
		#endif

		return;				 
	}

	//������ճ�ʱ��ͳ�Ƴ�ʱ����=====================================================
	if(nRecvLen  == -1)
	{
 		//���û�յ����ݣ�ͳ���޽��մ���=============================================
		if(bSleep == FALSE)
		{
			nWakeCount++;
			if(nWakeCount >= g_nSleepPoint)
			{
				//��������ʱ�ĳ�ʱʱ��
				SetSleepRecvTimeOut();			
	
				#ifdef _SI4432_H_
				//���õ�Ĭ��Ƶ��
				SetFreqToDefault();
				#endif
	
				//��������״̬
				bSleep = TRUE;
			}
		}

		//���������״̬��������5����================================================
		if(bSleep == TRUE)
		{
			#ifdef _SI4432_H_
			//��Դ���ƣ�׼����������ģʽ
			Si4432Standby();
			#endif
			
			PowerLightOff();
			SigLightOff();

			#ifdef _MCU_ENTER_SLEEP_
			MCUEnterSleep();
			#endif
			
			//��������ģʽ
			g_nSleepIndex = g_nSleepTicketMax;
			while(g_nSleepIndex--)
			{
				MCUSleep();
			}

			#ifdef _MCU_ENTER_SLEEP_
			MCUExitSleep();	 
			#endif

			#ifdef _SI4432_H_
			//�ָ�����ǰ״̬
			Si4432Ready();
			#endif

			//��һ��ָʾ��
			PostLightOn(1);
		}
	} 
}



