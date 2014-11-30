/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Seri1.c
**创   建   人: 罗仕强
**创 建 日  期: 2007年3月15日

**改		编: 杨承凯(kady1984@163.com)
**最后修改日期: 2007年10月9日

**描        述: 基于uCosII的异步串口收发 
********************************************************************************************************/
#define _SERIAL_C_

//需要的头文件---------------------------------------------------------------
#include "includes.h"

//信号定义-------------------------------------------------------------------
//USART1
#ifdef _USE_COMM1_
#define COMM1_485_PORT	GPIOA 
#define COMM1_485		8
#endif

//USART2
#ifdef _USE_COMM2_
#define COMM2_485_PORT	GPIOA
#define COMM2_485		1
#endif

//USART3
#ifdef _USE_COMM3_
#define COMM3_485_PORT	GPIOA
#define COMM3_485		5
#endif

//数据定义
static DCB 			g_Dcb[3];									//设备控制块			

//USART1
#ifdef _USE_COMM1_
static OS_EVENT 	*m_pComm1Event;		//通道1事件控制块
static OS_EVENT 	*m_pSendSyncEvent1;	//通道1发送同步事件
#endif

//USART2
#ifdef _USE_COMM2_
static OS_EVENT 	*m_pComm2Event;	    //通道2事件控制块
static OS_EVENT 	*m_pSendSyncEvent2;	//通道2发送同步事件
#endif

//USART3
#ifdef _USE_COMM3_
static OS_EVENT 	*m_pComm3Event;	    //通道2事件控制块
static OS_EVENT 	*m_pSendSyncEvent3;	//通道2发送同步事件
#endif


#ifdef _USE_COMM1_
uint8 g_pSendBuff1[256];
uint8 g_pRecvBuff1[256];
#endif

#ifdef _USE_COMM3_
uint8 g_pSendBuff2[256];
uint8 g_pRecvBuff2[256];
#endif

#ifdef _USE_COMM2_
uint8 g_pSendBuff3[256];
uint8 g_pRecvBuff3[256];
#endif

