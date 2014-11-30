/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: UserComm.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��09��18��
**����޸�����: 2007��09��18��
**��        ��: ������Դ�ļ�
*****************************************************************************************************************/
#define _USERCOMM_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "UserComm.h"
//#include "BrgNet.h"
//#include "DataManage.h"
#include "Sensor.h"

//defines
#define COLLECTOR_MODEL			201

#define ENABLE_LACAL_ZEROED		1					//���ص���ʹ��
#define ENABLE_CARRY_SEND		1					//�ز�ʹ��
//#define DEBUG_RECV_SYNC_INFOR	1					//�Ƿ�ش�ͬ����Ϣ

//��ʱʱ�䶨��
#define SLEEP_RECV_TIMEOUT		200					//����ʱ���ճ�ʱʱ�䣬��λΪms
#define NORMAL_RECV_TIMEOUT		5000				//��̬�½��ճ�ʱʱ�䣬��λΪms

#define DEVICE_WRONG_ORDER		0

//signal defines--------------------------------------------------------------------------------------------------
//****************************************************************************************************************
//��Ҫ��Ч���������
uint8 	 g_nState = 0;							//�ɼ���״̬��
uint8 	 Timer3Counter = 0;						//��ʱ��3�ֶμ�����
uint8 	 Timer3CounterMax = 4;					//��ʱ��3�ֶμ��������ֵ
BOOL  	 bDisableDynamic = TRUE;					//��ֹ��ʹ�ܶ�̬�ɼ�
uint16 	 nRequireCollector = 0;					//��̬�ɼ�ʱ�����еĲɼ�����
uint16 	 nSyncCode;								//ͬ��������
uint32 	 nRequireAddr = 0;						//��̬�ɼ�ʱ�����е����ݵ�ַ
uint8 	 nHostReadBytes = 0;						//�ϼ�Ҫ��ȡ�����ݸ���
uint8 	 g_nDynChannelID = 0;					//��̬�ɼ���������ͨ����
uint32 	 nHostDeviceByteDiff = 0;				//�����ͱ��ص����ݸ�����ֵ


//Timer3����========================================================================
#define	Timer3Stop()		TMR3CN &= ~0x04			//ֹͣ��ʱ��3
#define	Timer3Start()		TMR3CN |= 0x04			//����ʱ��3
#define	CallTimer3ISR()		TMR3CN |= 0x80			//���ö�ʱ��3�ж�
#define	ClearTimer3ISR()	TMR3CN &= ~0x80			//�����ʱ��3�ж�
#define	CloseTimer3ISR()	EIE1 &= ~0x80			//�ض�ʱ��3�ж�
#define	OpenTimer3ISR()		EIE1 |= 0x80			//����ʱ��3�ж�
uint8 TH3RL = 0;									//��ʱ��3���ֽ�����ֵ
uint8 TL3RL = 0;									//��ʱ��3���ֽ�����ֵ
uint32 nRateTime = 0;								//��¼�������һ�β�������Ĳ�������/ 4
uint32 nHostTime = 0;								//��¼��������ʱ�䣬��λ100us
uint32 nLocalTime = 0;								//��¼����Ӧ�ø��µ���ʱ�䣬��λ100us
uint32 nCycleTimeLeft = 0;							//��¼����Ӧ�ø���ʱ��Ļ�������
uint32 nCycleTicketLeft = 0;						//��¼����Ӧ�ø���ʱ��Ļ�������
uint32 nTicketPer100us;								//ÿ100us�Ķ�ʱ������
float fTicketPer100us;								//ÿ100us�Ķ�ʱ������(������)

//���߲��ֲ�������=================================
static uint8 NrfRxBuff[64];							//�ϲ����ݽ��ջ�����
static uint8 NrfTxBuff[128];						//�ϲ����ݷ��ͻ�����

//4432������Ϣ
WIRELESSDCB *pWirelessDCB;							//�����豸ָ��

//Ƶ�ʲ���
uint16 nDefaultFreq;								//Ĭ��Ƶ��
uint16 nCurFreq;									//��ǰƵ��
uint16 nCarryFreq;									//��ǰ�ز�Ƶ��
uint16 nCarrySendCycles;							//�ز���������

//������
uint8 nCltFunc;										//�ɼ���������
uint8 nCltCfg;										//�ɼ���������
float fBrgSens[SENSOR_MAX_COUNT];					//������

//AD�洢ϵ��
float g_fADStoreParam[SENSOR_MAX_COUNT];			//����ת��ϵ��

//ϵ��
float fCltK[4];

//������
#define DEFAULT_SAMPLE_RATE		40					//Ĭ�ϲ�����
uint16 nBrgSetSampleRate;							//Ҫ���õĲ�����
uint16 nBrgCurSampleRate;							//��ǰ������
uint16 nBrgRefSampleSplt = SAMPLE_SLIP_SYNC_RT;		//��̬�ɼ��ο�������(ͬ���ź�Ƶ��)
float SYS_OPT_SEG fBrgSampleDelt;					//�����ο�����������������͵��ڲɼ�����

//��ǰ״̬
BOOL bArrTaskBusy[SENSOR_MAX_COUNT];				//����æ��־
//uint8 nArrSensorTask[SENSOR_MAX_COUNT];			//����������
uint8 nMemTask;										//�洢������

BOOL g_bPrepareEnable;								//��¼�Ƿ�ʹ��Ԥ�ɼ�
BOOL g_bLocalSetZero;								//��¼�Ƿ�ʹ�ñ��ص���
BOOL bDynEnd;										//����AD������̬�ɼ�ʱ����һ�����ݴ洢

//���в���
STORE_DATA_TYPE_S g_nCurDataType[SENSOR_MAX_COUNT];	//��¼��ǰҪ�������������

//����ֵ
uint32 SYS_OPT_SEG nLocalCount;						//���ش洢����ֵ
uint32 nDynLocalAddr[SENSOR_MAX_COUNT];				//���ݴ洢����ַ����
uint32 nStoreAddr[SENSOR_MAX_COUNT];				//�Ѵ洢���ݼ���
uint32 SYS_OPT_SEG nHostCount;						//Զ�̴洢����ֵ
float  SYS_OPT_SEG fHostCount;						//Զ�̴洢����ֵ������
BOOL   g_nAllowedSample = 0;						//�Ƿ��������

//��ʱ����
static float TmpFloat;								//��ʱ����
static float TmpFloatZero;							//��ʱ��������
static float fMesureCur;							//��ǰӦ��ֵ

//�¼����ƿ��������=======================================
static OS_EVENT 	*pMemEvent = NULL;				//�¼����ƿ�
static uint8 		nMemErr;						//�����־

uint32 nPushDynNum = 0;								//���붯̬�������ĸ���
BOOL bADPushDyn;									//��Ҫ���붯̬�����������ݸ���
uint32 nFlashErasedSect;							//Flash�������ĵ�ַ
uint32 nFlashToBeErase;								//��Ҫ������Flash��ַ

//�洢����
BOOL bDataNeedStore[SENSOR_MAX_COUNT];				//��Ҫ�洢���ݵ�ͨ��
BOOL bDataNeedStoreClone[SENSOR_MAX_COUNT];			//��Ҫ�洢���ݵ�ͨ��

//��¼�Ƿ���Ӵ�������־
uint8 nSensorConnected[SENSOR_MAX_COUNT];

uint8 g_nMeasureType[SENSOR_MAX_COUNT];				//�ɼ�������


//�ⲿ��������==================================
extern float	g_fBatteryHigh;						//�������ѹ����λV
extern float	g_fBatteryLow;						//��ص͵�ѹ����λV
extern uint8	g_nBatteryRate;						//��ص���
extern float	g_fBatteryVol;						//��ص�ѹ

