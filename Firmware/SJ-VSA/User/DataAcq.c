/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: DataAcq.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��09��18��
**����޸�����: 2007��10��18��
**��        ��: ���ݲɼ�Դ�ļ�
*****************************************************************************************************************/
#define 	_DATA_ACQ_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "DataManage.h"
#include "DataAcq.h"
#include "CommApp.h"

//ʹ�ú���ִ�ж�̬�ɼ�����
#define USE_MACRO_TO_SPEED


//���ݲɼ�������
#define DATA_ACQ_TASK_NONE			0		
#define DATA_ACQ_TASK_START			1
#define DATA_ACQ_TASK_STOP			2
#define DATA_ACQ_TASK_DETECTION		3
#define DATA_ACQ_TASK_PREPARE		4


//��������ʽ
#define	MEASURE_VOL			0x01		//������ѹ��ʹ��AD���۲���
#define	MEASURE_STRAIN		0x02		//����Ӧ�䣬ʹ��У������


//��̬���ݲɼ��Ļ�������С����
#define	DYN_BUFF_LEN		NAND_PAGE_SIZE*2		//��̬���ݻ�������С
#define	DYN_STORE_THRES		NAND_PAGE_SIZE			//���ݴ洢��ֵ

#define DYN_BUFF_LEN_MASK	0x00000FFF
#define DYN_SECT_LEN_MASK	0x000007FF


//���ݶ���========================================================================================================

//Ӧ���ŵ�Դ������������
//����AD���ÿ�����������΢����
//��AD����ΪN
//
//					   2.5V * 10^6 
//FKBRGVOT_DEFAUT = -------------------
//						   2^23
//����ã�
//#define FKBRGVOT_DEFAUT		0.298023f				//����΢��ʱ�����ۼ������
//#define FKBRGVOT_DEFAUT		0.2896097f			//����΢��ʱ��ʵ��������(��ΪPGA��128����������ȫ��ȷ)
//�򣺦�V = N * FKBRGVOT_DEFAUT
//
//			  S*��V
//��Ϊ��1��= -------- ��(ȫ��ʱS = 1������ʱS = 2���ķ�֮һ��ʱS = 4)
//			  K * U
//�֣�U = 3.3V
//��FKBRGVOT_DEFAUT = FKBRGVOT_DEFAUT / 3.3 = 0.0877605f
//��ʱ��
//			  N * S * FKBRGVOT_DEFAUT
//		��= --------------------------- ��(ȫ��ʱS = 1������ʱS = 2���ķ�֮һ��ʱS = 4)
//			             K
//#define FKBRGVOT_DEFAUT 		0.0877605f

//****************************************************************************************************************


//�¼�
static OS_EVENT *pDataAcqEvent;			//���ݲ����¼�
static OS_EVENT *pDataStoreEvent;		//���ݴ洢�¼�
static OS_EVENT* pDataStoreDiskSem; 	//���ݴ����ź�


//����������
static uint8 nDataAcqTaskInner;

//PGA����Ϊ1ʱAD�����������΢����
float g_fADStep[DATA_ACQ_COUNT];

//PGA����Ϊ����ֵʱAD�����������΢����(g_fADStep��������)
float g_fADVolParam[DATA_ACQ_COUNT];

//����ѹ����ΪĿ������ϵ��
float g_fADStoreParam[DATA_ACQ_COUNT];
														   
//����Ϊ��λ����Ҫ�����ݵ���ϵ��(�绻��ΪӦ������ȡ��ϵ��)
//g_fADParam = g_fADVolParam * g_fADStoreParam;
float g_fADParam[DATA_ACQ_COUNT];

//��¼��ǰҪ�������������
STORE_DATA_TYPE_S g_nCurDataType[DATA_ACQ_COUNT];	

//�˵�Ĳ�����ʽ
uint8 g_nMeasureType[DATA_ACQ_COUNT];


//������
uint16 nBrgSetSampleRate;							//Ҫ���õĲ�����
float fBrgSetSampleRate;

//��̬���ݲɼ���Ϣ
uint32 nLocalCount;									//���ش洢����ֵ
uint32 nDynLocalAddr[DATA_ACQ_COUNT];				//���ݴ洢����ַ����

int8 g_nDynBuff[DATA_ACQ_COUNT][DYN_BUFF_LEN];		//��̬���ݻ�����
uint32 nDynIndex[DATA_ACQ_COUNT];					//��������
uint32 nCurSectStart[DATA_ACQ_COUNT];				//��ǰ����εĿ�ʼ����
uint32 nCurSectLen[DATA_ACQ_COUNT];					//��ǰ����εĳ���
uint32 g_nStoreSectStart[DATA_ACQ_COUNT];			//��ǰ����εĿ�ʼ����
uint32 g_nStoreSectLen[DATA_ACQ_COUNT];				//��ǰ����εĳ���


BOOL bDataNeedStore[DATA_ACQ_COUNT];				//��Ҫ�洢���ݵ�ͨ��
BOOL bDataNeedStoreClone[DATA_ACQ_COUNT];			//��Ҫ�洢���ݵ�ͨ��
uint32 nStoreAddr[DATA_ACQ_COUNT];					//�Ѵ洢���ݼ���

//������
float g_fMesureZeroVol[DATA_ACQ_COUNT];				//�洢���洢���еĵ����ѹ
DT_STORE g_sMesureZero[DATA_ACQ_COUNT];				//�������ݣ����ڼ���Ĳ���

//��¼�Ƿ���Ӵ�������־
uint8 nDataAcqConnected[DATA_ACQ_COUNT];  