/****************************************************************************
* 名	称：Comm1VariInit()
* 功	能：串口全局变量初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void UsartVariInit(void)
{

//////////////////////////UART1////////////////////////////////////////
#ifdef _USE_COMM1_	 
	g_Dcb[0].Usart = USART1;
	g_Dcb[0].Port = GPIOA;
	g_Dcb[0].UsartClk = RCC_APB2Periph_USART1;
	g_Dcb[0].PortClk = RCC_APB2Periph_GPIOA;
	g_Dcb[0].RxPin = GPIO_Pin_10;
	g_Dcb[0].TxPin = GPIO_Pin_9;

#ifdef USE_DMA_MODE
#ifdef _USART_DMA_TX__
	g_Dcb[0].Tx_DMA_Channel = DMA1_Channel4;
	g_Dcb[0].Tx_DMA_Flag = DMA1_FLAG_TC4;
#endif

#ifdef _USART_DMA_RX__
	g_Dcb[0].Rx_DMA_Channel = DMA1_Channel5;
	g_Dcb[0].Rx_DMA_Flag = DMA1_FLAG_TC5;
#endif

	g_Dcb[0].DMA_DR_Base = 0x40013804;
#endif

	g_Dcb[0].Timer = TIM2;						//通道1接收定时器
	g_Dcb[0].TimerClk = RCC_APB1Periph_TIM2;	//通道1定时器时钟

	g_Dcb[0].nBaudRate = 9600;					//波特率							
	g_Dcb[0].nVerify = USART_Parity_No;	    	//校验位,0为NONE，1为偶校验
	
	g_Dcb[0].pSendBuff = g_pSendBuff1;
	g_Dcb[0].pRecvBuff = g_pRecvBuff1;
	g_Dcb[0].nReceiveIndex = 0;				//接收索引
	g_Dcb[0].nSendIndex = 0;				//发送索引
	g_Dcb[0].bReceiving = 0;				//接收标志
	g_Dcb[0].bReceiveEnd =  0;				//接收完标志
	g_Dcb[0].bSendEnd = 1;					//发送完标志
	g_Dcb[0].bIsReceivedData = 0; 			//是否收到数据
	g_Dcb[0].nCommErr = 0;					//错误标志
	g_Dcb[0].nRecvTimeout = 0;				//超时定时器
	g_Dcb[0].bSendSync = 1;
	m_pComm1Event = NULL;		//通道1事件控制块
	m_pSendSyncEvent1 = NULL;	//通道1发送同步事件
#endif


//////////////////////////UART2////////////////////////////////////////
#ifdef _USE_COMM2_ 
	g_Dcb[1].Usart = USART2;
	g_Dcb[1].Port = GPIOA;
	g_Dcb[1].UsartClk = RCC_APB1Periph_USART2;
	g_Dcb[1].PortClk = RCC_APB2Periph_GPIOA;
	g_Dcb[1].RxPin = GPIO_Pin_3;
	g_Dcb[1].TxPin = GPIO_Pin_2;

#ifdef USE_DMA_MODE

#ifdef _USART_DMA_TX__
	g_Dcb[1].Tx_DMA_Channel = DMA1_Channel7;
	g_Dcb[1].Tx_DMA_Flag = DMA1_FLAG_TC7;
#endif

#ifdef _USART_DMA_RX__
	g_Dcb[1].Rx_DMA_Channel = DMA1_Channel6;
	g_Dcb[1].Rx_DMA_Flag = DMA1_FLAG_TC6;
#endif

	g_Dcb[1].DMA_DR_Base = 0x40004404;
#endif

	g_Dcb[1].Timer = TIM3;						//通道2接收定时器
	g_Dcb[1].TimerClk = RCC_APB1Periph_TIM3;	//通道2定时器时钟
	
	g_Dcb[1].nBaudRate = 9600;					//波特率							
	g_Dcb[1].nVerify = USART_Parity_No;	    	//校验位,0为NONE，1为偶校验
	
	g_Dcb[1].pSendBuff = g_pSendBuff3;
	g_Dcb[1].pRecvBuff = g_pRecvBuff3;
	g_Dcb[1].nReceiveIndex = 0;				//接收索引
	g_Dcb[1].nSendIndex = 0;				//发送索引
	g_Dcb[1].bReceiving = 0;				//接收标志
	g_Dcb[1].bReceiveEnd =  0;				//接收完标志
	g_Dcb[1].bSendEnd = 1;					//发送完标志
	g_Dcb[1].nCommErr = 0;					//错误标志
	g_Dcb[1].nRecvTimeout = 0;				//超时定时器
	g_Dcb[1].bSendSync = 1;
	m_pComm2Event = NULL;	    			//通道3事件控制块
	m_pSendSyncEvent2 = NULL;				//通道3发送同步事件
#endif


//////////////////////////UART3////////////////////////////////////////
#ifdef _USE_COMM3_	  
	g_Dcb[2].Usart = USART3;
	g_Dcb[2].Port = GPIOB;
	g_Dcb[2].UsartClk = RCC_APB1Periph_USART3;
	g_Dcb[2].PortClk = RCC_APB2Periph_GPIOB;
	g_Dcb[2].RxPin = GPIO_Pin_11;
	g_Dcb[2].TxPin = GPIO_Pin_10;

#ifdef USE_DMA_MODE

#ifdef _USART_DMA_TX__
	g_Dcb[2].Tx_DMA_Channel = DMA1_Channel2;
	g_Dcb[2].Tx_DMA_Flag = DMA1_FLAG_TC2;
#endif

#ifdef _USART_DMA_RX__
	g_Dcb[2].Rx_DMA_Channel = DMA1_Channel3;
	g_Dcb[2].Rx_DMA_Flag = DMA1_FLAG_TC3;
#endif

	g_Dcb[2].DMA_DR_Base = 0x40004804;
#endif

	g_Dcb[2].Timer = TIM4;						//通道3接收定时器
	g_Dcb[2].TimerClk = RCC_APB1Periph_TIM4;	//通道3定时器时钟

	g_Dcb[2].nBaudRate = 9600;					//波特率							
	g_Dcb[2].nVerify = USART_Parity_No;	    	//校验位,0为NONE，1为偶校验
	
	g_Dcb[2].pSendBuff = g_pSendBuff2;
	g_Dcb[2].pRecvBuff = g_pRecvBuff2;
	g_Dcb[2].nReceiveIndex = 0;					//接收索引
	g_Dcb[2].nSendIndex = 0;					//发送索引
	g_Dcb[2].bReceiving = 0;					//接收标志
	g_Dcb[2].bReceiveEnd =  0;					//接收完标志
	g_Dcb[2].bSendEnd = 1;						//发送完标志
	g_Dcb[2].nCommErr = 0;						//错误标志
	g_Dcb[2].nRecvTimeout = 100;				//超时定时器
	g_Dcb[2].bSendSync = 1;
	m_pComm3Event = NULL;	    				//通道3事件控制块
	m_pSendSyncEvent3 = NULL;					//通道3发送同步事件
#endif

}

/****************************************************************************
* 名	称：SetCommBaudRate()
* 功	能：设置波特率
* 入口参数：nPort，端口；nBaudRate，波特率
* 出口参数：无
* 说	明：在Open之前，如果已经Open了，则要重启Open
****************************************************************************/
void SetCommBaudRate(uint8 nPort, uint32 nBaudRate)
{
	//内部零索引
	nPort -= 1;
	if(nPort >= 3) return;

	//设置波特率
	g_Dcb[nPort].nBaudRate = nBaudRate;
}

/****************************************************************************
* 名	称：SetCommRecvTimeOut()
* 功	能：设置接收超时时间
* 入口参数：nPort，端口；nRecvTimeout，超时时间(ms)
* 出口参数：无
* 说	明：随时可设置
****************************************************************************/
void SetCommRecvTimeOut(uint8 nPort, uint16 nRecvTimeout)
{
	//内部零索引
	nPort -= 1;
	if(nPort >= 3) return;

	//方式在输入1—9时除以10等于0
	if((nRecvTimeout >= 1 ) && (nRecvTimeout <= 9 ))
	{
		nRecvTimeout = 10;
	}

	//设置接收超时时间
	g_Dcb[nPort].nRecvTimeout = nRecvTimeout / 10;
}


/****************************************************************************
* 名	称：CommPortOpen()
* 功	能：打开端口
* 入口参数：nPort，端口
* 出口参数：无
* 说	明：无
****************************************************************************/
void CommPortOpen(uint8 nPort)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//内部零索引
	nPort -= 1;
	if(nPort >= 3) return;

	//设置发送端口属性
	GPIO_InitStructure.GPIO_Pin = g_Dcb[nPort].TxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(g_Dcb[nPort].Port, &GPIO_InitStructure);

	//设置接收端口属性
	GPIO_InitStructure.GPIO_Pin = g_Dcb[nPort].RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(g_Dcb[nPort].Port, &GPIO_InitStructure);
}


