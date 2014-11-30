/****************************************Copyright (c)************************************************************
**                              
**                         			�����������Ƽ����޹�˾
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: PowerKey.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2011��1��5��
**����޸�����: 2011��1��5��
**��        ��: ���ؿ���
*****************************************************************************************************************/
#include "includes.h"
#include "bsp\PowerKey.h"
#include "bsp\Lights.h"

static OS_EVENT 	*pPowerEvent;						    	//�¼����ƿ�
static uint8 		nPowerErr;									//�����־

//������ʼ��
void PowerKeyInit(void) reentrant
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//������ѹ���ƽţ�����
	MakePushPull(POWER_CTR);
	SetHi(POWER_CTR);

	//��ѹ����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//ӳ���ж�
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);	  	//ADRDY4

	//���������ж�
	
	EXTI->FTSR |= EXTI_Line2;
}

void PowerIntDisable(void) reentrant
{
	EXTI->IMR &= EXTI_Line2;		
}

void PowerIntEnable(void) reentrant
{
	EXTI->IMR |= EXTI_Line2;	
}

/****************************************************************************
* ��	�ƣ�LightsInit()
* ��	�ܣ�ָʾ�Ƴ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void PowerKeyVarInit(void) reentrant
{
	//�����ź���
	pPowerEvent = OSSemCreate(0);
}

//�ػ�
void PowerDown(void) reentrant
{
	SetLo(POWER_CTR);
}

void PowerEventClear(void) reentrant
{
	pPowerEvent->OSEventCnt = 0;
}

void TaskPowerKey(void *nouse) reentrant
{
	nouse = nouse;

	//������
	while(1)
	{
		PowerIntEnable();
		//�ȴ������¼�
		OSSemPend(pPowerEvent, 0, &nPowerErr);

		PowerIntDisable();
		//�ӳٰ���ȷ��
		OSTimeDly(OS_TICKS_PER_SEC/2);

		//�Ƿ���Ȼ����
//		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) != 0)
		if(GetSignal(POWER_KEY) != 0)
		{
			//�ѷſ�������
			continue;
		}

		//ָʾ����1���ػ�
		PowerLightOn();	 
		SigLightOn();
		OSTimeDly(OS_TICKS_PER_SEC);
		PowerLightOff();
		SigLightOff();

		//�ػ�
		PowerDown();
		while(1);
	}
}

extern BOOL	bSleep;
extern int16	nWakeCount;

//�����жϴ���
void EXTI2_IRQHandler()
{
	//�����ж�		 
	OSIntEnter();
	
	//ͨ��3
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line2);
		bSleep = FALSE;		//�˳�����
		nWakeCount = 0;	
		if(GetSignal(POWER_KEY) == 0)
		{
			OSSemPost(pPowerEvent);
		}
	}
	//�˳��ж�
	OSIntExit();							
}






