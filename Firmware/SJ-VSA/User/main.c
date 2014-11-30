/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: mian.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��2��25��
**����޸�����: 2008��3��11��
**��        ��: ������Դ�ļ�
**ע        ��: ����OsCpu\OS_CFG.H�����ú��ʵ�OS_MAX_TASKS������OS_MAX_EVENTS�����ȡ�
**�� �� ˵  ��������ǰȷ�ϣ�1.��·�汾�ţ�2.ID��FLASH����
*****************************************************************************************************************/

/**********ϵͳ����********************
//Keil ARM ������
//������ʱ�ӣ�72MHz
//�߳��л�ʱ��10us
//uint32 �����������߼����㣺30��ָ������
//float �Ӽ������㣺90��ָ�����ڣ�������150��ָ������
//memcpy 2K���ݣ���ʱ22us
//forѭ�����鸴��2K���ݣ���ʱ750us!!!
//nand flash(K9F1G08U0A) д��2k���ݣ���ʱ550us
//nand flash(K9F1G08U0A) ��ȡ2k���ݣ���ʱ800us
*/

#include "includes.h"
#include "stdarg.h"
//#include "DataManage.h"
//#include "DataAcq.h"
//#include "CommApp.h"
//#include "DS18B20\DS18B20.h"
//#include "Bsp\LowPower.h"
//#include "PowerManager.h"


//���������=========================================================================
#define  TASK_STK_SIZE                  128      /* Size of each task's stacks (# of WORDs)            */



//������===========================================================================
//�����ջ
OS_STK TaskStartStk[TASK_STK_SIZE];						//��������Ķ�ջ
OS_STK TaskCommStk[TASK_STK_SIZE];						//ͨ������Ķ�ջ
OS_STK TaskDataAcqStk[TASK_STK_SIZE];					//����������Ķ�ջ
OS_STK TaskMonitorStk[TASK_STK_SIZE];					//��Դ��ѹ��������ջ
OS_STK TaskDataStoreStk[TASK_STK_SIZE];					//���ݴ洢�����ջ
OS_STK TaskADStk[TASK_STK_SIZE];						//AD���ݶ�ȡ�����ջ
OS_STK TaskSampleStk[TASK_STK_SIZE];					//���������ջ
OS_STK TaskDiskStk[TASK_STK_SIZE];						//���̹��������ջ
OS_STK TaskPowerKeyStk[TASK_STK_SIZE];					//���������ջ
OS_STK TaskTemperatureStk[TASK_STK_SIZE];				//�¶������ջ
OS_STK TaskDataManageStk[TASK_STK_SIZE];				//�������������ջ
OS_STK TaskLCDStk[TASK_STK_SIZE];						//LCD�����ջ

//������
void TaskStart(void *nouse) reentrant;					//��������					
void TaskComm(void *nouse) reentrant;					//ͨ������
void TaskMonitor(void *nouse) reentrant;				//�������
void TaskDataAcq(void *nouse) reentrant;				//����������
void TaskDataStore(void *nouse) reentrant;				//�洢����
void TaskAD(void *nouse) reentrant;						//AD���ݶ�ȡ����
void TaskSample(void *nouse) reentrant;					//��������
void TaskDisk(void *nouse) reentrant;					//���̹�������
void TaskPowerKey(void *nouse) reentrant;				//������������
void TaskTemperature(void *nouse) reentrant;			//�¶ȼ������
void TaskDataManage(void *nouse) reentrant;				//������������
void TaskLCD(void *nouse) reentrant;					//LCD����
//-----------------------------------------------------------------------------------

//��ڳ���============================================================================
//��ʼ��Ӳ������ʼ��ϵͳ��������������
int main(void)
{
	//��Ҫ���޸Ĵ˺������û���ʼ�����������TaskStart������
	BSP_Init();  											//Ӳ����ʼ��
	OSInit();												//ϵͳ��ʼ��
	OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1],0);		//������������
	OSStart();												//����ϵͳ
	return 0;
}