/****************************************************************************
* 名	称：CommPortShut()
* 功	能：关闭端口
* 入口参数：nPort，端口
* 出口参数：无
* 说	明：无
****************************************************************************/
void CommPortShut(uint8 nPort)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//内部零索引
	nPort -= 1;
	if(nPort >= 3) return;

	//设置发送端口属性
	GPIO_InitStructure.GPIO_Pin = g_Dcb[nPort].TxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(g_Dcb[nPort].Port, &GPIO_InitStructure);

	//设置接收端口属性
	GPIO_InitStructure.GPIO_Pin = g_Dcb[nPort].RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(g_Dcb[nPort].Port, &GPIO_InitStructure);
}

/****************************************************************************
* 名	称：Comm1PortInit()
* 功	能：串口端口初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void UART_Configuration(DCB* pDCB)
{
	USART_InitTypeDef USART_InitStructure; 

#ifdef USE_DMA_MODE
	DMA_InitTypeDef DMA_InitStructure;
#endif

    GPIO_InitTypeDef GPIO_InitStructure;
	
	//配置时钟
	RCC_APB2PeriphClockCmd(pDCB->PortClk, ENABLE);
	
	if(pDCB->UsartClk == RCC_APB2Periph_USART1)
	{
		RCC_APB2PeriphClockCmd(pDCB->UsartClk, ENABLE);
	}
	else
	{
		RCC_APB1PeriphClockCmd(pDCB->UsartClk, ENABLE);
	}
	
	//配置UART参数
	USART_InitStructure.USART_BaudRate = pDCB->nBaudRate; 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(pDCB->Usart, &USART_InitStructure);

	//使能串口接收中断
	USART_ITConfig(pDCB->Usart, USART_IT_RXNE, ENABLE);
	
	//使能串口
	USART_Cmd(pDCB->Usart, ENABLE);

#ifdef USE_DMA_MODE

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

#ifdef _USART_DMA_TX__
	DMA_DeInit(pDCB->Tx_DMA_Channel);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = pDCB->DMA_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pDCB->pSendBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(pDCB->Tx_DMA_Channel, &DMA_InitStructure);	 //pDCB->Tx_DMA_Channel
	DMA_ITConfig(pDCB->Tx_DMA_Channel, DMA_IT_TC, ENABLE); //pDCB->Tx_DMA_Channel   
#endif

#ifdef _USART_DMA_RX__
	DMA_DeInit(pDCB->Rx_DMA_Channel);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = pDCB->DMA_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pDCB->pRecvBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(pDCB->Rx_DMA_Channel, &DMA_InitStructure);
	DMA_ITConfig(pDCB->Rx_DMA_Channel, DMA_IT_TC, ENABLE);    
#endif

#endif

#ifdef _USE_COMM1_
#ifdef	_COMM1_485_
	if(pDCB->Usart == USART1)
		MakePushPull(COMM1_485);
#endif
#endif

#ifdef _USE_COMM2_
#ifdef	_COMM2_485_
	if(pDCB->Usart == USART2)
		MakePushPull(COMM2_485);
#endif
#endif


#ifdef _USE_COMM3_
#ifdef	_COMM3_485_
	if(pDCB->Usart == USART3)
		MakePushPull(COMM3_485);
#endif
#endif

	//设置发送端口属性
	GPIO_InitStructure.GPIO_Pin = pDCB->TxPin;		//UART4 TxPin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(pDCB->Port, &GPIO_InitStructure);

	//设置接收端口属性
	GPIO_InitStructure.GPIO_Pin = pDCB->RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(pDCB->Port, &GPIO_InitStructure);

	//清除发送完成标志
	USART_ClearFlag(pDCB->Usart, USART_FLAG_TC); 
}

void Timer_Config(DCB *pDCB) 
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(pDCB->TimerClk, ENABLE);

	//超时定时器配置
	TIM_TimeBaseStructure.TIM_Period = 30;  // TIM_Period
	TIM_TimeBaseStructure.TIM_Prescaler = SYSCLK / pDCB->nBaudRate - 1;	//TIM_Prescaler
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	
	TIM_TimeBaseInit(pDCB->Timer, &TIM_TimeBaseStructure);

	TIM_SetCounter(pDCB->Timer, 0);
	TIM_ARRPreloadConfig(pDCB->Timer, ENABLE);	  //自动重装载预装载允许	
	
	//以下对TIM2设置的代码用于使TIM2产生一次更新事件，目的是让TIM2
	//预分频器的缓冲区置入预装载寄存器的值(TIM2_PSC寄存器的内容);
	//自动装载影子寄存器被重新置入预装载寄存器的值(TIM2_ARR)。
	//之所以这么做是因为TIM2第一次定时时间不准确,通过分析认为是第一
	//次记数时影子寄存器的值不确定造成的,所以在此事先产生一次更新
	//事件以更新那些不能被操作的寄存器.
//	TIM_ITConfig(pDCB->Timer, TIM_IT_Update, ENABLE);	 //允许产生更新中断	
	TIM_Cmd(pDCB->Timer, ENABLE);					 	 //允许定时器	
	TIM_GenerateEvent(pDCB->Timer, TIM_EventSource_Update);//产生更新事件
	TIM_Cmd(pDCB->Timer, DISABLE);					 	 //禁止定时器	
	TIM_ITConfig(pDCB->Timer, TIM_IT_Update, DISABLE);	 //禁止中断	
	TIM_ClearITPendingBit(pDCB->Timer, TIM_IT_Update);		//清除中断标志	      
	/*	TIM2->CR1 = (1 << 2) |	(1 << 0);			//允许定时器和中断
	TIM2->EGR = 1 << 0;							//产生更新事件
	TIM2->CR1 &= ~(1 << 0);						//禁止定时器
	TIM2->CR1 &= ~(1 << 2);						//禁止中断
	TIM2->SR &= ~(1<<0);        				//清除中断标志	*/	
}

