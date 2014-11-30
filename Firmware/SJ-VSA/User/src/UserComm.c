#include "includes.h"
#include "ADS1246.h"
#include "Sensor.h"
#include "UserComm.h"
#include "CpuPortAccess.h"
#include <ucos_ii.h>

#define COLLECTOR_STATIC 0

uint8 g_nState = 0;							//�ɼ���״̬��
uint16 nBrgSetSampleRate;							//Ҫ���õĲ�����

//���в���
STORE_DATA_TYPE_S g_nCurDataType[SENSOR_MAX_COUNT];	//��¼��ǰҪ�������������

//��¼�Ƿ���Ӵ�������־
uint8 nSensorConnected[SENSOR_MAX_COUNT];
//��ǰ״̬
BOOL bArrTaskBusy[SENSOR_MAX_COUNT];				//����æ��־

//��������
#define	ADPortInit()		ADS1246PortInit()
#define	ADVariInit()		ADS1246VariInit()
#define	ADOpen()			ADS1246Open()
#define	ADClose()			ADS1246Close()
#define	ADSetSampleRate(x,y)	ADS1246SetSampleRate(x,y)
#define	ADSetRange(x,y)		ADS1246SetRange(x,y)
#define	ADSetGain(x,y)		ADS1246SetGain(x,y)
#define	ADStartSample()		ADS1246StartSample()		
#define	ADStopSample()		ADS1246StopSample()
#define	ADGetVol(x)			ADS1246GetVol(x)
#define	ADParamUpdate(x,y)	ADS1246SetParam(x,y)		//x:������y:ͨ��
#define	ADWaitDataReady()	ADS1246WaitDataReady()
#define	ADSensorDetection(x)	ADS1246SensorDetection(x)
/****************************************************************************
* ��	�ƣ�SensorProcess()
* ��	�ܣ��������������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
BOOL SensorProcess()
{
	uint8 i = 0;
	uint8 nTask;
	uint8 *pTask = &nTask;
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//�ȴ�����������
	WaitSensorEvent();

	//��ȡ����(��ͨ��ͬ���ɼ����������ۺ�)
	OS_ENTER_CRITICAL();
	nTask = 1;//GetSensorTask();
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