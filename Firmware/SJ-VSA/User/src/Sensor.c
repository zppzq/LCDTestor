/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: Sensor.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��09��18��
**����޸�����: 2007��10��18��
**��        ��: ������Դ�ļ�
*****************************************************************************************************************/
#define 	_SENSOR_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "Sensor.h"
//#include "DataManage.h"
#include "UserComm.h"

//�������========================================================================================================
#define FILTER_BUFF_MODE_NON		1
#define FILTER_BUFF_MODE_SUM		2
#define FILTER_BUFF_MODE_QUEUE		3

#define FILTER_BUFF_MODE 			FILTER_BUFF_MODE_QUEUE
#define USE_PLAN_DATA		  		//���ڷ�ֹAD���²���ʱ�����ݾͿ�������ƽ��
#define USE_MACRO_TO_SPEED


//signal defines--------------------------------------------------------------------------------------------------

//Ӧ���ŵ�Դ������������
//����AD���ÿ�����������΢����
//��AD����ΪN
//
//					   2.5V * 10^6 
//FKBRGVOT_DEFAUT = -------------------
//						   2^23
//����ã�
//#define FKBRGVOT_DEFAUT		0.298f				//����΢��ʱ�����ۼ������
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
//���ۼ����AD����
#define	AD_DEFUAT_STEP		 0.298f


//�ڲ���������
static INT8U Sensorerr;
static OS_EVENT *pSensorEvent;
static BOOL bSensorOpen;

//����������
static uint8 nSensorTaskInner = SENSOR_TASK_NONE;

//����ϵ��===============================================
//g_fADParam = g_fADVolParam * g_fADStoreParam
//AD��ϵ��
float g_fADParam[SENSOR_MAX_COUNT];		

//AD�����ѹϵ��
float g_fADVolParam[SENSOR_MAX_COUNT];

//�������ڼ�¼��ǰAD������������¼��flash��
float	g_fADStep[SENSOR_MAX_COUNT];

//----------------------------------------------------------------------------


//���ݻ���
uint8 nDynIndex[SENSOR_MAX_COUNT];					//��������
uint8 nCurSectStart[SENSOR_MAX_COUNT];				//��ǰ����εĿ�ʼ����
uint8 nCurSectLen[SENSOR_MAX_COUNT];				//��ǰ����εĳ���
uint8 g_nStoreSectStart[SENSOR_MAX_COUNT];			//��ǰ����εĿ�ʼ����
uint8 g_nStoreSectLen[SENSOR_MAX_COUNT];			//��ǰ����εĳ���
int8 g_nDynBuff[SENSOR_MAX_COUNT][DYN_BUFF_LEN];	//��̬���ݻ�����


static uint16 nDynCount[SENSOR_MAX_COUNT];			//��̬���ݸ���
int8 *pCurDynBuff;									//��ǰ������ַ
int8 *pStoreDynBuff;								//���洢��������ַ

//�����ⲿ����===========================================
extern uint32  nLocalCount;				//���ش洢����ֵ

extern uint8 g_nADCurGain[SENSOR_MAX_COUNT];		//��¼��ǰ����

float g_fADStoreParam[SENSOR_MAX_COUNT];		//AD�洢ϵ��

extern uint32 nDynLocalAddr[SENSOR_MAX_COUNT];		//���ݴ洢����ַ����

extern BOOL bDataNeedStore[SENSOR_MAX_COUNT];		//��Ҫ�洢���ݵ�ͨ��

extern BOOL bArrTaskBusy[SENSOR_MAX_COUNT];			//����æ��־

uint8 g_nMeasureType[SENSOR_MAX_COUNT];				//�ɼ�������

/*****************************************************************************************************************
* ��	�ƣ�SensorInit()
* ��	�ܣ���������ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/

/*****************************************************************************************************************
* ��	�ƣ�WaitSensorStart()
* ��	�ܣ��ȴ��򿪴�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef	_SENSOR_START_EN_
void WaitSensorEvent()
{
	OSSemPend(pSensorEvent, 0, &Sensorerr);
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�SetSensorTask()
* ��	�ܣ����ô���������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SetSensorTask(uint8 nTask)
{
	nSensorTaskInner = nTask;
}

/*****************************************************************************************************************
* ��	�ƣ�GetSensorTask()
* ��	�ܣ���ȡ����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
uint8 GetSensorTask() 
{
	return nSensorTaskInner;
}

/*****************************************************************************************************************
* ��	�ƣ�SensorParamCacul()
* ��	�ܣ�����AD����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SensorParamCacul()
{
	uint8 i;
	for(i=0;i<SENSOR_MAX_COUNT;i++)
	{
		if(g_nMeasureType[i] == MEASURE_STRAIN)
		{
			//�����Ӧ�䣬����У��ϵ������AD��ѹϵ��
			g_fADVolParam[i] = g_fADStep[i] / (fp32)(0x01 << g_nADCurGain[i]);
		}
		else 
		{
			//��������ѹ����ʹ�����۲�������AD��ѹϵ��
			g_fADVolParam[i] = AD_DEFUAT_STEP / (fp32)(0x01 << g_nADCurGain[i]);
		}

		//����AD��ϵ��
		g_fADParam[i] = g_fADStoreParam[i] * g_fADVolParam[i];
	}
}