/****************************************************************************
* 名	称：OpenComm1()
* 功	能：打开串口
* 入口参数：无
* 出口参数：无
* 说	明：设置串口参数,在调用此函数之前请先初始化DCB,即SetDCB()
****************************************************************************/
#ifdef _USE_COMM1_

//串口1
void OpenComm1(void)
{
	//配置
	UART_Configuration(&g_Dcb[0]);
	Timer_Config(&g_Dcb[0]);

	//设置485接收
#ifdef	_COMM1_485_
	SetLo(COMM1_485);		
#endif

	//创建信号量
	if(m_pComm1Event == NULL) m_pComm1Event = OSSemCreate(0);
	if(m_pSendSyncEvent1 == NULL) m_pSendSyncEvent1 = OSSemCreate(0);

	//延时等待初始化状态
	OSTimeDly(OS_TICKS_PER_SEC/50);

	//清除发送完中断标志
	USART_ClearITPendingBit(g_Dcb[0].Usart, USART_FLAG_TC);	
}
#endif	//_USE_COMM1_


//串口2
#ifdef _USE_COMM2_
void OpenComm2(void)
{
	//配置
	UART_Configuration(&g_Dcb[1]);
	Timer_Config(&g_Dcb[1]);

	//设置485接收
#ifdef	_COMM2_485_
	SetLo(COMM2_485);		
#endif
	
	//创建信号量
	if(m_pComm2Event == NULL) m_pComm2Event = OSSemCreate(0);
	if(m_pSendSyncEvent2 == NULL) m_pSendSyncEvent2 = OSSemCreate(0);
	
	//延时等待初始化状态
	OSTimeDly(OS_TICKS_PER_SEC/50);
	
	//清除发送完中断标志
	USART_ClearITPendingBit(g_Dcb[1].Usart, USART_FLAG_TC);
}
#endif	//_USE_COMM2_

//串口3
#ifdef _USE_COMM3_
void OpenComm3(void)
{
	//配置
	UART_Configuration(&g_Dcb[2]);
	Timer_Config(&g_Dcb[2]);

	//设置485接收
#ifdef	_COMM3_485_
	SetLo(COMM3_485);
#endif
	
	//创建信号量
	if(m_pComm3Event == NULL) m_pComm3Event = OSSemCreate(0);
	if(m_pSendSyncEvent3 == NULL) m_pSendSyncEvent3 = OSSemCreate(0);
	
	//延时等待初始化状态
	OSTimeDly(OS_TICKS_PER_SEC/50);

	//清除发送完中断标志
	USART_ClearITPendingBit(g_Dcb[2].Usart, USART_FLAG_TC);	
}
#endif	//_USE_COMM3_

