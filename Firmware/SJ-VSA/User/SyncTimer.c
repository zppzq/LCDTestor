/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: SyncTimer.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2011��04��05��
**����޸�����: 2011��04��05��
**��        ��: ͬ����ʱ��Դ�ļ�
*****************************************************************************************************************/
#include "includes.h"
#include "SyncTimer.h"

//�ⲿ����
extern BOOL   g_nAllowedSample;						//�Ƿ��������



uint32 nTicketPer100us;								//ÿ100us�Ķ�ʱ������
float fTicketPer100us;								//ÿ100us�Ķ�ʱ������(������)


uint32 	SyncTimerCounter = 0;						//��ʱ��3�ֶμ�����
uint8 	SyncTimerCounterMax = 4;					//��ʱ��3�ֶμ��������ֵ
uint32  nCycleTicketLeft = 0;						//��¼����Ӧ�ø���ʱ��Ļ�������
																						  
/****************************************************************************
* ��	�ƣ�SyncTimerInit()
* ��	�ܣ���ʱ��3��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SyncTimerInit() reentrant
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//ʼ������
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 20;  						// TIM_Period
	TIM_TimeBaseStructure.TIM_Prescaler = SYSCLK / 2 / 1000;		//TIM_Prescaler
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	TIM_SetCounter(TIM5, 0);
	TIM_ARRPreloadConfig(TIM5, ENABLE);	  //�Զ���װ��Ԥװ������

	
	//�ڲ�������ʼ��


}

void SyncTimerStop()
{
	TIM_Cmd(TIM5, DISABLE);		//��ֹ������
	TIM_SetCounter(TIM5, 0);	//���������
	TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);	 //��ֹ�ж�
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);	 //����жϱ�־		
}

void SyncTimerStart()
{
	//�����һ�μ��ж�
	TIM5->CR1 = (1 << 2) |	(1 << 0);			//������������ж�
	TIM5->EGR = 1 << 0;							//���������¼�
	TIM5->CR1 &= ~(1 << 0);						//��ֹ������
	TIM5->CR1 &= ~(1 << 2);						//��ֹ�ж�
	TIM5->SR &= ~(1<<0);        				//����жϱ�־		

	//������ʱ��
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);	 //������������ж�	
	TIM_Cmd(TIM5, ENABLE);					 	 //���������	
}

//����жϱ�־
void ClearSyncTimerISR()
{
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);	 //����жϱ�־			
}


//��ֹ�ж�
void CloseSyncTimerISR()
{
	TIM_Cmd(TIM5, DISABLE);					 	 //��ֹ������
	TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);	 //��ֹ�ж�
}

//�����ж�
void OpenSyncTimerISR()
{
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);	 //������������ж�	
	TIM_Cmd(TIM5, ENABLE);					 	 //���������
}


//��λ����ֵ������
void SyncTimerResetRun() reentrant
{
	TIM_SetCounter(TIM5, 0);

   	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);	 //������������ж�	
	TIM_Cmd(TIM5, ENABLE);					 	 //���������
}


//���ö�ʱ����ʱƵ��
void SetSyncTimerRate(int nRate) reentrant
{
	uint16 nCount = 0;
	uint16 nPrescaler = 0;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint32 nSysClk = SYSCLK;

	//�淶��������
	nRate = (nRate < AD_RATE_MIN) ? AD_RATE_MIN : nRate;
	nRate = (nRate > AD_RATE_MAX) ? AD_RATE_MAX : nRate;

	//���㶨ʱ������
	
	if(nSysClk/2/nRate < 60000)
	{
		nPrescaler = 2;
		nCount = nSysClk/2/nRate;
	}
	else if(nSysClk/20/nRate < 60000)
	{
		nPrescaler = 20;
		nCount = nSysClk/20/nRate;	
	}
	else if(nSysClk/200/nRate < 60000)
	{
		nPrescaler = 200;
		nCount = nSysClk/200/nRate;	
	}
	else if(nSysClk/2000/nRate < 60000)
	{
		nPrescaler = 2000;
		nCount = nSysClk/2000/nRate;	
	}
		
	//���ò���
	TIM_TimeBaseStructure.TIM_Period = nCount - 1;  				//װ��ֵӦ����1
	TIM_TimeBaseStructure.TIM_Prescaler = nPrescaler - 1;			//װ��ֵӦ����1
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	//���ö�ʱ��
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	
	//Ԥ�ȼ���һЩ����
	fTicketPer100us	= 100.0f / (1000000.0f * (float)nPrescaler / (float)SYSCLK);
	nTicketPer100us = fTicketPer100us;
}

//У��������
uint16 nTimeSetData = 0;
void SyncTimerCaliAndRun() reentrant
{
	//��ҪԤ���ʱ�����
	//nCycleTicketLeft = nCycleTimeLeft * nTicketPer100us;  	

	//����ֶ���Ŀ
	//SyncTimerCounter = nCycleTicketLeft / nRateTime; 	
	//SyncTimerCounter = SyncTimerCounterMax - SyncTimerCounter - 1;

	//����ֶκ�ʣ���ʱ��
	//nCycleTicketLeft = nCycleTicketLeft % nRateTime;

	//����ʣ��ʱ��
	//nTimeSetData = -nCycleTicketLeft;
	
	//��ʼ����
	SyncTimerStart();
}

//�жϴ������
void TIM5_IRQHandler (void) 
{
	//���ж�
	OSIntEnter();

	if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)	
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);	 //����жϱ�־	      
		
		//SyncTimerCounter++;

		//��������
		if(g_nAllowedSample == TRUE)
		{
			PostSampleSem();
		}

	}

	//���ж�
	OSIntExit();
}