extern uint16	g_nSleepPoint;					    //���֮������(��λΪ5S)
extern uint16	g_nSleepTicketMax;					//����ʱ��Ϊ���(��λΪ5S)

extern uint8 g_nStoreSectStart[SENSOR_MAX_COUNT];				//��ǰ����εĿ�ʼ����
extern uint8 g_nStoreSectLen[SENSOR_MAX_COUNT];					//��ǰ����εĳ���
extern int8 g_nDynBuff[SENSOR_MAX_COUNT][DYN_BUFF_LEN];			//��̬���ݻ�����

extern float idata g_fMesureZeroVol[SENSOR_MAX_COUNT];			//�洢���洢���еĵ����ѹ
extern DT_STORE idata g_sMesureZero[SENSOR_MAX_COUNT];			//�������ݣ����ڼ���Ĳ���


//���������
uint8 idata nPrepareCount;
uint8 idata nTempChannel;

//���߿���
uint16 nSleepRecvTimout = SLEEP_RECV_TIMEOUT;		//����ʱ���ճ�ʱʱ�䣬��λΪms
uint16 nNormalRecvTimeout = NORMAL_RECV_TIMEOUT;	//��̬�½��ճ�ʱʱ�䣬��λΪms

float fMesureCurTemp;
int16 nMesureCurTemp;

/****************************************************************************
* ��	�ƣ�Timer3Init()
* ��	�ܣ���ʱ��3��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void Timer3Init() reentrant
{
	TMR3RLH = TH3RL;		//��ʼ������ֵ
	TMR3RLL = TL3RL;		//��ʼ������ֵ
	TMR3CN = 0x00;			//�����־λ
	CKCON &= 0x3F;			//��ϵͳʱ�ӵ�12��Ƶ��Ϊ��ʱ��ʱ��
	EIE1 |= 0x80;			//����ʱ��3�ж�
}

/****************************************************************************
* ��	�ƣ�Timer3ResetRun()
* ��	�ܣ���ʱ��3��λ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void Timer3ResetRun() reentrant
{
	TMR3CN = 0x00;

 	Timer3Counter = 0;
	TMR3H = TMR3RLH;
	TMR3L = TMR3RLL;	
	
	Timer3Start();			//��ʼ����
}

/****************************************************************************
* ��	�ƣ�SetTimer3Rate()
* ��	�ܣ����ö�ʱ��3���Ƶ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void SetTimer3Rate(int nRate) reentrant
{
	unsigned int nCount;

	//�淶��������
	nRate = (nRate < AD_RATE_MIN) ? AD_RATE_MIN : nRate;
	nRate = (nRate > AD_RATE_MAX) ? AD_RATE_MAX : nRate;

	//ȷ���ֶ���Ŀ
	if(nRate >= 32)	Timer3CounterMax = 1;
	else if(nRate >= 16) Timer3CounterMax = 2;
	else if(nRate >= 8) Timer3CounterMax = 4;
	else if(nRate >= 4) Timer3CounterMax = 8;
	else if(nRate >= 2) Timer3CounterMax = 16;
	else Timer3CounterMax = 32;

	//���㶨ʱʱ��
	nRateTime = (SYSCLK / 12 ) / nRate;
	nRateTime /= Timer3CounterMax;
	nCount = -nRateTime;

	//���ض�ʱ��
	TMR3RLH = HIBYTE(nCount);
	TMR3RLL = LOBYTE(nCount);
	TMR3H = TMR3RLH;
	TMR3L = TMR3RLL;
	
	//Ԥ�ȼ���һЩ����
	fTicketPer100us	= 100.0f / (12000000.0f / (float)SYSCLK);
	nTicketPer100us = fTicketPer100us;
}

/****************************************************************************
* ��	�ƣ�Timer3SetAndRun()
* ��	�ܣ���ʱ��3���ò�����
* ��ڲ�������
* ���ڲ�������
* ˵	����ͬ�������ؼ�����֮ǰ�ȸ��±��ؼ���
****************************************************************************/
uint16 nTimeSetData = 0;
void Timer3CaliAndRun() reentrant
{
	TMR3CN = 0x00;

	//��ҪԤ���ʱ�����
	nCycleTicketLeft = nCycleTimeLeft * nTicketPer100us;  	

	//����ֶ���Ŀ
	Timer3Counter = nCycleTicketLeft / nRateTime;
	
	//����Ҫ���еķֶ���Ŀ
	//if(Timer3Counter >= Timer3CounterMax)
	//{
	//	Timer3Counter = Timer3CounterMax - 1;		//�����ϲ�Ӧ�����е�����
	//}
	Timer3Counter = Timer3CounterMax - Timer3Counter - 1;

	//����ֶκ�ʣ���ʱ��
	nCycleTicketLeft = nCycleTicketLeft % nRateTime;


	//����ʣ��ʱ��
	nTimeSetData = -nCycleTicketLeft;
	TMR3H = HIBYTE(nTimeSetData);
	TMR3L = LOBYTE(nTimeSetData);	
	
	//��ʼ����
	Timer3Start();
}