/****************************************************************************
* 名	称：CloseComm1()
* 功	能：关闭串口
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/

//串口1
#ifdef _USE_COMM1_
void CloseComm1(void)
{
	USART_DeInit(g_Dcb[0].Usart);
	USART_Cmd(g_Dcb[0].Usart, DISABLE);

#ifdef	_COMM1_485_
	SetLo(COMM1_485);		//设置为接收状态
#endif

#ifdef _USART_DMA_TX__
	DMA_DeInit(g_Dcb[0].Tx_DMA_Channel);
	DMA_Cmd(g_Dcb[0].Tx_DMA_Channel, DISABLE);
#endif

#ifdef _USART_DMA_RX__ 
	DMA_DeInit(g_Dcb[0].Rx_DMA_Channel);
	DMA_Cmd(g_Dcb[0].Rx_DMA_Channel, DISABLE);
#endif

	TIM_DeInit(TIM2);
	TIM_Cmd(TIM2, DISABLE);
}
#endif	//_USE_COMM1_


//串口2
#ifdef _USE_COMM2_
void CloseComm2(void)
{
	USART_DeInit(g_Dcb[1].Usart);
	USART_Cmd(g_Dcb[1].Usart, DISABLE);

#ifdef	_COMM2_485_
	SetLo(COMM2_485);		//设置为接收状态
#endif

#ifdef _USART_DMA_TX__
	DMA_DeInit(g_Dcb[1].Tx_DMA_Channel);
	DMA_Cmd(g_Dcb[1].Tx_DMA_Channel, DISABLE);
#endif

#ifdef _USART_DMA_RX__ 
	DMA_DeInit(g_Dcb[1].Rx_DMA_Channel);
	DMA_Cmd(g_Dcb[1].Rx_DMA_Channel, DISABLE);
#endif

	TIM_DeInit(TIM3);
	TIM_Cmd(TIM3, DISABLE);
}
#endif	//_USE_COMM2_


//串口3
#ifdef _USE_COMM3_
void CloseComm3(void)
{
	USART_DeInit(g_Dcb[2].Usart);
	USART_Cmd(g_Dcb[2].Usart, DISABLE);
#ifdef	_COMM3_485_
	SetLo(COMM3_485);		//设置为接收状态
#endif

#ifdef _USART_DMA_TX__
	DMA_DeInit(g_Dcb[2].Tx_DMA_Channel);
	DMA_Cmd(g_Dcb[2].Tx_DMA_Channel, DISABLE);
#endif

#ifdef _USART_DMA_RX__ 
	DMA_DeInit(g_Dcb[2].Rx_DMA_Channel);
	DMA_Cmd(g_Dcb[2].Rx_DMA_Channel, DISABLE);
#endif

	TIM_DeInit(TIM4);
	TIM_Cmd(TIM4, DISABLE);
}
#endif	//_USE_COMM3_


/****************************************************************************
* 名	称：SendComm1()
* 功	能：发送数据
* 入口参数：pSendData是待发送的数据，nDataSize是数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
//串口1
#ifdef _USE_COMM1_
void SendComm1(uint8 *pSendData, int16 nDataSize)
{
	if(nDataSize == 0) return;

	//等待发送完毕
	while(g_Dcb[0].bSendEnd == FALSE);

	//将待发送数据放入发送缓冲区
	g_Dcb[0].nDataSize = nDataSize;
	memcpy(g_Dcb[0].pSendBuff, pSendData, nDataSize);
	
#ifdef	_COMM1_485_
	SetHi(COMM1_485);			//设置为发送状态
#endif
	
	//发送数据
	g_Dcb[0].bSendEnd = FALSE;

#ifdef USE_DMA_MODE		//DMA方式发送
	g_Dcb[0].Tx_DMA_Channel->CNDTR = g_Dcb[0].nDataSize;			// 发送字节数量
	USART_DMACmd(g_Dcb[0].Usart, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(g_Dcb[0].Tx_DMA_Channel, ENABLE);						//始能DMA通道

	//等待发送完成
	OSSemPend(m_pSendSyncEvent1, 0, &g_Dcb[0].nCommErr);

	//此时最后两字节数据还正在传送，等待其完成
	while (USART_GetFlagStatus(g_Dcb[0].Usart, USART_FLAG_TC) == RESET);

	//关RS485
#ifdef	_COMM1_485_	   	
	SetLo(COMM1_485);			
#endif

#else		//USART方式发送

	USART_ClearFlag(USART1, USART_FLAG_TC | USART_IT_TXE);     /* 清发送完成标志，Transmission Complete flag */
	for(i=0; i<g_Dcb[0].nDataSize; i++)
	{
		USART_SendData(g_Dcb[0].Usart, g_Dcb[0].pSendBuff[i]);
		while (USART_GetFlagStatus(g_Dcb[0].Usart, USART_FLAG_TC) == RESET);
	}

#ifdef	_COMM1_485_ 
	SetLo(COMM1_485);
#endif
	g_Dcb[0].bSendEnd = TRUE;
#endif	//USE_DMA_MODE
}
#endif	//_USE_COMM1_