//�ɼ���״̬��
uint8 g_nState = 0;	

//����æ��־
uint8 g_bTaskBusy;


//�����ⲿ����===========================================
extern uint32 nHostCount;							//Զ�̴洢����ֵ


/*****************************************************************************************************************
* ��	�ƣ�DataStoreInit()
* ��	�ܣ����ݴ洢��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_INIT_EN_
void DataStoreInit() reentrant
{
	//�������ݴ洢�ź�
	pDataStoreEvent = OSSemCreate(0);  

	//�������ݴ����ź�
	pDataStoreDiskSem = OSSemCreate(0);
}
#endif



/*****************************************************************************************************************
* ��	�ƣ�DataAcqInit()
* ��	�ܣ����ݲɼ���ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_INIT_EN_
void DataAcqInit() reentrant
{
	//�������ݲɼ��ź���
	pDataAcqEvent = OSSemCreate(0);	 

	//���ݲɼ�����
	nDataAcqTaskInner = DATA_ACQ_TASK_NONE;

	//״̬��ʼ��
	g_bTaskBusy = FALSE;	
}
#endif


/*****************************************************************************************************************
* ��	�ƣ�DataAcqParamInit()
* ��	�ܣ����ݲɼ�������ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void DataAcqParamInit(void) reentrant
{
	uint8 i;

	//Ĭ�ϲ�����
	nBrgSetSampleRate = 20;

	//����Ĭ�ϲ���
	for(i=0; i < DATA_ACQ_COUNT; i++)
	{
		g_fADStep[i] = F_AD_STEP;
		g_fMesureZeroVol[i] = 0;
		g_sMesureZero[i] = 0;


		//Ĭ�ϴ��ѹ
		g_nMeasureType[i] = MEASURE_VOL;
		g_nCurDataType[i].nStoreType = STORE_TYPE_UV;
		g_nCurDataType[i].nDataType = DATA_TYPE_0F_128G;
	}

	//����Ĳ���
	DataInitZero(bDataNeedStore);
	DataInitZero(g_fADStoreParam); 
}

/*****************************************************************************************************************
* ��	�ƣ�OnHostParamSet()
* ��	�ܣ��������ò���
* ��ڲ������洢����:�Ƿ�Ҫ�����Ӧ�䣬fSense:������ϵ����fRange:���̡�
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
void OnHostParamSet(uint8 nEp, uint8 nType, fp32 fSense, fp32 fRange) reentrant
{
	uint8 tmp1,tmp2;
	
	//�жϵ�4λ��ȷ���洢���ݸ�ʽ
	tmp1 = nType;

	//Ҫ��洢�ϴ���ѹֵ==================
	if(5 == tmp1) 		
	{
		g_nMeasureType[nEp] = MEASURE_VOL;		//������ѹ

		g_fADStoreParam[nEp] = 1.0f;

		//���ݵ�ѹֵ���ô洢����
		if(fRange <= 301) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 100.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 601) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 50.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 1501) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 20.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 3001)
		{
			tmp2 = DATA_TYPE_1F_128G;
			g_fADStoreParam[nEp] = 10.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 19532) tmp2 = DATA_TYPE_0F_128G;
		else if(fRange <= 30001) tmp2 = DATA_TYPE_0F_64G;
		else if(fRange <= 39062) 
		{
			tmp2 = DATA_TYPE_0F_64G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 78126) 
		{
			tmp2 = DATA_TYPE_0F_32G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 156251) 
		{
			tmp2 = DATA_TYPE_0F_16G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 312501) 
		{
			tmp2 = DATA_TYPE_0F_8G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}			
		else if(fRange <= 625001) 
		{
			tmp2 = DATA_TYPE_0F_4G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}			
		else if(fRange <= 1250001) 
		{
			tmp2 = DATA_TYPE_0F_2G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}			
		else 
		{
			tmp2 = DATA_TYPE_0F_1G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}			
	}

	//�洢�ϴ�Ӧ��ֵ
	else		
	{
		g_nMeasureType[nEp] = MEASURE_STRAIN;		//����Ӧ��

		//ȫ��
		if(1 == tmp1) g_fADStoreParam[nEp] = 1.0f / (3.3f * fSense);

		//����
		else if(2 == tmp1) g_fADStoreParam[nEp] = 1.0f / (1.65f * fSense);

		// 1/4��
		else if(4 == tmp1) g_fADStoreParam[nEp] = 1.0f / (0.825f * fSense);

		//����Ӧ��ֵ���ô洢����
		if(fRange <= 101) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 100.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 201) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 50.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 501) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 20.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 2001)
		{
			tmp2 = DATA_TYPE_1F_128G;
			g_fADStoreParam[nEp] = 10.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 10001) tmp2 = DATA_TYPE_0F_128G;
		else if(fRange <= 20001) tmp2 = DATA_TYPE_0F_64G;
		else if(fRange <= 40001) 
		{
			tmp2 = DATA_TYPE_0F_32G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 80001) 
		{
			tmp2 = DATA_TYPE_0F_16G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 160001) 
		{
			tmp2 = DATA_TYPE_0F_8G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 320001) 
		{
			tmp2 = DATA_TYPE_0F_4G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 640001) 
		{
			tmp2 = DATA_TYPE_0F_2G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}
		else 
		{
			tmp2 = DATA_TYPE_0F_1G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}
	}

	//��������
	g_nCurDataType[nEp].nDataType = tmp2;
}


/*****************************************************************************************************************
* ��	�ƣ�DataAcqParamCacul()
* ��	�ܣ�����AD����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void DataAcqParamCacul() reentrant
{
	uint8 i;

	for(i = 0; i < DATA_ACQ_COUNT; i++)
	{
		if(g_nMeasureType[i] == MEASURE_STRAIN)
		{
			//�����Ӧ�䣬����У��ϵ������AD��ѹϵ��
			g_fADVolParam[i] = g_fADStep[i] / (fp32)(0x01 << ADGetGain(i));
		}
		else 
		{
			//��������ѹ����ʹ�����۲�������AD��ѹϵ��
			g_fADVolParam[i] = F_AD_STEP / (fp32)(0x01 << ADGetGain(i));
		}

		//����AD��ϵ��
		g_fADParam[i] = g_fADStoreParam[i] * g_fADVolParam[i];

		//�������
		g_sMesureZero[i] = (DT_STORE)(g_fMesureZeroVol[i] * g_fADStoreParam[i]);
	}
}


/*****************************************************************************************************************
* ��	�ƣ�DataAcqParamStore()
* ��	�ܣ��洢����������
* ��ڲ�������ʼ�ֽڵ�ַ
* ���ڲ��������ֽڵ�ַ
* ˵	������
*****************************************************************************************************************/
#ifdef	_MAIN_PARAM_STORE_
void DataAcqParamStore() reentrant
{
	uint8 i;
	for(i = 0; i < DATA_ACQ_COUNT; i++)
	{
		g_TerminalParam.arrADStep[i] = g_fADStep[i];
		g_TerminalParam.arrZeroVol[i] = g_fMesureZeroVol[i];
	}
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�DataAcqParamLoad()
* ��	�ܣ���ȡ����������
* ��ڲ�������ʼ�ֽڵ�ַ
* ���ڲ��������ֽڵ�ַ
* ˵	������
*****************************************************************************************************************/
#ifdef	_MAIN_PARAM_STORE_
void DataAcqParamLoad() reentrant
{
	uint8 i;
	for(i = 0; i<DATA_ACQ_COUNT; i++)
	{
		g_fADStep[i] = g_TerminalParam.arrADStep[i];
		g_fMesureZeroVol[i] = g_TerminalParam.arrZeroVol[i];
	}
}
#endif


/*****************************************************************************************************************
* ��	�ƣ�PostDataAcqPro()
* ��	�ܣ��򿪴���������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_POST_DATA_ACQ_PRO_
void PostDataAcqPro() reentrant
{
	OSSemPost(pDataAcqEvent);
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�WaitDataAcqStart()
* ��	�ܣ��ȴ��򿪴�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_START_EN_
void WaitDataAcqEvent() reentrant
{
	uint8 nErr;
	OSSemPend(pDataAcqEvent, 0, &nErr);
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�SetDataAcqTask()
* ��	�ܣ����ô���������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SetDataAcqTask(uint8 nTask) reentrant
{
	nDataAcqTaskInner = nTask;
}

/*****************************************************************************************************************
* ��	�ƣ�GetDataAcqTask()
* ��	�ܣ���ȡ����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
uint8 GetDataAcqTask() reentrant
{
	return nDataAcqTaskInner;
}

/*****************************************************************************************************************
* ��	�ƣ�DataAcqStop()
* ��	�ܣ��򿪴�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_STOP_EN_
void DataAcqStop() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//���ؿ�ʼ***************************************
	//�رմ�����
	OS_ENTER_CRITICAL();
	nDataAcqTaskInner = DATA_ACQ_TASK_STOP;
	OS_EXIT_CRITICAL();
	
	//�����ź�
	pDataAcqEvent->OSEventCnt = 0;
	OSSemPost(pDataAcqEvent);

	//���ؽ���***************************************
}
#endif






//
//��̬�ɼ�������Ϣ
//
#define STATIC_NOISE_COUNT	4
#define STATIC_BUFF_LEN		8
static int32 nArrStaticData[DATA_ACQ_COUNT][STATIC_BUFF_LEN];
static int32 nArrStaticDataSum[DATA_ACQ_COUNT];
static uint8 nArrStaticIndex[DATA_ACQ_COUNT];
static uint8 nArrStaticNoiseIndex[DATA_ACQ_COUNT];
static BOOL bStaticEnd[DATA_ACQ_COUNT];


/*****************************************************************************************************************
* ��	�ƣ�DataAcqResetStaticIndex()
* ��	�ܣ���λ��̬�ɼ�����
* ��ڲ�������
* ���ڲ�������
* ˵	������ÿ�ξ�̬�ɼ��洢��0��ʼ
*****************************************************************************************************************/
void DataAcqResetStaticIndex() reentrant
{
	//��ʼ����̬�ɼ���Ϣ
	DataInitZero(nArrStaticIndex);
	DataInitZero(nArrStaticNoiseIndex);
	DataInit(bStaticEnd, FALSE);
}

/*****************************************************************************************************************
* ��	�ƣ�DataAcqSetStatic()
* ��	�ܣ���ȡ��������̬����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_STATIC_EN_
void DataAcqSetStatic(int32 nValue, uint8 nChannelID) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//������
	if(nChannelID >= DATA_ACQ_COUNT) return;

	//�ж��Ƿ�ȫ��ת����ϣ���������ش�������Դ��ֹͣ�ɼ�
	if(bStaticEnd[0] & bStaticEnd[1] & bStaticEnd[2] & bStaticEnd[3])
	{
		ADS1246StopStatic();
		g_bTaskBusy = FALSE;
		return;
	}

	//�����ʼ���ȶ�������
	if(nArrStaticNoiseIndex[nChannelID] < STATIC_NOISE_COUNT)
	{
		nArrStaticNoiseIndex[nChannelID]++;
		return;
	}

	//�����ٽ���
	OS_ENTER_CRITICAL();
	
	//��������
	if(nArrStaticIndex[nChannelID] < STATIC_BUFF_LEN) 
	{
		nArrStaticData[nChannelID][nArrStaticIndex[nChannelID]] = nValue;
		nArrStaticIndex[nChannelID]++;
	}

	//�жϻ����Ƿ����
	if(nArrStaticIndex[nChannelID] >= STATIC_BUFF_LEN) 
	{
		nArrStaticIndex[nChannelID] = 0;	
		bStaticEnd[nChannelID] = TRUE;
	}

	//�˳��ٽ���
	OS_EXIT_CRITICAL();		
}
#endif


/*****************************************************************************************************************
* ��	�ƣ�DataAcqGetStatic()
* ��	�ܣ���ȡ��������̬����
* ��ڲ�������
* ���ڲ�������ѹֵ
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_STATIC_EN_
float DataAcqGetStatic(uint8 nChannelID) reentrant
{
	uint8 i;

	//�����Ч��
	if(nChannelID >= DATA_ACQ_COUNT) return 0;

	//��AD��
	nArrStaticDataSum[nChannelID] = 0;
	for(i = 0; i < STATIC_BUFF_LEN; i++)
	{
		nArrStaticDataSum[nChannelID] += nArrStaticData[nChannelID][i];
	}

	//��AD��ֵ
	nArrStaticDataSum[nChannelID] /= STATIC_BUFF_LEN;

	//���ص�ѹֵ
	return ((float)nArrStaticDataSum[nChannelID]) * g_fADVolParam[nChannelID];	  
}
#endif








//
//��̬�ɼ�������Ϣ
//
static int32 SYS_OPT_SEG nDynSum[DATA_ACQ_COUNT];				//��̬���ݺ�
static int16 SYS_OPT_SEG nDynCurGetStore[DATA_ACQ_COUNT];

/*****************************************************************************************************************
* ��	�ƣ�DataAcqSetDynamic()
* ��	�ܣ������ջ��������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_ 
#ifndef USE_MACRO_TO_SPEED	 
void DataAcqSetDynamic(int32 nValue, uint8 nChannelID) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//��������
	OS_ENTER_CRITICAL();
	nDynSum[nChannelID] = nValue; 	  
	OS_EXIT_CRITICAL();	
}
  
#else

//ʹ�ú��������ĺ�������������ٶ�
#define DataAcqSetDynamic(nValue, nChannelID)								   	\
{																				\
	OS_ENTER_CRITICAL();														\
	nDynSum[nChannelID] = nValue; 												\
	OS_EXIT_CRITICAL();															\
}
#endif	//USE_MACRO_TO_SPEED

#endif	//_DATA_ACQ_GET_DYNAMIC_EN_



/*****************************************************************************************************************
* ��	�ƣ�DataAcqGetDynamic()
* ��	�ܣ���ȡ��������̬����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
#ifndef USE_MACRO_TO_SPEED
DT_STORE DataAcqGetDynamic(uint8 nChannelID) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif													 

	//��ADֵ���㵽������ֵ
	OS_ENTER_CRITICAL();  
	nDynamicAvrg32 = nDynSum[nChannelID];  
	nDynCurGetStore[nChannelID] = (DT_STORE)((float)nDynamicAvrg32 * g_fADParam[nChannelID]);
	OS_EXIT_CRITICAL();
	
	//��ȥ���ֵ
	nDynCurGetStore[nChannelID] -= g_sMesureZero[nChannelID];	
	
	//���ؽ��
	return nDynCurGetStore[nChannelID];	
}
#else

//ʹ�ú��������ĺ�������������ٶ�
#define DataAcqGetDynamic(nChannelID)																	  	\
{																											\
	OS_ENTER_CRITICAL();																					\
	nDynCurGetStore[nChannelID] = (DT_STORE)((float)nDynSum[nChannelID] * g_fADParam[nChannelID]);			\
	nDynCurGetStore[nChannelID] -= g_sMesureZero[nChannelID];												\
	OS_EXIT_CRITICAL();																						\
}

#endif		//USE_MACRO_TO_SPEED
#endif		//_DATA_ACQ_GET_DYNAMIC_EN_



//����ʱ��ʱʹ�õĴ���
//static int32 AdTestValue[4] = {200, 213, 205, 232};
//static int32 AdTestSin[12] = {0, 5, 8, 10, 8, 5, 0, -5, -8, -10, -8, -5};
//    nDynCurGetStore[nChannelID] = AdTestSin[AdTestValue[nChannelID]++];										\
//    if(AdTestValue[nChannelID]	>= 12) AdTestValue[nChannelID]=0;\
//    nDynCurGetStore[nChannelID] = AdTestValue[nChannelID]++;												\
//    if(AdTestValue[nChannelID] > 237)  AdTestValue[nChannelID] = 200;											\
//


//
//���ݱ�����Ҫ����Ϣ
//
#define USE_NEG_INVERT
static uint8 SYS_OPT_SEG nDynCodingLen;
static int8 SYS_OPT_SEG nDynCodingBuff[4];						//��̬���ݱ��뻺����
static int16 SYS_OPT_SEG nDynCodingPrev[DATA_ACQ_COUNT];		//��һ������
static int16 SYS_OPT_SEG nDynCodingCur;							//��ǰ����
static int16 SYS_OPT_SEG nDynCodingDelt;						//���ݲ�ֵ


/*****************************************************************************************************************
* ��	�ƣ�LoadCodingData()
* ��	�ܣ�װ�ر�������
* ��ڲ�����pData: ָ��洢��ʼ��ַ; nDataLen: װ�����ݸ���
* ���ڲ���������󷵻����ݸ���:1��3
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
#ifndef USE_MACRO_TO_SPEED
void LoadCodingData(int8 *pData, uint8 nChannelID) reentrant
{
	//��ȡ��̬����
	nDynCodingCur = DataAcqGetDynamic(nChannelID);

	//���㱾�����ݺ��ϴ����ݵĲ�ֵ
	nDynCodingDelt = nDynCodingCur - nDynCodingPrev[nChannelID];

	//��¼��һ����������
	nDynCodingPrev[nChannelID] = nDynCodingCur;

	if(abs(nDynCodingDelt) < (uint16)0x007F)
	{
		pData[0] = nDynCodingDelt;

		pData[0] = nDynCodingDelt;										
		if(pData[0] < 0) 	  											
		{																
			pData[0] -= 1;			   									
			pData[0] = ~pData[0];	   									
			pData[0] |= 0x80;											
		}																
		nDynCodingLen = 1;												
	}
	else
	{
		pData[0] = 0x80;												
		pData[1] = LABYTE(nDynCodingCur, 0);							
		pData[2] = LABYTE(nDynCodingCur, 1);							
		nDynCodingLen = 3;												
	}
}

#else

//ʹ�ú��������ĺ�������������ٶ�
#define LoadCodingData(pData, nChannelID)								\
{																		\
	DataAcqGetDynamic(nChannelID);										\
	nDynCodingCur = nDynCurGetStore[nChannelID];						\
	nDynCodingDelt = nDynCodingCur - nDynCodingPrev[nChannelID];		\
	nDynCodingPrev[nChannelID] = nDynCodingCur;							\
																		\
	if(abs(nDynCodingDelt) < (uint16)0x007F)							\
	{																	\
		pData[0] = nDynCodingDelt;										\
		if(pData[0] < 0) 	  											\
		{																\
			pData[0] -= 1;			   									\
			pData[0] = ~pData[0];	   									\
			pData[0] |= 0x80;											\
		}																\
		nDynCodingLen = 1;												\
	}																	\
	else																\
	{																	\
		pData[0] = 0x80;												\
		pData[1] = LABYTE(nDynCodingCur, 0);							\
		pData[2] = LABYTE(nDynCodingCur, 1);							\
		nDynCodingLen = 3;												\
	}																	\
}																		
#endif		//USE_MACRO_TO_SPEED
#endif		//_DATA_ACQ_GET_DYNAMIC_EN_


/*****************************************************************************************************************
* ��	�ƣ�DataAcqPushDynamic()
* ��	�ܣ�����ǰ��̬���ݷ���洢��������,���洢������������������洢
* ��ڲ�����Ҫ�����ݸ���
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
void DataAcqPushDynamic(uint8 nLen) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	uint8 i;

	//�������һ����
	while(nLen--)
	{
		//ͨ��0======================================================================================================
		//��ȡ��������
		LoadCodingData(nDynCodingBuff, 0);

		//�洢����̬��������
		for(i=0; i < nDynCodingLen; i++)
		{
			//�����ٽ���
			OS_ENTER_CRITICAL();

			//��¼����
			g_nDynBuff[0][nDynIndex[0]] = nDynCodingBuff[i];
			
			//�˳��ٽ���
			OS_EXIT_CRITICAL();
			
			//��������
			nDynIndex[0]++;							//�����ȼ�һ
			nDynIndex[0] &= DYN_BUFF_LEN_MASK;		//��ֹ���
			
			nCurSectLen[0]++ ;						//�γ��ȼ�һ
			nDynLocalAddr[0]++;						//�ۼӵ�ַ

			//�ж��Ƿ�Ҫ�洢����============================
			if(nCurSectLen[0] >= DYN_STORE_THRES)
			{
				//�����ٽ���
				OS_ENTER_CRITICAL();

				//���ô洢��
				g_nStoreSectStart[0] = nCurSectStart[0];
				g_nStoreSectLen[0] = nCurSectLen[0];

				//�˳��ٽ���
				OS_EXIT_CRITICAL();

				//���µ�ǰ�����
				nCurSectStart[0] += nCurSectLen[0];
				nCurSectStart[0] &= DYN_BUFF_LEN_MASK;	//��ֹ���
				
				//��ǰ�γ�������
				nCurSectLen[0] = 0;
				
				//���ʹ洢�ź���
				PostDataStore(0);
			}
		}
		//---------------------------------------------------------------------------------------------------------



		//ͨ��1======================================================================================================
		//��ȡ��������
		LoadCodingData(nDynCodingBuff, 1);

		//�洢����̬��������
		for(i=0; i < nDynCodingLen; i++)
		{
			//�����ٽ���
			OS_ENTER_CRITICAL();

			//��¼����
			g_nDynBuff[1][nDynIndex[1]] = nDynCodingBuff[i];
			
			//�˳��ٽ���
			OS_EXIT_CRITICAL();

			//��������
			nDynIndex[1]++;							//�����ȼ�һ
			nDynIndex[1] &= DYN_BUFF_LEN_MASK;		//��ֹ���

			nCurSectLen[1]++ ;						//�γ��ȼ�һ
			nDynLocalAddr[1]++;						//�ۼӵ�ַ


			//�ж��Ƿ�Ҫ�洢����============================
			if(nCurSectLen[1] >= DYN_STORE_THRES)
			{
				//�����ٽ���
				OS_ENTER_CRITICAL();

				//���ô洢��
				g_nStoreSectStart[1] = nCurSectStart[1];
				g_nStoreSectLen[1] = nCurSectLen[1];

				//�˳��ٽ���
				OS_EXIT_CRITICAL();

				//���µ�ǰ�����
				nCurSectStart[1] += nCurSectLen[1];
				nCurSectStart[1] &= DYN_BUFF_LEN_MASK;	//��ֹ���
				nCurSectLen[1] = 0;
				
				//���ʹ洢�ź���
				PostDataStore(1);
			}
		}
		//---------------------------------------------------------------------------------------------------------


		//ͨ��2======================================================================================================
		//��ȡ��������
		LoadCodingData(nDynCodingBuff, 2);

		//�洢����̬��������
		for(i=0; i < nDynCodingLen; i++)
		{
			//�����ٽ���
			OS_ENTER_CRITICAL();

			//��¼����
			g_nDynBuff[2][nDynIndex[2]] = nDynCodingBuff[i];
			
			//�˳��ٽ���
			OS_EXIT_CRITICAL();

			//��������
			nDynIndex[2]++;							//�����ȼ�һ
			nDynIndex[2] &= DYN_BUFF_LEN_MASK;		//��ֹ���
			
			nCurSectLen[2]++ ;						//�γ��ȼ�һ
			nDynLocalAddr[2]++;						//�ۼӵ�ַ


			//�ж��Ƿ�Ҫ�洢����============================
			if(nCurSectLen[2] >= DYN_STORE_THRES)
			{
				//�����ٽ���
				OS_ENTER_CRITICAL();

				//���ô洢��
				g_nStoreSectStart[2] = nCurSectStart[2];
				g_nStoreSectLen[2] = nCurSectLen[2];

				//�˳��ٽ���
				OS_EXIT_CRITICAL();

				//���µ�ǰ�����
				nCurSectStart[2] += nCurSectLen[2];
				nCurSectStart[2] &= DYN_BUFF_LEN_MASK;	//��ֹ���
				nCurSectLen[2] = 0;
				
				//���ʹ洢�ź���
				PostDataStore(2);
			}
		}
		//---------------------------------------------------------------------------------------------------------



		//ͨ��3======================================================================================================
		//��ȡ��������
		LoadCodingData(nDynCodingBuff, 3);

		//�洢����̬��������
		for(i=0; i < nDynCodingLen; i++)
		{
			//�����ٽ���
			OS_ENTER_CRITICAL();

			//��¼����
			g_nDynBuff[3][nDynIndex[3]] = nDynCodingBuff[i];
			
			//�˳��ٽ���
			OS_EXIT_CRITICAL();

			//��������
			nDynIndex[3]++;			//�����ȼ�һ
			nDynIndex[3] &= DYN_BUFF_LEN_MASK;		//��ֹ���
			
			nCurSectLen[3]++ ;		//�γ��ȼ�һ
			nDynLocalAddr[3]++;		//�ۼӵ�ַ

			
			//�ж��Ƿ�Ҫ�洢����============================
			if(nCurSectLen[3] >= DYN_STORE_THRES)
			{
				//�����ٽ���
				OS_ENTER_CRITICAL();

				//���ô洢��
				g_nStoreSectStart[3] = nCurSectStart[3];
				g_nStoreSectLen[3] = nCurSectLen[3];

				//�˳��ٽ���
				OS_EXIT_CRITICAL();

				//���µ�ǰ�����
				nCurSectStart[3] += nCurSectLen[3];
				nCurSectStart[3] &= DYN_BUFF_LEN_MASK;	//��ֹ���
				nCurSectLen[3] = 0;
				
				//���ʹ洢�ź���
				PostDataStore(3);
			}
		}
		//---------------------------------------------------------------------------------------------------------


		OS_ENTER_CRITICAL();			//�����ٽ���
		nLocalCount++;					//���±��ض�̬���ݸ���
		OS_EXIT_CRITICAL();				//�˳��ٽ���
	}
}
#endif


/*****************************************************************************************************************
* ��	�ƣ�DataAcqDataStore()
* ��	�ܣ��洢����
* ��ڲ�������
* ���ڲ�������
* ˵	�������ڽ�����̬�ɼ�ʱ�洢С������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
void DataAcqDataStore() reentrant
{
	uint8 i;
	
	//���ô洢��
	for(i=0; i<DATA_ACQ_COUNT; i++)
	{
		//����Ҫ�洢�ĵ�ַ
		g_nStoreSectStart[i] = nCurSectStart[i];
		g_nStoreSectLen[i] = nCurSectLen[i];


		PostDataStore(i);
	}
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�DataLoad()
* ��	�ܣ�װ�ض�̬����
* ��ڲ�������
* ���ڲ��������ݵ�ַ
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
void DataLoad(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint16 nlen,uint8 nChannelID) reentrant
{
	uint32 	DLnLenTmp;
	uint32 	DLnMemReadLen;				//�洢�����ܶ������ݳ���
	uint32 	DLnDynBuffStart;			//�ڻ������еĿ�ʼ����

	//����������=========================================
	if(DYN_BUFF_LEN < nDynLocalAddr[nChannelID])
	{
		//�����ݵ�ַ�ڴ洢����
		if(nStartAddr < nDynLocalAddr[nChannelID] - DYN_BUFF_LEN)
		{
			//�Ӵ洢����װ������
			DLnMemReadLen = nDynLocalAddr[nChannelID] - DYN_BUFF_LEN - nStartAddr;
			DLnLenTmp = (DLnMemReadLen > nlen) ? nlen : DLnMemReadLen;
			MemDataLoad(pSem, pdat, nStartAddr, DLnLenTmp, nChannelID);  

			//�ж��Ƿ��ȡ��ϣ�������
			if(nlen == DLnLenTmp) return;

			//����ʣ��ʣ�೤��
			nlen -= DLnLenTmp;

			//���´洢��ַָ��
			pdat += DLnLenTmp;

			//���¿�ʼ��ַ
			nStartAddr = nDynLocalAddr[nChannelID] - DYN_BUFF_LEN;
		}


		//�ӻ�������ȡ����=================================
		//��ȡ�ڻ������е���Ե�ַ������ʼ��С��DYN_BUFF_LEN
		DLnDynBuffStart = nStartAddr - (nDynLocalAddr[nChannelID] - DYN_BUFF_LEN);	

		//���ϻ�������ʼ��ַ����
		DLnDynBuffStart += nDynIndex[nChannelID];
		DLnDynBuffStart &= DYN_BUFF_LEN_MASK;		//��ֹ���
		while(nlen--)
		{
			*pdat = g_nDynBuff[nChannelID][DLnDynBuffStart];
			pdat++;
			DLnDynBuffStart++;
			DLnDynBuffStart &= DYN_BUFF_LEN_MASK;		//��ֹ���
		}

		return;
	}

	//������δ��=====================================
	else if(0 < nDynLocalAddr[nChannelID])
	{
		memcpy(pdat, g_nDynBuff[nChannelID] + nStartAddr, nlen);
		return;
	}

	//��ֱ�Ӷ������Ӵ洢����ֱ��װ��=============
	MemDataLoad(pSem, pdat, nStartAddr, nlen, nChannelID);
	
	return;
}
#endif


/*****************************************************************************************************************
* ��	�ƣ�DataAcqStart()
* ��	�ܣ��򿪴�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_START_EN_
void DataAcqStart() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//��λ��̬����(��̬����Ҳ��ʹ��һ��)
	DataInitZero(nDynSum);
	DataInitZero(nDynCodingBuff);
	DataInitZero(nDynCodingPrev);
	DataInitZero(bDataNeedStore);

	DataInitZero(nDynIndex);
	DataInitZero(nCurSectStart);
	DataInitZero(nCurSectLen);
	DataInitZero(g_nStoreSectStart);
	DataInitZero(g_nStoreSectLen);
	DataInitZero(nDynCurGetStore);

	DataInitZero(nStoreAddr);	
	DataInitZero(nDynLocalAddr);

	
	//��λ��̬����
	DataAcqResetStaticIndex();


	//���ؿ�ʼ***************************************
	//�������� 
	OS_ENTER_CRITICAL();
	nDataAcqTaskInner = DATA_ACQ_TASK_START;
	OS_EXIT_CRITICAL();
	
	//�������ź�
	OSSemPost(pDataAcqEvent);	
	//���ؽ���***************************************  
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�DataAcqPauseResume()
* ��	�ܣ�����ͣ�лָ�
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_PAUSE_RESUME_
void DataAcqPauseResume() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//��������
	OS_ENTER_CRITICAL();
	nDataAcqTaskInner = DATA_ACQ_TASK_START;
	OS_EXIT_CRITICAL();
	
	//�����ź�
	pDataAcqEvent->OSEventCnt = 0;
	OSSemPost(pDataAcqEvent);
}
#endif

/****************************************************************************
* ��	�ƣ�ADProcess()
* ��	�ܣ�AD����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
extern BOOL 	g_bDetectionSensor;							//�Ƿ��⴫����
extern uint8 	g_nDetectionSensorCount[DATA_ACQ_COUNT];	
void ADProcess(void)
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	int32 nAdValue;

	//�޳�ʱ�ȴ�ADת�����
	ADWaitDataReady();

	//�ж�����·�жϣ�����������
	//ͨ��1
	if(g_ADC_IT_Channel & 0x01) 
	{
		g_ADC_IT_Channel &= ~0x01;
		nAdValue = ADS1246GetValue(0);

		if((g_nState & COLLECTOR_DYNAMIC) != 0)
		{
			//��̬�ɼ�
			DataAcqSetDynamic(nAdValue, 0);
		}
		else if((g_nState & COLLECTOR_STATIC) != 0)
		{
			//��̬�ɼ�
			DataAcqSetStatic(nAdValue, 0);
		}
		else if(g_bDetectionSensor == TRUE)
		{
			//��⴫�����Ƿ����
			ADS1246IsSensorConnected(nAdValue, 0);
		} 
	}

	//ͨ��2
	if(g_ADC_IT_Channel & 0x02) 
	{
		g_ADC_IT_Channel &= ~0x02;
		nAdValue = ADS1246GetValue(1);

		if((g_nState & COLLECTOR_DYNAMIC) != 0)
		{
			//��̬�ɼ�
			DataAcqSetDynamic(nAdValue, 1);
		}
		else if((g_nState & COLLECTOR_STATIC) != 0)
		{
			//��̬�ɼ�
			DataAcqSetStatic(nAdValue, 1);
		}
		else if(g_bDetectionSensor == TRUE)
		{
			//��⴫�����Ƿ����
			ADS1246IsSensorConnected(nAdValue, 1);
		}
	}	

	//ͨ��3
	if(g_ADC_IT_Channel & 0x04) 
	{
		g_ADC_IT_Channel &= ~0x04;
		nAdValue = ADS1246GetValue(2);

		if((g_nState & COLLECTOR_DYNAMIC) != 0)
		{
			//��̬�ɼ�
			DataAcqSetDynamic(nAdValue, 2);
		}
		else if((g_nState & COLLECTOR_STATIC) != 0)
		{
			//��̬�ɼ�
			DataAcqSetStatic(nAdValue, 2);
		}
		else if(g_bDetectionSensor == TRUE)
		{
			//��⴫�����Ƿ����
			ADS1246IsSensorConnected(nAdValue, 2);
		}  
	}

	//ͨ��4
	if(g_ADC_IT_Channel & 0x08) 
	{
		g_ADC_IT_Channel &= ~0x08;
		nAdValue = ADS1246GetValue(3);

		if((g_nState & COLLECTOR_DYNAMIC) != 0)
		{
			//��̬�ɼ�
			DataAcqSetDynamic(nAdValue, 3);
		}
		else if((g_nState & COLLECTOR_STATIC) != 0)
		{
			//��̬�ɼ�
			DataAcqSetStatic(nAdValue, 3);
		}
		else if(g_bDetectionSensor == TRUE)
		{
			//��⴫�����Ƿ����
			ADS1246IsSensorConnected(nAdValue, 3);
		}
	}
}

/****************************************************************************
* ��	�ƣ�SampleProcess()
* ��	�ܣ�AD����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SampleProcess() reentrant
{
	//��Ҫ�洢���ݵĸ���
	uint32 nPushDynNum;

	//�޳�ʱ�ȴ��ź���
	WaitSampleEvent();
	
	//������Ҫ�洢�����ݸ���
	//Ĭ�ϴ�һ���������λ��ʱ��ȱ���ʱ��죬�����ݸ�������λ������
	nPushDynNum = 1;
	if(nHostCount > nLocalCount) 
	{
		nPushDynNum = nHostCount - nLocalCount;
	}
	
	//�洢��̬����
	DataAcqPushDynamic(nPushDynNum);	
	
}


/****************************************************************************
* ��	�ƣ�DataAcqProcess()
* ��	�ܣ����ݲɼ����ﴦ�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
BOOL DataAcqProcess() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif	 

	uint8 i;
	uint8 nTask;

	//�ȴ�����������
	WaitDataAcqEvent();

	//��ȡ����(��ͨ��ͬ���ɼ����������ۺ�)
	OS_ENTER_CRITICAL();
	nTask = GetDataAcqTask();
	SetDataAcqTask(DATA_ACQ_TASK_NONE);
	OS_EXIT_CRITICAL();

	//����������====================================================================================
	if(nTask == DATA_ACQ_TASK_START)
	{
		//����AD
		ADOpen();
	
		//��������
		for(i = 0; i < DATA_ACQ_COUNT; i++)
		{
			//���ò�����
			if((COLLECTOR_STATIC & g_nState) == COLLECTOR_STATIC)
			{
				//��̬�ɼ�ģʽ
				ADSetSampleRate(20, i);
			}
			else											
			{
				//��̬�ɼ�ģʽ
				ADSetSampleRate(nBrgSetSampleRate, i);
			}
			
			//��������
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
		DataAcqParamCacul();			
	
		//��ʼ�ɼ�
		ADStartSample();

		//����
		return TRUE;
	}
	
	//ֹͣ���ݲɼ�====================================================================================
	else if(nTask == DATA_ACQ_TASK_STOP)
	{
		//�ر�AD
		ADClose();
		
		//����
		return TRUE;
	}

	//��⴫�����Ƿ����==============================================================================
	else if(nTask == DATA_ACQ_TASK_DETECTION)
	{
		//���ɼ�����Ӵ�����
		ADSensorDetection(nDataAcqConnected);

		//���ô���������״̬
		g_bTaskBusy = FALSE;

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
void PostDataStore(uint8 nEp) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	OS_ENTER_CRITICAL();
	bDataNeedStore[nEp] = TRUE;
	OS_EXIT_CRITICAL();

	OSSemPost(pDataStoreEvent);
}

/****************************************************************************
* ��	�ƣ�MemEraseProcess()
* ��	�ܣ������洢������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void DataStoreProcess() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	uint8 i;
	uint8 nDataStoreErr;
	uint8 *pStoreDynBuffTmp;

	//�ȴ����ݲɼ��Ĵ洢�¼�
	OSSemPend(pDataStoreEvent, 0, &nDataStoreErr);

	//��ȡҪ�洢���ݵ�ͨ��
	OS_ENTER_CRITICAL();
	memcpy(bDataNeedStoreClone, bDataNeedStore, sizeof(bDataNeedStore));
	memset(bDataNeedStore, 0, sizeof(bDataNeedStore)); 
	OS_EXIT_CRITICAL();

	//�洢��Ӧͨ��������
	for(i = 0; i < DATA_ACQ_COUNT; i++)
	{
		if(bDataNeedStoreClone[i] == FALSE) continue;

		if(i == 1)
		{
			nDataStoreErr = 5;
		}
		
		//!!!!ȷ����������СΪ�洢�δ�С��������!!!!!!!!!
		pStoreDynBuffTmp = (uint8*)(g_nDynBuff[i] + g_nStoreSectStart[i]);			
		MemDataStore(pDataStoreDiskSem, pStoreDynBuffTmp, nStoreAddr[i], 0x0800, i);

		//�洢��ַ����
		OS_ENTER_CRITICAL();
		nStoreAddr[i] += 0x0800;
		OS_EXIT_CRITICAL();
	}	
}