/****************************************************************************
* ��	�ƣ�SetNormalRecvTimeOut()
* ��	�ܣ����������Ľ��ճ�ʱʱ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SetNormalRecvTimeOut() reentrant
{
	//����Nrf905���ճ�ʱʱ��
	//ע���������ʱ����ɼ�������ʱ���й�
	WirelessSetRecvTimeOut(nNormalRecvTimeout);
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
	//����Nrf905���ճ�ʱʱ��
	//ע���������ʱ����ɼ�������ʱ���й�
	WirelessSetRecvTimeOut(nSleepRecvTimout);
}

/****************************************************************************
* ��	�ƣ�SetFreqToDefault()
* ��	�ܣ�Ƶ������ΪĬ��Ƶ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SetFreqToDefault() reentrant
{
	//����ΪĬ��Ƶ��
	WirelessSetNeedFreq(nDefaultFreq);
}

/****************************************************************************
* ��	�ƣ�SetFreqToCur()
* ��	�ܣ�Ƶ������Ϊ��ǰƵ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SetFreqToCur() reentrant
{
	//Ƶ������
	WirelessSetNeedFreq(nCurFreq);
}

/*****************************************************************************************************************
* ��	�ƣ�UserCommInit()
* ��	�ܣ�ͨ��Ӧ�ò��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void UserCommInit() reentrant
{
	uint8 i;

	//�����洢�������¼�
	if(pMemEvent == NULL)
	{
		pMemEvent = OSSemCreate(0);	
	}	

	//��ʱ��3��ʼ��(ͬ��ʱ�Ӷ�ʱ��)
	Timer3Init();

	//����������
	nBrgCurSampleRate = 32;
	nBrgSetSampleRate = 32;

	//��ֹ��̬�ɼ�������Flashʱ�Զ�ʹ�ܣ�ֹͣ��̬�ɼ�ʱ�Զ���ֹ
	bDisableDynamic = TRUE;
	nFlashErasedSect = 0;
	nFlashToBeErase = 0;
	g_nAllowedSample = 0;

	//����ͨ�Ų���
	pWirelessDCB = Si4432GetDCB();
	nDefaultFreq = FREQ_DEVICE_DEFAULT;		//����Ĭ��Ƶ��
	nCurFreq = nDefaultFreq;				//��ǰΪĬ��Ƶ��
	pWirelessDCB->nFreq = nDefaultFreq;		//ͨ��Ƶ������

	//���ó��泬ʱʱ��
	SetNormalRecvTimeOut();

	//���ݳ�ʼ��
	DataInitZero(bArrTaskBusy);
	DataInitZero(bDataNeedStore);
	nMemTask = 0;

	g_bPrepareEnable = FALSE;		//��ʼ״̬����ʹ��Ԥ�ɼ����ڿ���ʱ�����вɼ�
	g_bLocalSetZero = FALSE;

	for(i=0;i<SENSOR_MAX_COUNT;i++)
	{
		g_nCurDataType[i].nStoreType = STORE_TYPE_UV;	//Ĭ�ϴ��ѹ
		g_nCurDataType[i].nDataType = DATA_TYPE_0F_128G;
		fBrgSens[i] = 2.0f;

		g_nMeasureType[i] = MEASURE_VOL;
	}

}

/*****************************************************************************************************************
* ��	�ƣ�SetStoreType()
* ��	�ܣ����ô洢����
* ��ڲ������洢����:�Ƿ�Ҫ�����Ӧ�䡣fSense:������ϵ����fRange:���̡�
* ���ڲ�������
* ˵	������
*====================================================================
* PGA		��ѹ(uv)		Ӧ��(ue)
* 1			2500000			1515151
* 2			1250000			757575
* 4			625000			378787
* 8			312500			189393
* 16		156250			94696
* 32		78125			47348
* 64		39062.5			23674
* 128		19531.25		11837
*====================================================================
*****************************************************************************************************************/
void SetStoreType(uint8 nType, fp32 fSense, fp32 fRange) reentrant
{
	uint8 tmp1,tmp2;
	
	//�жϵ�4λ��ȷ���洢���ݸ�ʽ
	tmp1 = nType;

	//Ҫ��洢�ϴ���ѹֵ==================
	if(5 == tmp1) 		
	{
		g_nMeasureType[g_nChannelID] = MEASURE_VOL;		//������ѹ

		g_fADStoreParam[g_nChannelID] = 1.0f;

		//���ݵ�ѹֵ���ô洢����
		if(fRange <= 301) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 100.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 601) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 50.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 1501) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 20.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 3001)
		{
			tmp2 = DATA_TYPE_1F_128G;
			g_fADStoreParam[g_nChannelID] = 10.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 19532) tmp2 = DATA_TYPE_0F_128G;
		else if(fRange <= 30001) tmp2 = DATA_TYPE_0F_64G;
		else if(fRange <= 39062) 
		{
			tmp2 = DATA_TYPE_0F_64G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 78126) 
		{
			tmp2 = DATA_TYPE_0F_32G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 156251) 
		{
			tmp2 = DATA_TYPE_0F_16G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 312501) 
		{
			tmp2 = DATA_TYPE_0F_8G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}			
		else if(fRange <= 625001) 
		{
			tmp2 = DATA_TYPE_0F_4G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}			
		else if(fRange <= 1250001) 
		{
			tmp2 = DATA_TYPE_0F_2G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}			
		else 
		{
			tmp2 = DATA_TYPE_0F_1G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}			
	}

	//�洢�ϴ�Ӧ��ֵ
	else		
	{
		g_nMeasureType[g_nChannelID] = MEASURE_STRAIN;		//����Ӧ��

		//ȫ��
		if(1 == tmp1) g_fADStoreParam[g_nChannelID] = 1.0f / (3.3f * fSense);

		//����
		else if(2 == tmp1) g_fADStoreParam[g_nChannelID] = 1.0f / (1.65f * fSense);

		// 1/4��
		else if(4 == tmp1) g_fADStoreParam[g_nChannelID] = 1.0f / (0.825f * fSense);

		//����Ӧ��ֵ���ô洢����
		if(fRange <= 101) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 100.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 201) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 50.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 501) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 20.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 2001)
		{
			tmp2 = DATA_TYPE_1F_128G;
			g_fADStoreParam[g_nChannelID] = 10.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 10001) tmp2 = DATA_TYPE_0F_128G;
		else if(fRange <= 20001) tmp2 = DATA_TYPE_0F_64G;
		else if(fRange <= 40001) 
		{
			tmp2 = DATA_TYPE_0F_32G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 80001) 
		{
			tmp2 = DATA_TYPE_0F_16G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 160001) 
		{
			tmp2 = DATA_TYPE_0F_8G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 320001) 
		{
			tmp2 = DATA_TYPE_0F_4G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 640001) 
		{
			tmp2 = DATA_TYPE_0F_2G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}
		else 
		{
			tmp2 = DATA_TYPE_0F_1G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}
	}

	//��������
	g_nCurDataType[g_nChannelID].nDataType = tmp2;
}