//串口2		  
#ifdef _USE_COMM2_
void SendComm2(uint8 *pSendData, int16 nDataSize)
{
	if(nDataSize == 0) return;

	//等待发送完毕
	while(g_Dcb[1].bSendEnd == FALSE);

	//将待发送数据放入发送缓冲区
	g_Dcb[1].nSendSize = nDataSize;	 
	memcpy(g_Dcb[1].pSendBuff, pSendData, nDataSize);

	//开启485发送
#ifdef _COMM2_485_	   	
	SetHi(COMM2_485);			
#endif
	
	//设置发送状态
	g_Dcb[1].bSendEnd = FALSE;

	//DMA方式发送
#ifdef USE_DMA_MODE	
	
	//用DMA发送不能在DMA中断里关485控制线，当DMA发送完中断的时候串口数据并没有发完
	g_Dcb[1].Tx_DMA_Channel->CNDTR = g_Dcb[1].nSendSize;			// 发送字节数量
	USART_DMACmd(g_Dcb[1].Usart, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(g_Dcb[1].Tx_DMA_Channel, ENABLE);						//始能DMA通道

	//等待发送完成信号量
	OSSemPend(m_pSendSyncEvent2, 0, &g_Dcb[1].nCommErr);  
	
	//此时最后两字节数据还正在传送，等待其完成
	while (USART_GetFlagStatus(g_Dcb[1].Usart, USART_FLAG_TC) == RESET);

	//关RS485
#ifdef	_COMM2_485_	   	
	SetLo(COMM2_485);			
#endif

	
#else
	//循环发送		
	USART_ClearFlag(g_Dcb[1].Usart, USART_FLAG_TC | USART_IT_TXE);
	for(i=0; i<g_Dcb[1].nDataSize; i++)
	{
		USART_SendData(g_Dcb[1].Usart, g_Dcb[1].pSendBuff[i]);
		while (USART_GetFlagStatus(g_Dcb[1].Usart, USART_FLAG_TC) == RESET);
	}

	g_Dcb[1].bSendEnd = TRUE;

	//关RS485
#ifdef	_COMM2_485_
	SetLo(COMM2_485);
#endif
	

#endif	//USE_DMA_MODE

}
#endif	//_USE_COMM2_


//串口3
#ifdef _USE_COMM3_
void SendComm3(uint8 *pSendData, int16 nDataSize)
{
	if(nDataSize == 0) return;

	//等待发送完毕
	while(g_Dcb[2].bSendEnd == FALSE);

	//将待发送数据放入发送缓冲区
	g_Dcb[2].nSendSize = nDataSize;
	memcpy(g_Dcb[2].pSendBuff, pSendData, nDataSize);

#ifdef	_COMM3_485_
	SetHi(COMM3_485);			//设置为发送状态
#endif
	
	//发送数据
	g_Dcb[2].bSendEnd = FALSE;


#ifdef USE_DMA_MODE		//DMA方式发送
	
	g_Dcb[2].Tx_DMA_Channel->CNDTR = g_Dcb[2].nSendSize;			// 发送字节数量
	USART_DMACmd(g_Dcb[2].Usart, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(g_Dcb[2].Tx_DMA_Channel, ENABLE);						//始能DMA通道

	//等待发送完成
	OSSemPend(m_pSendSyncEvent3, 0, &g_Dcb[2].nCommErr);

	//此时最后两字节数据还正在传送，等待其完成
	while (USART_GetFlagStatus(g_Dcb[2].Usart, USART_FLAG_TC) == RESET);

	//关RS485
#ifdef	_COMM3_485_	   	
	SetLo(COMM3_485);			
#endif

#else		//USART方式发送
	for(i=0; i<g_Dcb[2].nDataSize; i++)
	{
		USART_SendData(g_Dcb[2].Usart, g_Dcb[2].pSendBuff[i]);
		while (USART_GetFlagStatus(g_Dcb[2].Usart, USART_FLAG_TC) == RESET);
	}

#ifdef	_COMM3_485_
	SetLo(COMM3_485);
#endif


	g_Dcb[2].bSendEnd = TRUE;
#endif	//USE_DMA_MODE
}
#endif	//_USE_COMM3_


/****************************************************************************
* 名	称：ReadComm1Data()
* 功	能：读串口数据
* 入口参数：pData：数据转存的地址；nLen：要读取数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/

//串口1
#ifdef _USE_COMM1_
void ClearComm1(void)
{
	if(m_pComm1Event != NULL)
	{
		m_pComm1Event->OSEventCnt = 0;
	}

	g_Dcb[0].bReceiveEnd = FALSE;
	g_Dcb[0].nReceiveIndex = 0;
}

int16 RecvComm1(uint8* pData, int16 nLen)
{
	uint16 nRealRead;

#ifdef	_COMM1_485_ 
	SetLo(COMM1_485);
#endif

	if(g_Dcb[0].bReceiveEnd == FALSE)
	{
		OSSemPend(m_pComm1Event, g_Dcb[0].nRecvTimeout, &g_Dcb[0].nCommErr);

		if(g_Dcb[0].nCommErr != 0)
		{
			//接收超时
			return -1;
		}
	}
	
	if(nLen <= 0 || nLen>g_Dcb[0].nReceiveIndex)
	{
		nRealRead = g_Dcb[0].nReceiveIndex;
	}
	else
	{
		nRealRead = nLen;
	}

	memcpy(pData, g_Dcb[0].pRecvBuff, nRealRead);
	g_Dcb[0].nReceiveIndex = 0;
	g_Dcb[0].bReceiveEnd = FALSE;	
	return nRealRead;
}
#endif

//串口2
#ifdef _USE_COMM2_
void ClearComm2(void)
{
	if(m_pComm2Event != NULL)
	{
		m_pComm2Event->OSEventCnt = 0;
	}

	g_Dcb[1].bReceiveEnd = FALSE;
	g_Dcb[1].nReceiveIndex = 0;
}


int16 RecvComm2(uint8* pData, int16 nLen)
{
	uint16 nRealRead;

#ifdef	_COMM2_485_
	SetLo(COMM2_485);
#endif

	if(g_Dcb[1].bReceiveEnd == FALSE)
	{
		//等待接收完成		
		OSSemPend(m_pComm2Event, g_Dcb[1].nRecvTimeout, &g_Dcb[1].nCommErr);

		if(g_Dcb[1].nCommErr != 0)
		{
			//接收超时
			return -1;
		}
	}



	if(nLen > g_Dcb[1].nReceiveIndex)
	{
		nRealRead = g_Dcb[1].nReceiveIndex;
	}
	else
	{
		nRealRead = nLen;
	}

	memcpy(pData, g_Dcb[1].pRecvBuff, nRealRead);	 
	g_Dcb[1].nReceiveIndex = 0;
	g_Dcb[1].bReceiveEnd = FALSE;	
	
	return nRealRead;
}
#endif

//串口3
#ifdef _USE_COMM3_
void ClearComm3(void)
{
	if(m_pComm3Event != NULL)
	{
		m_pComm3Event->OSEventCnt = 0;
	}

	g_Dcb[2].bReceiveEnd = FALSE;
	g_Dcb[2].nReceiveIndex = 0;
}

int16 RecvComm3(uint8* pData, int16 nLen)
{
	uint16 nRealRead;

#ifdef	_COMM3_485_ 
	SetLo(COMM3_485);
#endif

	if(g_Dcb[2].bReceiveEnd == FALSE)
	{
		OSSemPend(m_pComm3Event, g_Dcb[2].nRecvTimeout, &g_Dcb[2].nCommErr);

		if(g_Dcb[2].nCommErr != 0)
		{
			//接收超时
			return -1;
		}
	}

	if(nLen > g_Dcb[2].nReceiveIndex)
	{
		nRealRead = g_Dcb[2].nReceiveIndex;
	}
	else
	{
		nRealRead = nLen;
	}

	memcpy(pData, g_Dcb[2].pRecvBuff, nRealRead);	  
	g_Dcb[2].nReceiveIndex = 0;
	g_Dcb[2].bReceiveEnd = FALSE;	
	
	return nRealRead;
}
#endif


/****************************************************************************
* 名	称：IsComm1SendEnd()
* 功	能：判断串口发送完毕标志
* 入口参数：无
* 出口参数：0：		未发送完毕
			1：		发送完毕标志
* 说	明：无
****************************************************************************/
#ifdef	_IS_SEND_END1_
BOOL IsComm1SendEnd(void)
{
	return g_Dcb[0].bSendEnd;
}
#endif


#ifdef	_IS_SEND_END2_
BOOL IsComm2SendEnd(void)
{
	return g_Dcb[1].bSendEnd;
}
#endif


#ifdef	_IS_SEND_END3_
BOOL IsComm3SendEnd(void)
{
	return g_Dcb[2].bSendEnd;
}
#endif

/****************************************************************************
* 名	称：IsComm1ReceiveEnd()
* 功	能：判断串口接收完毕标志
* 入口参数：无
* 出口参数：0：		未接收完毕
			1：		接收完毕标志
* 说	明：无
****************************************************************************/
#ifdef	_IS_RECEIVE_END1_
uint16 IsComm1ReceiveEnd(void)
{
	if(g_Dcb[0].bReceiveEnd)
	{
		return g_Dcb[0].nReceiveIndex;	
	}
	return 0;
}
#endif

#ifdef	_IS_RECEIVE_END2_
uint16 IsComm2ReceiveEnd(void)
{
	if(g_Dcb[1].bReceiveEnd)
	{
		return g_Dcb[1].nReceiveIndex;	
	}
	return 0;
}
#endif

#ifdef	_IS_RECEIVE_END3_
uint16 IsComm3ReceiveEnd(void)
{
	if(g_Dcb[2].bReceiveEnd)
	{
		return g_Dcb[2].nReceiveIndex;	
	}
	return 0;
}
#endif

/****************************************************************************
* 名	称：TIM2_IRQHandler
* 功	能：定时器2中断
* 入口参数：无
* 出口参数：无
* 说	明：定时器2中断服务函数，表示接收数据完毕，进行数据处理
****************************************************************************/

//串口1接收定时器中断
#ifdef _USE_COMM1_
void TIM2_IRQHandler (void) 
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//进入中断
	OSIntEnter(); 
	
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)	
	{
		//-------收到一个字节后，延时一段时间仍没收到，认为是数据收完-----------
		OS_ENTER_CRITICAL();
		TIM_Cmd(TIM2, DISABLE);		//禁止定时器
		TIM_SetCounter(TIM2, 0);	//清零定时器
		TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);	 //禁止中断
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //清除中断标志	      
		OS_EXIT_CRITICAL();			

			
		g_Dcb[0].bReceiving = 0;
		g_Dcb[0].bReceiveEnd = 1;			//置接收完标志
		OSSemPost(m_pComm1Event);			//发送信号
		
	}

	//退出中断
	OSIntExit(); 
}
#endif

