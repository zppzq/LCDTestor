/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: SyncTimer.c
**创   建   人: 杨承凯
**创 建 日  期: 2011年04月05日
**最后修改日期: 2011年04月05日
**描        述: 同步定时器源文件
*****************************************************************************************************************/
#include "includes.h"
#include "SyncTimer.h"

//外部变量
extern BOOL   g_nAllowedSample;						//是否允许采样



uint32 nTicketPer100us;								//每100us的定时器个数
float fTicketPer100us;								//每100us的定时器个数(浮点数)


uint32 	SyncTimerCounter = 0;						//定时器3分段计数器
uint8 	SyncTimerCounterMax = 4;					//定时器3分段计数器最大值
uint32  nCycleTicketLeft = 0;						//记录本地应该更新时间的机器周期
																						  
/****************************************************************************
* 名	称：SyncTimerInit()
* 功	能：定时器3初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void SyncTimerInit() reentrant
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//始终配置
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 20;  						// TIM_Period
	TIM_TimeBaseStructure.TIM_Prescaler = SYSCLK / 2 / 1000;		//TIM_Prescaler
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	TIM_SetCounter(TIM5, 0);
	TIM_ARRPreloadConfig(TIM5, ENABLE);	  //自动重装载预装载允许

	
	//内部变量初始化


}

void SyncTimerStop()
{
	TIM_Cmd(TIM5, DISABLE);		//禁止计数器
	TIM_SetCounter(TIM5, 0);	//清零计数器
	TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);	 //禁止中断
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);	 //清除中断标志		
}

void SyncTimerStart()
{
	//清除第一次假中断
	TIM5->CR1 = (1 << 2) |	(1 << 0);			//允许计数器和中断
	TIM5->EGR = 1 << 0;							//产生更新事件
	TIM5->CR1 &= ~(1 << 0);						//禁止计数器
	TIM5->CR1 &= ~(1 << 2);						//禁止中断
	TIM5->SR &= ~(1<<0);        				//清除中断标志		

	//开启定时器
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);	 //允许产生更新中断	
	TIM_Cmd(TIM5, ENABLE);					 	 //允许计数器	
}

//清除中断标志
void ClearSyncTimerISR()
{
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);	 //清除中断标志			
}


//禁止中断
void CloseSyncTimerISR()
{
	TIM_Cmd(TIM5, DISABLE);					 	 //禁止计数器
	TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);	 //禁止中断
}

//开启中断
void OpenSyncTimerISR()
{
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);	 //允许产生更新中断	
	TIM_Cmd(TIM5, ENABLE);					 	 //允许计数器
}


//复位计数值并运行
void SyncTimerResetRun() reentrant
{
	TIM_SetCounter(TIM5, 0);

   	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);	 //允许产生更新中断	
	TIM_Cmd(TIM5, ENABLE);					 	 //允许计数器
}


//设置定时器定时频率
void SetSyncTimerRate(int nRate) reentrant
{
	uint16 nCount = 0;
	uint16 nPrescaler = 0;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint32 nSysClk = SYSCLK;

	//规范化采样率
	nRate = (nRate < AD_RATE_MIN) ? AD_RATE_MIN : nRate;
	nRate = (nRate > AD_RATE_MAX) ? AD_RATE_MAX : nRate;

	//计算定时器参数
	
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
		
	//配置参数
	TIM_TimeBaseStructure.TIM_Period = nCount - 1;  				//装载值应当减1
	TIM_TimeBaseStructure.TIM_Prescaler = nPrescaler - 1;			//装载值应当减1
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	//设置定时器
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	
	//预先计算一些参数
	fTicketPer100us	= 100.0f / (1000000.0f * (float)nPrescaler / (float)SYSCLK);
	nTicketPer100us = fTicketPer100us;
}

//校正并运行
uint16 nTimeSetData = 0;
void SyncTimerCaliAndRun() reentrant
{
	//需要预设的时间节拍
	//nCycleTicketLeft = nCycleTimeLeft * nTicketPer100us;  	

	//计算分段数目
	//SyncTimerCounter = nCycleTicketLeft / nRateTime; 	
	//SyncTimerCounter = SyncTimerCounterMax - SyncTimerCounter - 1;

	//计算分段后剩余的时间
	//nCycleTicketLeft = nCycleTicketLeft % nRateTime;

	//计算剩余时间
	//nTimeSetData = -nCycleTicketLeft;
	
	//开始运行
	SyncTimerStart();
}

//中断处理程序
void TIM5_IRQHandler (void) 
{
	//进中断
	OSIntEnter();

	if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)	
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);	 //清除中断标志	      
		
		//SyncTimerCounter++;

		//采样数据
		if(g_nAllowedSample == TRUE)
		{
			PostSampleSem();
		}

	}

	//出中断
	OSIntExit();
}