//��������============================================================================
//��ʼ����������������������
void TaskStart(void *nouse) reentrant
{
	//������뾯��
	nouse = nouse; 			 	
	//BSP������ʼ��
	BspVariInit();

	//�ӳ�100msȷ��
	//OSTimeDly(OS_TICKS_PER_SEC/10);	 

	nouse = nouse; 	
	

	//���ƣ���ʾ���Էſ�����
	PowerLightOn();	
	SigLightOn(); 
	
	//�ȴ��ſ�
//	while(GetSignal(POWER_KEY) == 0);

	//�ſ���ָʾ����˸����
	PowerLightOff();  
	SigLightOff();
	OSTimeDly(OS_TICKS_PER_SEC/2);
	PowerLightOn();
	SigLightOn();
	OSTimeDly(OS_TICKS_PER_SEC/20);
	PowerLightOff();
	SigLightOff();
	OSTimeDly(OS_TICKS_PER_SEC/5);
	PowerLightOn();
	SigLightOn();
	OSTimeDly(OS_TICKS_PER_SEC/20);
	PowerLightOff();
	SigLightOff();		
//	PowerEventClear();

/*	//��Դ�������	
	OSTaskCreate(TaskPowerKey, (void *)0, &TaskPowerKeyStk[TASK_STK_SIZE - 1], 1);	

	//����ADת������
	OSTaskCreate(TaskAD, (void *)0, &TaskADStk[TASK_STK_SIZE - 1], 2);
	
	//��������
	OSTaskCreate(TaskSample, (void *)0, &TaskSampleStk[TASK_STK_SIZE - 1], 3);

	//���̹���
	OSTaskCreate(TaskDisk, (void *)0, &TaskDiskStk[TASK_STK_SIZE - 1], 4);

	//�洢����
	OSTaskCreate(TaskDataStore, (void *)0, &TaskDataStoreStk[TASK_STK_SIZE - 1], 5);

	//����ͨ������
	OSTaskCreate(TaskComm, (void *)0, &TaskCommStk[TASK_STK_SIZE - 1], 6);			

	//������������
	OSTaskCreate(TaskDataManage, (void *)0, &TaskDataManageStk[TASK_STK_SIZE - 1], 7);

	//��������������
	OSTaskCreate(TaskDataAcq, (void *)0, &TaskDataAcqStk[TASK_STK_SIZE - 1], 8);	
	
	//������Դ��������
	OSTaskCreate(TaskMonitor, (void *)0, &TaskMonitorStk[TASK_STK_SIZE - 1], 9);
  */
  	//������Դ��������
	OSTaskCreate(TaskLCD, (void *)0, &TaskLCDStk[TASK_STK_SIZE - 1], 10);

	//ָʾ�Ʒ���
	for(;;)
	{
		LightsProcess();
	}
}

//LCD����
void TaskLCD(void *nouse) reentrant
{
  	nouse = nouse;

	LCDInit();

	for (;;)
	{
		LCDProcess();
		OSTimeDly(5 * OS_TICKS_PER_SEC);
	}
}


/*
//ADת������=============================================================
void TaskAD(void *nouse) reentrant
{
	nouse = nouse;

	for(;;)
	{		
		ADProcess();
	}
}

//���ݳ�������=============================================================
void TaskSample(void *nouse) reentrant
{
	nouse = nouse;

	for(;;)
	{
		SampleProcess();
	}
}

//���̹����߳�
void TaskDisk(void *nouse) reentrant
{
	nouse = nouse;

	//������
	DiskOpen();	 

	for(;;)
	{
		DiskProcess();	
	}
}

//���ݴ洢���񣬸���������ڴ��̹�������֮������===========================
void TaskDataStore(void *nouse) reentrant
{
	nouse = nouse;

	//��ʼ��
	DataStoreInit();

 	for(;;)
	{
		DataStoreProcess();
	}
}

//ͨ������=================================================================
void TaskComm(void *nouse) reentrant
{
	nouse = nouse;

 	//ͨ�ų�ʼ��
	UserCommInit();

	//����������Ϊ57600
	SetCommBaudRate(2, 57600);

	//���ó��泬ʱ
	SetNormalRecvTimeOut();
	
	//��ͨ�Žӿ�
	OpenComm2();

	//ѭ��������
	for(;;)
	{ 
		//�Ƿ�͵�ѹ������͵�ѹ�����ٽ���ͨ��
		if(IsLowPower() == TRUE)
		{
			OSTimeDly(OS_TICKS_PER_SEC*5);
			continue;
		}

		//ͨ��������
		CommProcess();
	} 
}

//�����������񣬸���������ڴ��̹�������֮������============================
void TaskDataManage(void *nouse) reentrant
{
	nouse = nouse;

	//��ʼ����������
	DataManageInit();

	//�Ӵ��̶�ȡ����
	ParamLoad();

	//���ݹ���
 	for(;;)
	{
		DataManagerProcess();
	}
}

//���������񣬸�������������ݹ�������֮������==============================
void TaskDataAcq(void *nouse) reentrant
{
	nouse = nouse;

	//���ݲɼ���ʼ��
	DataAcqInit();

	//���ݲɼ�������ʼ��
	DataAcqParamInit();

	//�Ƿ�Ҫ�Ӳ������������ز���
	if(IsParamEmpty() == TRUE)
	{
		//���õ�����������
		DataAcqParamStore(); 
	}
	else
	{
		//�Ӳ�������������
		DataAcqParamLoad();
	}

	//����ѭ��
	for(;;)
	{
		//����������	
		DataAcqProcess();
	}
}


//��Դ�������==============================================================
void TaskMonitor(void *nouse) reentrant
{
	nouse = nouse;

	//��ʱ1���ִ��
	OSTimeDly(OS_TICKS_PER_SEC);

	//��ʼ����Դ�������
	PowerManagerInit();

	//�������һ�ε�ѹ����
	PowerTest();

	//��Դ��������
 	for(;;)
	{
		PowerManagerProcess();
	}
}


//DS18B20�¶ȼ��
uint8 TempID[5][8];
uint32 g_nTemperature[4];
void TaskTemperature(void *nouse) reentrant
{
	uint8 i;
	nouse = nouse;
	DS18B20_Configuration();
	if(TRUE == OWFirst())
	{
		memcpy(TempID[0], ROM_NO, 8);
		for(i = 1; i<5; i++)
		{
			if(TRUE == OWNext())
			{
				memcpy(TempID[i], ROM_NO, 8);
			}
			else
			{
				break;
			}
			
		}
	}
	for(i = 0;; i++)
	{
		i &= 0x03;
		g_nTemperature[i] = GetTemperature(TempID[i]);
		OSTimeDly(10);
	}
}

*/