//串口2接收定时器中断
#ifdef _USE_COMM2_
void TIM3_IRQHandler (void) 
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//进入中断
	OSIntEnter(); 

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)	
	{
		//-------收到一个字节后，延时一段时间仍没收到，认为是数据收完-----------  
		OS_ENTER_CRITICAL();
		TIM_Cmd(TIM3, DISABLE);							//禁止定时器
		TIM_SetCounter(TIM3, 0);						//清零定时器	 
		TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);	 	//禁止中断
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	 	//清除中断标志	 
		OS_EXIT_CRITICAL();			

		
		g_Dcb[1].bReceiving = 0;	
		g_Dcb[1].bReceiveEnd = 1;						//置接收完标志
		OSSemPost(m_pComm2Event);						//发送信号
		 
	}

	//退出中断
	OSIntExit(); 
}
#endif

//串口3接收定时器中断
#ifdef _USE_COMM3_
void TIM4_IRQHandler (void) 
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//进入中断
	OSIntEnter();

	if(TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)	
	{
		//-------收到一个字节后，延时一段时间仍没收到，认为是数据收完-----------
		OS_ENTER_CRITICAL();
		TIM_Cmd(TIM4, DISABLE);							//禁止定时器
		TIM_SetCounter(TIM4, 0);						//清零定时器
		TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);	 	//禁止中断
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	 	//清除中断标志	      
		OS_EXIT_CRITICAL();			

		 	
		g_Dcb[2].bReceiving = 0;
		g_Dcb[2].bReceiveEnd = 1;			//置接收完标志			
		OSSemPost(m_pComm3Event);			//发送信号
		
	}

	//退出中断
	OSIntExit(); 
}
#endif