/*****************************************************************************************************************
* ��	�ƣ�HostCommProcess()
* ��	�ܣ��ϲ�ͨ�Ŵ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
BOOL HostCommProcess(uint8 nLen) reentrant
{
	uint8 i;
	uint8 nOrder;										//������
	BOOL bNeedReply;									//��־λ��ָʾ�Ƿ���Ҫ��������
	uint8 *pBuff = NrfRxBuff;							//���ջ�����
	uint8 *pSendBuff = NrfTxBuff+BRG_ADDR_LEN;			//Ӧ�ó̷�������֡ͷ��
	uint8 nSendLen;										//�������ݳ���

	//�����²�Э������
	if(BrgNetRecv(pBuff, nLen - BRG_ADRC_LEN) != TRUE) return FALSE;
		
	//�ظ������ʼ��
	nOrder = *pBuff++;									//��ȡ������
	*pSendBuff++ = nOrder;								//����ָ��ظ�ʱ��Ҫ

	//�������ȼ���ߵ�����
	if(BRG_WAKE_UP == nOrder)
	{
		//������æ��־
		DataInitZero(bArrTaskBusy);

		//ʹ��Ԥ�ɼ�,����ʱֹͣ�ɼ�����λAD
		#ifdef USE_STATIC_PREPARE
		{
			SensorStop();
		}
		#endif
	}

	//�������æ
	if(bArrTaskBusy[g_nChannelID] == TRUE)
	{
		*pSendBuff++ = DEVICE_STATE_BUSY;
		nSendLen = 2;
		BrgNetSend(NrfTxBuff + BRG_ADDR_LEN, nSendLen);
		return TRUE;
	}	

	//�ظ�״̬��ʼ��
	*pSendBuff++ = DEVICE_DATA_VALID;					//ָʾ�ظ�������Ч���ƶ�ָ�뵽������
	bNeedReply = TRUE;									//Ĭ����Ҫ�ظ�	
	nSendLen = 2;										//Ĭ�ϻظ�����Ϊ2

	// Э�鴦��������Ҫ�ظ�ǰ������============================
	switch(nOrder)
	{
 		//�����źţ����ظ�
		case BRG_WAKE_UP:
			if(pBuff[0] == WAKEUP_RESET_FREQ)
			{
				if(*((uint16*)(pBuff+1)) == WAKEUP_RESET_KEY)
				{	
					//��λƵ��
					nCurFreq = FREQ_DEVICE_DEFAULT;
				}
			}
			bNeedReply = FALSE;
			break;

		//����Ƶ�ʣ�������ǰ���ֽڱ�ʾҪ���õ�Ƶ��ֵ
		case BRG_SET_FREQ:
			if(BRG_FREQ_KEY == *(uint32*)(pBuff))
			{
				//��ɱ���ѭ����ͨ������SetFreqToCur����
				nCurFreq = *((uint16*)(pBuff+4));
			}
			else
			{
				bNeedReply = FALSE;	 
			}		
			break;


		//�����豸
		case BRG_CONNECT_DEVICE:
			//У��ؼ���
			if(BRG_CONNECT_KEY != *(uint32*)pBuff)
			{
				bNeedReply = FALSE;		//���ظ�
				nOrder = 0;	 			//ȡ������Ĳ���
				break;
			}

			//У���豸����
			if(COLLECTOR_MODEL == *((uint16*)(pBuff+4)))
			{
				nCltFunc = *(pBuff+6);						//���ܺ�
				nCltCfg = *(pBuff+7);						//������
				
				//Ԥ�ɼ���־
				if(0 < (nCltCfg & 0x02)) g_bPrepareEnable = TRUE;
				else g_bPrepareEnable = FALSE;
				
				fCltK[0] = *((float*)(pBuff+8));		//����
				fCltK[1] = *((float*)(pBuff+12));		//������ϵ��(K)
				fCltK[2] = *((float*)(pBuff+16));		//�����͵�ѹ��

				//��ѹ������
				if((fCltK[2] < g_fBatteryVol) && (fCltK[2] > 3.5f))
				{
					//�������õ�����Χ
					g_fBatteryLow = fCltK[2];
					g_fBatteryHigh =  g_fBatteryLow + 1.0f;
				}

			}

			//�ɼ������ʹ���
			else
			{
				//ȡ������Ĳ���
				nOrder = 0;
			}

			//���ص�ص���
			*pSendBuff = g_nBatteryRate;
			
			//���ûش����ݳ���
			nSendLen += sizeof(g_nBatteryRate);

			//���ص�ǰ�豸����
			*((uint16*)(pSendBuff+1)) = (uint16)COLLECTOR_MODEL;
			nSendLen += 2;

			break;
		
		//���ɼ���״̬��������ǰһ�ֽڴ��״̬
		case BRG_READ_STATE:

			//��ѯ��ص���
			if(pBuff[0] == BRG_STATE_BATTERY)
			{
				*pSendBuff = g_nState;
				*(pSendBuff+1) = BRG_STATE_BATTERY;
				*(pSendBuff+2) = g_nBatteryRate;
				nSendLen += 3;
			}

			//Ԥ��ѯ������״̬
			else if(pBuff[0] == BRG_STATE_SENSOR_PRE)
			{
				//���ô���������״̬
				DataInit(bArrTaskBusy, TRUE);

				SetSensorTask(SENSOR_TASK_DETECTION);
				PostSensorPro();
			}

			//��ѯ������״̬
			else if(pBuff[0] == BRG_STATE_SENSOR)
			{
				*pSendBuff = g_nState;
				*(pSendBuff+1) = BRG_STATE_SENSOR;
				*(pSendBuff+2) = nSensorConnected[0];
				*(pSendBuff+3) = nSensorConnected[1];
				*(pSendBuff+4) = nSensorConnected[2];
				*(pSendBuff+5) = nSensorConnected[3];
				nSendLen += 6;
			}

			//��ѯ�汾��
			else if(pBuff[0] == BRG_STATE_VERSION)
			{
				*pSendBuff = g_nState;
				*(pSendBuff+1) = BRG_STATE_VERSION;	 
				*((unsigned long*)(pSendBuff+2)) = SYSTEM_VERSION;

				nSendLen += 6;
			}

			break;
		
		//��ȡ��ص���
		case BRG_GET_POWER:
			*pSendBuff = g_nBatteryRate;
			nSendLen += 1;
			break;

		case BRG_SLEEP_CONTROL:
			//�ؼ���У��
			if(BRG_SLEEP_KEY != *(uint32*)pBuff) 
			{
				bNeedReply = FALSE;
				break;
			}

			//�������(��λΪ5��)
			g_nSleepPoint = *((uint16*)(pBuff+4));

			//���߶��(��λΪ5��)
			g_nSleepTicketMax = *((uint16*)(pBuff+6));

			//����ʱ����ʱ��(����)
			nSleepRecvTimout = *((uint16*)(pBuff+8));
			break;

		//�ɼ���У�㣬������δ��
		case BRG_SET_ZERO:

			//У��ؼ���
			if(*((uint32*)pBuff) != BRG_ZERO_KEY)
			{
				bNeedReply = FALSE;
				break;				
			}

			//ֻ�б��ص���ʱ������һ����
			if(TRUE == g_bLocalSetZero)
			{
				//����
				if(pBuff[4] == SET_ZERO_EACH)
				{
					g_fMesureZeroVol[g_nChannelID] = SensorGetStatic(g_nChannelID);
					g_sMesureZero[g_nChannelID] = (DT_STORE)(g_fMesureZeroVol[g_nChannelID] * g_fADStoreParam[g_nChannelID]);
					*(DT_STORE*)pSendBuff = g_sMesureZero[g_nChannelID];
					g_nState |= COLLECTOR_ZERO;

					//���ûش����ݳ���
					nSendLen += sizeof(DT_STORE);
				}
				
				//ȡ������
				else if(pBuff[4] == CANCEL_ZERO_EACH)
				{
					g_fMesureZeroVol[g_nChannelID] = 0;
					g_sMesureZero[g_nChannelID] = 0;
					*(DT_STORE*)pSendBuff = g_sMesureZero[g_nChannelID];
					g_nState &= ~COLLECTOR_ZERO;

					//���ûش����ݳ���
					nSendLen += sizeof(DT_STORE);
				}

				//�����
				else if(pBuff[4] == SET_ZERO_ARR)
				{
					nPrepareCount = pBuff[5];
					nPrepareCount = (nPrepareCount > SENSOR_MAX_COUNT) ? SENSOR_MAX_COUNT : nPrepareCount;
					
					OS_ENTER_CRITICAL();
					for(i = 0; i < nPrepareCount; i++)
					{
						//��ȡͨ����
						nTempChannel = pBuff[6+i] - 1;
						nTempChannel = (nTempChannel >= SENSOR_MAX_COUNT) ? (SENSOR_MAX_COUNT-1) : nTempChannel;

						//�Ը�ͨ������
						g_fMesureZeroVol[nTempChannel] = SensorGetStatic(nTempChannel);
						g_sMesureZero[nTempChannel] = (DT_STORE)(g_fMesureZeroVol[nTempChannel] * g_fADStoreParam[nTempChannel]);
						*(DT_STORE*)pSendBuff = g_sMesureZero[nTempChannel];
						g_nState |= COLLECTOR_ZERO;

						//���ûش����ݳ���
						pSendBuff += sizeof(DT_STORE);
						nSendLen += sizeof(DT_STORE);
					}
					OS_EXIT_CRITICAL();	
				}

				//��ȡ������
				else if(pBuff[4] == CANCEL_ZERO_ARR)
				{
					nPrepareCount = pBuff[5];
					nPrepareCount = (nPrepareCount > SENSOR_MAX_COUNT) ? SENSOR_MAX_COUNT : nPrepareCount;
					
					OS_ENTER_CRITICAL();
					for(i = 0; i < nPrepareCount; i++)
					{
						//��ȡͨ����
						nTempChannel = pBuff[6+i] - 1;
						nTempChannel = (nTempChannel >= SENSOR_MAX_COUNT) ? (SENSOR_MAX_COUNT-1) : nTempChannel;

						//�Ը�ͨ��ȡ������
						g_fMesureZeroVol[nTempChannel] = 0;
						g_sMesureZero[nTempChannel] = 0;
						*(DT_STORE*)pSendBuff = g_sMesureZero[nTempChannel];
						g_nState &= ~COLLECTOR_ZERO;

						//���ûش����ݳ���
						pSendBuff += sizeof(DT_STORE);
						nSendLen += sizeof(DT_STORE);
					}
					OS_EXIT_CRITICAL();	
				}

			}
			break;
			
		//��̬�ɼ�����
		case BRG_STATIC_OPERATION:
			
			//��ʼ��̬�ɼ�
			if(pBuff[0] == BRG_START_STATIC)
			{
				g_nState |= COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
				g_nState &= ~COLLECTOR_DYNAMIC;					//ȡ����̬�ɼ���־λ

				//��ʹ��Ԥ�ɼ�ʱ����ʼ��̬�ɼ�ʱ
				#ifndef USE_STATIC_PREPARE
				{
					SensorStart();
				}
				#endif
			}

			//��ʼ��̬�ɼ�
			else if(pBuff[0] == BRG_START_STATIC_ARR)
			{
				g_nState |= COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
				g_nState &= ~COLLECTOR_DYNAMIC;					//ȡ����̬�ɼ���־λ

				//��ʹ��Ԥ�ɼ�ʱ����ʼ��̬�ɼ�ʱ
				#ifndef USE_STATIC_PREPARE
				{
					SensorStart();
				}
				#endif
			}

			//������̬�ɼ�
			else if(pBuff[0] == BRG_END_STATIC)
			{
				g_nState &= ~COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
				SensorStop();									//ֹͣ���ݲɼ�
			}

			//������̬�ɼ�
			else if(pBuff[0] == BRG_END_STATIC_ARR)
			{
				g_nState &= ~COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
				SensorStop();									//ֹͣ���ݲɼ�
			}

			//Ԥ�ɼ�  			
			else if(pBuff[0] == BRG_PREPARE_STATIC)
			{
				g_nState |= COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
				g_nState &= ~COLLECTOR_DYNAMIC;					//ȡ����̬�ɼ���־λ

				OS_ENTER_CRITICAL();
				bArrTaskBusy[g_nChannelID] = TRUE;
				OS_EXIT_CRITICAL();

				SensorStart();
			}

			//Ԥ�ɼ���  			
			else if(pBuff[0] == BRG_PREPARE_STATIC_ARR)
			{
				g_nState |= COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
				g_nState &= ~COLLECTOR_DYNAMIC;					//ȡ����̬�ɼ���־λ

				OS_ENTER_CRITICAL();
				DataInit(bArrTaskBusy, TRUE);
				OS_EXIT_CRITICAL();

				SensorStart();
			}
			
			//��ȡ��̬����
			else if(pBuff[0] == BRG_READ_STATIC)
			{
				//��ȡ����
				OS_ENTER_CRITICAL();

				*(DT_STORE*)pSendBuff = (DT_STORE)(SensorGetStatic(g_nChannelID) * g_fADStoreParam[g_nChannelID]) 
										- g_sMesureZero[g_nChannelID];

				//���ûش����ݳ���
				nSendLen += sizeof(DT_STORE);

				//���ص�ص���
				*pSendBuff++ = g_nBatteryRate;
				nSendLen += 1;
				OS_EXIT_CRITICAL();

			}

			//��ȡ��̬������
			else if(pBuff[0] == BRG_READ_STATIC_ARR)
			{
				nPrepareCount = pBuff[1];	

				//��ȡ����
				OS_ENTER_CRITICAL();
				for(i = 0; i < nPrepareCount; i++)
				{
					//��ȡָ����ͨ����
					nTempChannel = pBuff[2+i] - 1;
					nTempChannel = (nTempChannel >= SENSOR_MAX_COUNT) ? (SENSOR_MAX_COUNT-1) : nTempChannel;

					//����������Ч��־
					*pSendBuff++ = DEVICE_DATA_VALID;
					nSendLen++;

					//��ȡ����
					*(DT_STORE*)pSendBuff = (DT_STORE)(SensorGetStatic(nTempChannel) * g_fADStoreParam[nTempChannel]) - g_sMesureZero[nTempChannel];
					pSendBuff += sizeof(DT_STORE);
					nSendLen += sizeof(DT_STORE);
	
				}

				//���ص�ص���
				*pSendBuff++ = g_nBatteryRate;
				nSendLen += 1;
				
				OS_EXIT_CRITICAL();
			}
			
			//����
			break;
		
		//��̬���ݲ���
		case BRG_DYNC_OPERATION:
			if(pBuff[0] == BRG_ENABLE_DYNC)
			{
				//ʹ�ܶ�̬�ɼ�
				bDisableDynamic = FALSE;
			}
			else if(pBuff[0] == BRG_DISABLE_DYNC)
			{
				//��ֹ��̬�ɼ�
				bDisableDynamic = TRUE;
			}

			//��ʼ��̬�ɼ�
			else if(pBuff[0] == BRG_START_DYNC)
			{
				//��ʼ��̬�ɼ�
				if(bDisableDynamic == TRUE)
				{
					pSendBuff[0] = 0x00;					//�ظ���Ч�ź�
				}
				else
				{
					//��ȡ����
					nBrgSetSampleRate = *((uint16*)(pBuff + 1));		//��¼Ҫ���õĲ����ʲ���
					nBrgRefSampleSplt = *((uint16*)(pBuff + 3));		//��¼�ο�������
		
					//�ظ���Ч�ź�
					pSendBuff[0] = 0x01;
				}
				
				//���ûش����ݳ���
				nSendLen += 1; 
			}
			
			//ֹͣ��̬�ɼ�			
			else if(pBuff[0] == BRG_END_DYNC)
			{
				//������̬�ɼ�
				g_nAllowedSample = 0;						//���������
				Timer3Stop();								//�ض�ʱ��3
				CloseTimer3ISR();							//�ض�ʱ��3�ж�
				g_nState &= ~COLLECTOR_DYNAMIC;				//ȡ����̬�ɼ���־λ
				SensorStop();								//�ش�����
				nFlashErasedSect = 0;						//��Flash����������
				if(FALSE == bDynEnd)
				{
					bDynEnd = TRUE;
					bDisableDynamic = TRUE;					//��ֹ��̬�ɼ�(ֻ�в����ɼ�����֮��Ż���Ӧ)
					
					//�洢���һ������
					SensorDataStore();
				}
			}

			//��ȡ��̬����
			else if(pBuff[0] == BRG_READ_DYNC)
			{
				//�����첽��ȡ��̬���ݣ���������Ŵ�ָ���ɼ�������ʼ������ָ�����ֽ���
				nRequireAddr = *((uint32*)(pBuff+1));		//��ȡ������������ 
				nHostReadBytes = pBuff[5];
				DataLoad(pSendBuff, nRequireAddr, nHostReadBytes, g_nChannelID);
				nSendLen += nHostReadBytes;
			}


			//��ͣ��̬�ɼ�
			else if(pBuff[0] == BRG_PAUSE_DYNC)
			{
				g_nAllowedSample = 0;						//���������
				Timer3Stop();								//�ض�ʱ��3
				CloseTimer3ISR();							//�ض�ʱ��3�ж�
				SensorStop();								//�ش�����
			
				//�ظ���ǰ����
				*((uint32*)pSendBuff) = nLocalCount;
				nSendLen += 4; 
			}

			//��ͣͬ��
			else if(pBuff[0] == BRG_PAUSE_SYNC)
			{
				nHostCount = *((uint32*)(pBuff+1));				//�������ݸ���
	
				//�洢����==========================
				if(nLocalCount < nHostCount)
				{
					//�洢����
					PostSampleSem();			
				}
			}

			//��ͣ�ָ���̬
			else if(pBuff[0] == BRG_PAUSE_RESUME)
			{
				SensorPauseResume();					//��������
	
				//������ʱ��
				bDynEnd = FALSE;						//ȡ��������־
				g_nAllowedSample = 0;					//���������
				Timer3ResetRun();						//������ʱ��3
				OpenTimer3ISR();						//������ʱ��3�ж�
	
				//�ظ���Ч�ź�
				pSendBuff[0] = 0x01;
			
				//���ûش����ݳ���
				nSendLen += 1; 
			}

			break;
			
		//����ͬ����ȡ��̬���ݣ���������Ŵ�ָ���ɼ�������ʼ������ָ�����ֽ���
		case BRG_READ_DYNAMIC:
			nRequireAddr = *((uint32*)pBuff);				//��ȡ������������ 
			nHostReadBytes = pBuff[4];
			DataLoad(pSendBuff, nRequireAddr, nHostReadBytes, g_nChannelID);
			nSendLen += nHostReadBytes;
			break;
			
		//��̬ͬ���ź�
		case BRG_SYNC_DYNAMIC:
			//�ж��Ƿ񱻽�ֹ=======================================================
			if(bDisableDynamic == TRUE) return FALSE;

			//�Ƿ��Ѿ���ʼ�ɼ������û�����˳�=====================================			
			if((g_nState & COLLECTOR_DYNAMIC) != COLLECTOR_DYNAMIC) return FALSE;

			//�������
			g_nAllowedSample = TRUE;

			//��ȡ������===========================================================
			nRequireCollector = *(uint16*)(pBuff);
			nSyncCode = nRequireCollector & BRG_SYNC_MASK;
			
			//ͬ���Ͳ�������=======================================================
			if((nSyncCode & SYNC_CALI_TIME) != 0x0000 )
			{
				//����������
				nHostTime = *((uint32*)(pBuff+2));						//��ȡ������������

				//������������
				fHostCount = (float)nHostTime;						  	//��������(����������)
				fHostCount *= fBrgSampleDelt;						   	//�����������
				nHostCount = (uint32)fHostCount;						//��������(����)

				//��������ʱ��
				fHostCount = (float)nHostCount;							//��ǰ�ɼ�����
				fHostCount /= fBrgSampleDelt;							//��ǰ�ɼ�������Ӧ��ʱ��(����������)
				nCycleTimeLeft = (uint32)fHostCount;					//��ǰ�ɼ�������Ӧ��ʱ��(ȡ��)
				nCycleTimeLeft = nHostTime - nCycleTimeLeft;			//�����ʱ��(100us��λ)
				
				//У׼��ʱ��========================
				//�ض�ʱ��3�ж��Է�ֹ�쳣
				CloseTimer3ISR();	
				
				//��ʱ��3ֹͣ
				Timer3Stop();

				//У׼��ʱ��3
				//����ɼ�����ܶ�
				if(nLocalCount > nHostCount + 1)
				{
					Timer3ResetRun();
				}
				else
				{
					//��һ�����һЩ
					Timer3CaliAndRun();
				}

				//�洢����==========================
				if(nLocalCount < nHostCount)
				{
					//�洢����
					PostSampleSem();			
				}

				//����ʱ��3�ж�
				OpenTimer3ISR();					
			}

			//�Ƿ�Ҫ�ش���̬ʵʱ����=====================================================
			nRequireCollector &= COLLECTOR_NET_ID;
			if(GetCollectorLocalID() == nRequireCollector)
			{
				//�ش�������
				if((nSyncCode & SYNC_RETURN_EMPTY) != 0x0000 )
				{
					*(pSendBuff - 2) = BRG_REASON_REPORT;	
					pSendBuff[0] = BRG_REASON_EMPTY;
					nSendLen++;
					break;
				} 

				//����ͨ����
				g_nDynChannelID = (*(pBuff + 6) & 0xF0) >> 4;

				//��Ҫ�ش����ݵ�ַ
				*(pBuff + 6) &= 0x0F;
				nRequireAddr = *(uint32*)(pBuff+6);
				
#ifdef DEBUG_RECV_SYNC_INFOR
				//�ض�ʱ��3�ж��Է�ֹ�쳣
				CloseTimer3ISR();				
				//����Ҫ�ش������ݳ���
				OS_ENTER_CRITICAL();
				if(nDynLocalAddr[g_nDynChannelID] > nRequireAddr)
				{
					nHostDeviceByteDiff = nDynLocalAddr[g_nDynChannelID] - nRequireAddr;
					if(nHostDeviceByteDiff > UPDATA_AREA_BYTES-8)
					{
						nHostDeviceByteDiff = UPDATA_AREA_BYTES-8;
					}
				}
				else nHostDeviceByteDiff = 0;
				OS_EXIT_CRITICAL();

				//����У���ֶ�
				*(pSendBuff - 1) = nRequireAddr;
				
				//װ���շ���Ϣ
				*((uint32*)(pSendBuff+0)) = nRequireAddr;
				*((uint32*)(pSendBuff+4)) = nDynLocalAddr[g_nDynChannelID];

				//װ������
				nHostReadBytes = nHostDeviceByteDiff;
				pSendBuff[8] = nHostReadBytes;
				DataLoad(pSendBuff+9, nRequireAddr, nHostReadBytes, g_nDynChannelID);	//������
				SetSendChannelID(g_nDynChannelID + 1);
				
				//���ûش����ݳ���
				nSendLen++;					   		//������
				nSendLen += nHostReadBytes+8;		//������	

				//����ʱ��3�ж�
				OpenTimer3ISR();
#else
				//�ض�ʱ��3�ж��Է�ֹ�쳣
				CloseTimer3ISR();				
				//����Ҫ�ش������ݳ���
				OS_ENTER_CRITICAL();
				if(nDynLocalAddr[g_nDynChannelID] > nRequireAddr)
				{
					nHostDeviceByteDiff = nDynLocalAddr[g_nDynChannelID] - nRequireAddr;
					if(nHostDeviceByteDiff > UPDATA_AREA_BYTES)
					{
						nHostDeviceByteDiff = UPDATA_AREA_BYTES;
					}
				}
				else nHostDeviceByteDiff = 0;
				OS_EXIT_CRITICAL();

				//����У���ֶ�
				*(pSendBuff - 1) = nRequireAddr;
				
				//װ������
				nHostReadBytes = nHostDeviceByteDiff;
				pSendBuff[0] = nHostReadBytes;
				DataLoad(pSendBuff+1, nRequireAddr, nHostReadBytes, g_nDynChannelID);	//������
				SetSendChannelID(g_nDynChannelID + 1);
				
				//���ûش����ݳ���
				nSendLen++;					   	//������
				nSendLen += nHostReadBytes;		//������	

				//����ʱ��3�ж�
				OpenTimer3ISR();
#endif
			}
			else
			{
				bNeedReply = FALSE;
			}
			
			//�˳�
			break;

		//����Flash����������һ�ֽڴ�Ű�ȫ�룬Ϊ0x35
		case BRG_ERASE_FLASH:
			//�жϰ�ȫ��
			if(*((uint16*)pBuff) != BRG_ERASE_FLASH_KEY)
			{
				bNeedReply = FALSE;
				break;
			}

			//��ȡ��������
			nFlashToBeErase = *((uint32*)(pBuff + 2));

			//����״̬
			Timer3Stop();						//�ض�ʱ��3
			break;

		//У׼�ɼ���
		case BRG_CALI_VALUE:
			//У��ؼ���
			if(*((uint32*)pBuff) != BRG_CALI_KEY)
			{
				bNeedReply = FALSE;
				break;				
			}

			//����
			if(pBuff[4] == CALI_MODE_SELF)
			{
				//У����
				TmpFloat = *((fp32*)(pBuff+5));
				TmpFloat = TmpFloat * 1.65f ;
				
				//���
				TmpFloatZero= *((fp32*)(pBuff+9));
				if(TRUE == g_bLocalSetZero)
				{
					//��ȡ���ص���ֵ
					TmpFloatZero = g_fMesureZeroVol[g_nChannelID];	
				}
				
				//��ȡ��ǰ������ѹֵ
				fMesureCur = SensorGetStatic(g_nChannelID);
				
				OS_ENTER_CRITICAL();

				//����ǰ��ѹֵ�����Ĳ�ֵ
				if(0 ==(fMesureCur - TmpFloatZero)) fMesureCur = 0.0001f;
				else fMesureCur = (float)(fMesureCur -TmpFloatZero);
				
				//(����*10000)/0��10000�ĵ�ѹ��ֵ
				g_fADStep[g_nChannelID] = (g_fADStep[g_nChannelID] * TmpFloat) / fMesureCur;

				OS_EXIT_CRITICAL();
			}
			else if(pBuff[4] == CALI_MODE_SET)
			{
				//ֱ������
				g_fADStep[g_nChannelID] = *((float*)(pBuff+5));
			}
			else if(pBuff[4] == CALI_MODE_FACTORY)
			{
				//�ָ���������
				g_fADStep[g_nChannelID] = F_AD_STEP;
			}
			
			//�ظ��²���
			*((float*)pSendBuff) = g_fADStep[g_nChannelID];
			nSendLen += sizeof(float);
			
			break;

		//�����ز�
		case BRG_SEND_CARRY:
			nCarryFreq = *((uint16*)pBuff);
			nCarrySendCycles = *((uint16*)(pBuff+2));
			break;
			
		//Ĭ�ϴ���
		default:
			//�ظ�û�ж�Ӧָ��
			*(pSendBuff - 1) = DEVICE_WRONG_ORDER;
			nSendLen += 2;
			return FALSE;
	}
	//-----------------------------------------------------------------------

	//�ظ���������===========================================================
	if(bNeedReply != FALSE)
	{	
		BrgNetSend(NrfTxBuff + BRG_ADDR_LEN, nSendLen);
	}
	//-----------------------------------------------------------------------

	//���лظ���Ĳ���==================================
	switch(nOrder)
	{
		case BRG_CONNECT_DEVICE:
			
			//����洢ϵ����������ϵ���Ȳ���
			SetStoreType(nCltFunc, fCltK[1], fCltK[0]);

			//�����־������SetStoreType֮����Ҫ����
			if(nCltCfg & 0x01) 
			{
				//��ʼ�����ص���ֵ
				for(i=0;i<SENSOR_MAX_COUNT;i++)
				{
					g_sMesureZero[i] = (DT_STORE)(g_fMesureZeroVol[i] * g_fADStoreParam[i]);
				}
				
				//�����־��λ
				g_bLocalSetZero = TRUE;
			}
			else 
			{
				//���������ֵ
				DataInitZero(g_sMesureZero);
				
				//�����־��λ
				g_bLocalSetZero = FALSE;
			}
			break;

		//�ɼ���У�㣬������δ��
		case BRG_SET_ZERO:
			
			//У��ؼ���
			if(*((uint32*)pBuff) != BRG_ZERO_KEY) break;
			
			if(pBuff[4] == SET_ZERO_EACH)
			{
				OS_ENTER_CRITICAL();
				bArrTaskBusy[g_nChannelID] = TRUE;
				OS_EXIT_CRITICAL();

				nMemTask = COLLECTOR_ZERO;	
				OSSemPost(pMemEvent);
			}
			else if(pBuff[4] == CANCEL_ZERO_EACH)
			{
				OS_ENTER_CRITICAL();
				ZeroParamRemove(g_nChannelID);
				OS_EXIT_CRITICAL();
			}

			else if(pBuff[4] == SET_ZERO_ARR)
			{
				nMemTask = COLLECTOR_ZERO;	
				OSSemPost(pMemEvent);
			}

			else if(pBuff[4] == CANCEL_ZERO_ARR)
			{
				nPrepareCount = pBuff[5];
				nPrepareCount = (nPrepareCount > SENSOR_MAX_COUNT) ? SENSOR_MAX_COUNT : nPrepareCount;

				for(i = 0; i < nPrepareCount; i++)
				{
					//��ȡͨ����
					nTempChannel = pBuff[6+i] - 1;
					nTempChannel = (nTempChannel >= SENSOR_MAX_COUNT) ? (SENSOR_MAX_COUNT-1) : nTempChannel;

					//ȡ������
					OS_ENTER_CRITICAL();
					ZeroParamRemove(nTempChannel);
					OS_EXIT_CRITICAL();	
				}
			}
			break;

		//��̬���ݲ���
		case BRG_DYNC_OPERATION:
			//��ʼ��̬�ɼ�
			if(pBuff[0] == BRG_START_DYNC)
			{
				if(bDisableDynamic == FALSE)
				{
					//�����������
					fBrgSampleDelt = (float)nBrgSetSampleRate;
					fBrgSampleDelt *= (float)nBrgRefSampleSplt;
					fBrgSampleDelt /= 1000000.0f;
		
					//���ò�����
					SetTimer3Rate(nBrgSetSampleRate);
					
					//����״̬
					nFlashErasedSect = 0;					//Flash�������ĳ���
 					nLocalCount = 0;						//����������
					nHostCount = 0;							//������������
					DataInitZero(nStoreAddr);				//������ش洢��ַ
					DataInitZero(nDynLocalAddr);			//��������ܵ�ַ
					g_nState |= COLLECTOR_DYNAMIC;			//�ö�̬�ɼ���־λ
					
					//��������
					SensorStart();							//��������

					//������ʱ��
					bDynEnd = FALSE;						//ȡ��������־
					g_nAllowedSample = 0;					//���������
					Timer3ResetRun();						//������ʱ��3
					OpenTimer3ISR();						//������ʱ��3�ж�
				}
			}
			break;
			
		//����Flash
		case BRG_ERASE_FLASH:
			//�жϰ�ȫ��
			if(*((uint16*)pBuff) != BRG_ERASE_FLASH_KEY) break;

			//ֹͣ�������Է�ֹ�쳣
			SensorStop();

			//��ӦSensorStop()����
			OSTimeDly(3);
		 
			//���ô洢����
			nMemTask = BRG_ERASE_FLASH;

			//���������ź�
			OSSemPost(pMemEvent);

			break;

		//У��
		case BRG_CALI_VALUE :
			//У��ؼ���
			if(*((uint32*)pBuff) != BRG_CALI_KEY) break;
			
			OS_ENTER_CRITICAL();
			ParamErase();									//����������
			MainParamStore();								//�洢����
			OS_EXIT_CRITICAL();
			SensorParamCacul();								//���¼���ADϵ��
			break;
			
		//�����ز�
		case BRG_SEND_CARRY : 
			pWirelessDCB->nFreq = nCarryFreq;
			WirelessSetNeedFreq(nCarryFreq);				//���ò���
			WirelessSendCarry(1);
			pWirelessDCB->nFreq = nCurFreq;
			WirelessSetNeedFreq(nCurFreq);					//���ò���
			break;
			
		default : break;
	}
	//---------------------------------------------------------------------
	
	return TRUE;	
}

/****************************************************************************
* ��	�ƣ�SensorProcess()
* ��	�ܣ��������������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
BOOL SensorProcess() reentrant
{
	uint8 i = 0;
	uint8 nTask;

	//�ȴ�����������
	WaitSensorEvent();

	//��ȡ����(��ͨ��ͬ���ɼ����������ۺ�)
	OS_ENTER_CRITICAL();
	nTask = GetSensorTask();
	SetSensorTask(SENSOR_TASK_NONE);
	OS_EXIT_CRITICAL();

	//����������====================================================================================
	if(nTask == SENSOR_TASK_START)
	{
		//����AD
		ADOpen();
	
		//��������
		for(i=0; i<SENSOR_MAX_COUNT; i++)
		{
			//���ò�����===============================
			if((COLLECTOR_STATIC & g_nState) == COLLECTOR_STATIC)		//��̬ģʽ��
			{
				ADSetSampleRate(20, i);
			}
			else									//��̬�ɼ�ģʽ
			{
				ADSetSampleRate(nBrgSetSampleRate, i);
			}
			
			//��������================================
			switch(g_nCurDataType[i].nDataType)
			{
				case DATA_TYPE_2F_128G : 
				case DATA_TYPE_1F_128G :
				case DATA_TYPE_0F_128G :
					ADSetGain(AD_GAIN_SELECT_128, i);		//����128��
					break;
				case DATA_TYPE_0F_64G :
					ADSetGain(AD_GAIN_SELECT_64, i);		//����64��
					break;
				case DATA_TYPE_0F_32G :
					ADSetGain(AD_GAIN_SELECT_32, i);		//����32��
					break;
				case DATA_TYPE_0F_16G :
					ADSetGain(AD_GAIN_SELECT_16, i);		//����16��
					break;
				case DATA_TYPE_0F_8G :
					ADSetGain(AD_GAIN_SELECT_8, i);			//����8��
					break;
				case DATA_TYPE_0F_4G :
					ADSetGain(AD_GAIN_SELECT_4, i);			//����4��
					break;
				case DATA_TYPE_0F_2G :
					ADSetGain(AD_GAIN_SELECT_2, i);			//����2��
					break;
				case DATA_TYPE_0F_1G :
					ADSetGain(AD_GAIN_SELECT_1, i);			//����1��
					break;
				default : 
					ADSetGain(AD_GAIN_SELECT_1, i);			//����1��
					break;
			}
		}
	
		//�������
		SensorParamCacul();			
	
		//��ʼ�ɼ�======================
		ADStartSample();

		//����
		return TRUE;
	}
	
	//�رմ�����====================================================================================
	else if(nTask == SENSOR_TASK_STOP)
	{
		//�ر�AD
		ADClose();
		
		//����
		return TRUE;
	}

	//��⴫����====================================================================================
	else if(nTask == SENSOR_TASK_DETECTION)
	{
		//���ɼ�����Ӵ�����
		ADSensorDetection(nSensorConnected);

		//���ô���������״̬
		DataInitZero(bArrTaskBusy);

		//����
		return TRUE;
	}

	return TRUE;
}

/****************************************************************************
* ��	�ƣ�PostDataStore()
* ��	�ܣ����ʹ������ź���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
uint8 nDynStoreChannel;
void PostDataStore(uint8 nChannelID) reentrant
{
	OS_ENTER_CRITICAL();
	bDataNeedStore[nChannelID] = TRUE;
	OS_EXIT_CRITICAL();

	OSSemPost(pMemEvent);
}

/****************************************************************************
* ��	�ƣ�MemEraseProcess()
* ��	�ܣ������洢������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
uint8 *pStoreDynBuffTmp;
uint8 nMemTaskCache;				//�洢���񻺳�
BOOL MemProcess() reentrant
{
	uint8 nTask = 0;
	uint8 i;

	//�޳�ʱ�ȴ�ADת����ɣ���洢�ź�
	OSSemPend(pMemEvent, 0, &nMemErr);

	//��̬���ݴ洢����================================================================================================
	//��ȡҪ�洢���ݵ�ͨ��
	OS_ENTER_CRITICAL();
	memcpy(bDataNeedStoreClone, bDataNeedStore, sizeof(bDataNeedStore));
	memset(bDataNeedStore, 0, sizeof(bDataNeedStore)); 
	OS_EXIT_CRITICAL();

	//ͳ���Ƿ���ͨ����Ҫ��������
	for(nTask = 0, i = 0; i < SENSOR_MAX_COUNT; i++)
	{
		nTask += bDataNeedStoreClone[i];
	}

	//�ж��Ƿ���Ҫ�洢
	if(nTask > 0)
	{
		for(i = 0; i < SENSOR_MAX_COUNT; i++)
		{
			if(bDataNeedStoreClone[i] == FALSE) continue;
			
			//�л���Ҫ�洢��ͨ��
			nDynStoreChannel = i;	

			//!!!!ȷ����������СΪ�洢�δ�С��������!!!!!!!!!
			pStoreDynBuffTmp = g_nDynBuff[nDynStoreChannel] + g_nStoreSectStart[nDynStoreChannel];
			MemDataStore(pStoreDynBuffTmp, nStoreAddr[nDynStoreChannel], g_nStoreSectLen[nDynStoreChannel], nDynStoreChannel);
			
			OS_ENTER_CRITICAL();
			nStoreAddr[nDynStoreChannel] += g_nStoreSectLen[nDynStoreChannel];
			//g_nStoreSectLen[nDynStoreChannel] = 0;
			OS_EXIT_CRITICAL();
		}

		return TRUE;
	}
	//------------------------------------------------------------------------------------------------------------------

	
	//��ȡ�洢����
	OS_ENTER_CRITICAL();
	nMemTaskCache = nMemTask;
	nMemTask = 0;
	OS_EXIT_CRITICAL();

	//��������=========================
	switch(nMemTaskCache)
	{
		//�����������
		case COLLECTOR_ZERO:
			ZeroParamErase();
			MainZeroParamStore();
			break;

		//����������
		case BRG_ERASE_FLASH:

			bDisableDynamic = FALSE;					//ʹ�ܶ�̬�ɼ�
			
			//�ж�Ҫ��Ĵ洢���Ƿ��Ѿ�����
			if(nFlashErasedSect < nFlashToBeErase)
			{
				//����ָ������������
				MemorizerEraseToSect(nFlashErasedSect, nFlashToBeErase);
	
				//��¼��Ϣ
				nFlashErasedSect = nFlashToBeErase;		//��¼��������
			}
			
			//ָʾ����ʾ
			PostLightOn(20);
			break;
		default : break;
	}

	OS_ENTER_CRITICAL();
	bArrTaskBusy[g_nChannelID] = FALSE;
	OS_EXIT_CRITICAL();

	
	//����
	return TRUE;
}

/****************************************************************************
* ��	�ƣ�SampleProcess()
* ��	�ܣ�AD����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
BOOL SampleProcess() reentrant
{
	//�޳�ʱ�ȴ��ź���=================================
	WaitSampleEvent();
	
	//������Ҫ�洢�����ݸ���
	nPushDynNum = 1;
	if(nHostCount > nLocalCount) 
	{
		nPushDynNum = nHostCount - nLocalCount;
	}
	
	//�洢��̬����
	SensorPushDynamic(nPushDynNum);	
	
	//����
	return TRUE;  
}

//��ʱ��3�ж���Ӧ����(���ݲ�����ʱ��)
void Timer3ISR() interrupt 14
{
	//���ж�
	OSIntEnter();

	//����жϱ�־
	ClearTimer3ISR();
	
	//��ʱ���ֶ�����һ
	Timer3Counter++;

	//�Ƿ񵽴����ʱ��
	if(Timer3Counter >= Timer3CounterMax)
	{
		//��ʱ���ֶ�����
		Timer3Counter = 0;

		//��������
		if(g_nAllowedSample == TRUE)
		{
			PostSampleSem();
		}
	}

	//���ж�
	OSIntExit();
}





/****************************************************************************
* ��	�ƣ�SensorDetectionAndShow()
* ��	�ܣ���鴫��������LED��ʾ��鴫�����Ľ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
#ifdef	POWER_ON_SENSOR_DETECTION 
void SensorDetectionAndShow() reentrant
{
	unsigned char i,j;

	//���ɼ�����Ӵ�����
	ADSensorDetection(nSensorConnected);

	for(i=0;i<SENSOR_MAX_COUNT;i++)
	{
		for(j = 0; j < (i+1); j++)
		{
			LightOn();
			OSTimeDly(2);

			LightOff();
			OSTimeDly(20);
		}  

		//ͣʱ��
		OSTimeDly(50);

		//��⵽������
		if(TRUE == nSensorConnected[i])
		{
			LightOn();
			OSTimeDly(50);
			LightOff();
			OSTimeDly(50);
		}

		//û��⵽������
		else
		{
			//��Ӧ��⵽��ʱ��
			LightOff();
			OSTimeDly(50);
			LightOff();
			OSTimeDly(50);
		}	
	}
}
#endif