/****************************************************************************
* 名	称：DMA1_Channel4_IRQHandler		DMA1_Channel4_IRQn
* 功	能：DMA发送中断
* 入口参数：无
* 出口参数：无
* 说	明：DMA发送中断服务函数，表示发送数据完毕
****************************************************************************/

//串口1 DMA
#ifdef _USART_DMA_TX__
#ifdef _USE_COMM1_
void DMA1_Channel4_IRQHandler(void)
{
	//进入中断
	OSIntEnter();	
	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC4)) //Modify
	{
		//设置状态
		g_Dcb[0].bSendEnd = 1;			
		USART_DMACmd(g_Dcb[0].Usart, USART_DMAReq_Tx, DISABLE);
		DMA_Cmd(g_Dcb[0].Tx_DMA_Channel, DISABLE);         

		//发送信号量
		OSSemPost(m_pSendSyncEvent1);
	}

	//清除DMA状态
	DMA_ClearFlag(DMA1_FLAG_GL4| DMA1_FLAG_TC4 | DMA1_FLAG_HT4 | DMA1_FLAG_TE4);

	//退出中断
	OSIntExit();
}
#endif //_USE_COMM1_
#endif //_USART_DMA_RX__


//串口2 DMA
#ifdef _USART_DMA_TX__
#ifdef _USE_COMM2_
//#ifndef _COMM2_485_
void DMA1_Channel7_IRQHandler(void)
{
	//进入中断
	OSIntEnter();	  
	
	//DMA处理完成
	if(DMA_GetFlagStatus(DMA1_FLAG_TC7))
	{
		//设置状态信息
		g_Dcb[1].bSendEnd = 1;										//置发送完标志
		USART_DMACmd(g_Dcb[1].Usart, USART_DMAReq_Tx, DISABLE); 	//除能DMA发送
		DMA_Cmd(g_Dcb[1].Tx_DMA_Channel, DISABLE);					//除能DMA通道  

		//发送信号量
		OSSemPost(m_pSendSyncEvent2);
	}

	//清除DMA标志
	DMA_ClearFlag(DMA1_FLAG_GL7| DMA1_FLAG_TC7 | DMA1_FLAG_HT7 | DMA1_FLAG_TE7);  //Modify

	//退出中断
	OSIntExit();
}
//#endif
#endif //_USE_COMM3_
#endif //_USART_DMA_RX__


//串口1 接收中断
#ifdef _USE_COMM1_
void USART1_IRQHandler(void)
{
	//进入中断
	OSIntEnter();

	if(USART_GetITStatus(g_Dcb[0].Usart, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(g_Dcb[0].Usart,USART_IT_RXNE);

		//清零定时器
		TIM_SetCounter(TIM2, 0);	

		//启动超时定时器
		if(g_Dcb[0].bReceiving == 0)
		{
			g_Dcb[0].bReceiving = 1;
						
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //清除中断标志	  
			TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	 //允许产生更新中断    
			TIM_Cmd(TIM2, ENABLE);					 	 //允许定时器
		}
		
		//缓存数据
		if(g_Dcb[0].nReceiveIndex < COMM1_RECEIVE_MAX_LEN)
		{
			g_Dcb[0].pRecvBuff[g_Dcb[0].nReceiveIndex++] = USART_ReceiveData(g_Dcb[0].Usart);
		}		
	}

	//退出中断
	OSIntExit();
}
#endif //_USE_COMM1_


//串口2 接收中断
#ifdef _USE_COMM2_
void USART2_IRQHandler(void)
{
	//进入中断
	OSIntEnter();

	if(USART_GetITStatus(g_Dcb[1].Usart, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(g_Dcb[1].Usart,USART_IT_RXNE);

		//清零定时器
		TIM_SetCounter(TIM3, 0);

		//启动超时定时器
		if(g_Dcb[1].bReceiving == 0)
		{
			g_Dcb[1].bReceiving = 1;
						
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	 //清除中断标志	  
			TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);	 //允许产生更新中断    
			TIM_Cmd(TIM3, ENABLE);					 	 //允许定时器
		}
			
		//缓存数据	
		if(g_Dcb[1].nReceiveIndex < COMM1_RECEIVE_MAX_LEN)
		{
			g_Dcb[1].pRecvBuff[g_Dcb[1].nReceiveIndex++] = USART_ReceiveData(g_Dcb[1].Usart);
		}		
	} 

	//退出中断
	OSIntExit(); 
}
#endif //_USE_COMM2_


//串口3 接收中断
#ifdef _USE_COMM3_
void USART3_IRQHandler(void)
{
	//进入中断
	OSIntEnter();


	if(USART_GetITStatus(g_Dcb[2].Usart, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(g_Dcb[2].Usart,USART_IT_RXNE);

		//清零定时器
		TIM_SetCounter(TIM4, 0);	

		//启动超时定时器
		if(g_Dcb[2].bReceiving == 0)
		{
			g_Dcb[2].bReceiving = 1;
						
			TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	 //清除中断标志	  
			TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);	 //允许产生更新中断    
			TIM_Cmd(TIM4, ENABLE);					 	 //允许定时器
		}
	
		//缓存数据
		if(g_Dcb[2].nReceiveIndex < COMM1_RECEIVE_MAX_LEN)
		{
			g_Dcb[2].pRecvBuff[g_Dcb[2].nReceiveIndex++] = USART_ReceiveData(g_Dcb[2].Usart);
		}		
	}

	//退出中断
	OSIntExit();
}
#endif //_USE_COMM3_




