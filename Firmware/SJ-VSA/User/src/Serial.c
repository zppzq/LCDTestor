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
#include <ucos_ii.h>

//信号定义-------------------------------------------------------------------
#if	_COMM1_485_ > 0
#define SendEn485_PORT	GPIOA
#define SendEn485		GPIO_Pin_1
#endif

//data define----------------------------------------------------------------
static uint8  	m_pReceiveBuff[COMM1_RECEIVE_MAX_LEN];			//接收缓冲区
uint8  			m_pSendBuff[COMM1_SEND_MAX_LEN];				//发送缓冲区
static uint16		m_nReceiveIndex;							//接收索引
#ifndef _USART_DMA_TX__
static uint16		m_nSendIndex;								//发送索引
#endif
static uint16		m_nSendSize;								//发送长度
//static uint32  		m_TimeCount;							//超时计数器
//static uint32		m_nTempTimer;
static BOOL 		m_bReceiveEnd ;								//接收完标志
static BOOL 		m_bSendEnd;									//发送完标志
static BOOL			m_bIsReceivedData;							//是否收到数据
static DCB 			g_Dcb;										//设备控制块			
static OS_EVENT 	*m_pCommEvent, *m_pSendSyncEvent;	    	//事件控制块
static uint8 		m_nCommErr;									//错误标志


#if	_COMM1_485_ > 0
void MakePushPull(uint16_t Bit)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* 将端口配置为推挽输出模式 */
	GPIO_InitStructure.GPIO_Pin = Bit;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SendEn485_PORT, &GPIO_InitStructure);
}

void MakeOpenDrain(uint16_t Bit)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* 将端口配置为漏级开路输出模式 */
	GPIO_InitStructure.GPIO_Pin = Bit;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SendEn485_PORT, &GPIO_InitStructure);
}

void SetLo(uint16_t Bit)
{
	GPIO_SetBits(SendEn485_PORT, Bit);	
}

void SetHi(uint16_t Bit)
{
	GPIO_ResetBits(SendEn485_PORT, Bit);	
}
#endif
/*----------------------------------------------------------------------------
USART
*----------------------------------------------------------------------------*/
/****************************************************************************
* 名	称：Comm1PortInit()
* 功	能：串口端口初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _USE_COMM1_
void UART_Slave_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure; 
	DMA_InitTypeDef DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable UART4 and GPIO clocks */
	RCC_APB2PeriphClockCmd(USART_485_SLAVE_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(USART_485_SLAVE_CLK, ENABLE);

	/* 将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = USART_485_SLAVE_TxPin;		//UART4 TxPin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART_485_SLAVE_GPIO, &GPIO_InitStructure);

	/* 将USART Rx的GPIO配置为浮空输入模式 */
	GPIO_InitStructure.GPIO_Pin = USART_485_SLAVE_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(USART_485_SLAVE_GPIO, &GPIO_InitStructure);
	
	/* 配置UART4参数
	    - BaudRate = 115200 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - No parity
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART_485_SLAVE, &USART_InitStructure);

	/* 使能 USART， 配置完毕 */
	USART_Cmd(USART_485_SLAVE, ENABLE);
	USART_ClearFlag(USART_485_HOST, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */

#if	_COMM1_485_ > 0
	MakePushPull(SendEn485);
#endif
}
#endif

void UART_Slave_Send(u8 *pData, u16 nSize)
{
	int i;
	for(i=0; i<nSize; i++)
	{
		USART_SendData(USART_485_SLAVE, pData[i]);
		while (USART_GetFlagStatus(USART_485_SLAVE, USART_FLAG_TC) == RESET);
	}
}


/****************************************************************************
* 名	称：Comm1VariInit()
* 功	能：串口全局变量初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _USE_COMM1_
void Comm1VariInit(void)
{
	g_Dcb.nBaudRate = 9600;		//初始默认值
//	g_Dcb.nBaudRate = 115200;		//初始默认值
	g_Dcb.nVerify = USART_Parity_No;
	g_Dcb.bSendSync = 1;

	m_nReceiveIndex = 0;
#ifndef _USART_DMA_TX__
	m_nSendIndex = 0;
#endif
	m_nSendSize = 0;
	m_bReceiveEnd = 0;
	m_bSendEnd = 1;
	m_pCommEvent = NULL;
	g_Dcb.nRecvTimeout = 0;
	m_bIsReceivedData = 0;
}
#endif


/****************************************************************************
* 名	称：SetDCB1()
* 功	能：设置串口参数
* 入口参数：pDcb是已定义的串口参数
* 出口参数：无
* 说	明：设置串口参数，注意：在开串口前调用
****************************************************************************/
#ifdef _SET_DCB1_
void SetDCB1(DCB *pDcb)
{
	g_Dcb = *pDcb;
}
#endif

void SetSyncMode(uint8 bSync)
{
	g_Dcb.bSendSync = bSync;
}


/****************************************************************************
* 名	称：SetComm1RecvTimeOut()
* 功	能：重设超时时间
* 入口参数：重设的超时时间，单位：ms
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_SET_COMM1_RECV_TIME_OUT_
void SetComm1RecvTimeOut(uint16 nTimeOut) 
{
	if(nTimeOut == 0)
	{
		g_Dcb.nRecvTimeout = 0;
		return;
	}
	g_Dcb.nRecvTimeout = (nTimeOut - 1) / 10 + 1;
}
#endif

/****************************************************************************
* 名	称：OpenComm1()
* 功	能：打开串口
* 入口参数：无
* 出口参数：无
* 说	明：设置串口参数,在调用此函数之前请先初始化DCB,即SetDCB()
****************************************************************************/
#ifdef _USE_COMM1_
void OpenComm1(void)
{
	USART_InitTypeDef USART_InitStructure; 
	DMA_InitTypeDef DMA_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	/* USART2 TX DMA1 Channel (triggered by USART2 Tx event) Config */
	DMA_DeInit(USART_485_HOST_Tx_DMA_Channel);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = USART_485_HOST_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)m_pSendBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(USART_485_HOST_Tx_DMA_Channel, &DMA_InitStructure);
	DMA_ITConfig(USART_485_HOST_Tx_DMA_Channel, DMA_IT_TC, ENABLE);    
	/* 配置USART2参数	(485总线---上层端口)
	    - BaudRate = 115200 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - No parity
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate = g_Dcb.nBaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = g_Dcb.nVerify;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART_485_HOST, &USART_InitStructure);

  /* Enable the USARTz Receive Interrupt */
  USART_ITConfig(USART_485_HOST, USART_IT_RXNE, ENABLE);

	/* 使能 USART， 配置完毕 */
	USART_Cmd(USART_485_HOST, ENABLE);


	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
		如下语句解决第1个字节无法正确发送出去的问题 */
	USART_ClearFlag(USART_485_HOST, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = (10000000 / g_Dcb.nBaudRate - 1)* 30;  // TIM_Period
	TIM_TimeBaseStructure.TIM_Prescaler = SYSCLK / 10000000;					   //TIM_Prescaler
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SetCounter(TIM2, 0);
//	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_ARRPreloadConfig(TIM2, ENABLE);	  //自动重装载预装载允许
	NVIC->ISER[0] |= (1 << (TIM2_IRQn & 0x1F));	
	//以下对TIM2设置的代码用于使TIM2产生一次更新事件，目的是让TIM2
	//预分频器的缓冲区置入预装载寄存器的值(TIM2_PSC寄存器的内容);
	//自动装载影子寄存器被重新置入预装载寄存器的值(TIM2_ARR)。
	//之所以这么做是因为TIM2第一次定时时间不准确,通过分析认为是第一
	//次记数时影子寄存器的值不确定造成的,所以在此事先产生一次更新
	//事件以更新那些不能被操作的寄存器.
/*	TIM2->CR1 = (1 << 2) |	(1 << 0);			//允许计数器和中断
	TIM2->EGR = 1 << 0;							//产生更新事件
	TIM2->CR1 &= ~(1 << 0);						//禁止计数器
	TIM2->CR1 &= ~(1 << 2);						//禁止中断
	TIM2->SR &= ~(1<<0);        				//清除中断标志	*/	

#if	_COMM1_485_ > 0
	SetLo(SendEn485);		//设置为接收状态
#endif

	//创建信号量
	if(m_pCommEvent == NULL) m_pCommEvent = OSSemCreate(0);
	if(m_pSendSyncEvent == NULL) m_pSendSyncEvent = OSSemCreate(0);

	OSTimeDly(OS_TICKS_PER_SEC/50);
	USART_ClearITPendingBit(USART_485_HOST, USART_FLAG_TC);	//清除发送完中断标志
}
#endif

/****************************************************************************
* 名	称：CloseComm1()
* 功	能：关闭串口
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_CLOSE_COMM1_
void CloseComm1(void)
{
	USART_DeInit(USART_485_HOST);
	USART_Cmd(USART_485_HOST, DISABLE);
}
#endif


/****************************************************************************
* 名	称：SendComm1()
* 功	能：发送数据
* 入口参数：pSendData是待发送的数据，nDataSize是数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SEND_COMM1_
void SendComm1(uint8 *pSendData,uint16 nDataSize)
{
	uint16 i;

	while(m_bSendEnd == FALSE);	//等待发送完毕

	//将待发送数据放入发送缓冲区
	m_nSendSize = nDataSize;
	for(i=0; i<m_nSendSize; i++) m_pSendBuff[i] = pSendData[i];
	
#if	_COMM1_485_ > 0
	SetHi(SendEn485);			//设置为发送状态
#endif
	
	//发送数据
	m_bSendEnd = 0;

	USART_485_HOST_Tx_DMA_Channel->CNDTR = m_nSendSize;					// 发送字节数量
	USART_DMACmd(USART_485_HOST, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(USART_485_HOST_Tx_DMA_Channel, ENABLE);	//始能DMA通道
//	USART_485_HOST_Tx_DMA_Channel->CCR  |= (1 << 0);					// 始能DMA通道                
	/* Enable DMA Channel5 complete transfer interrupt */ 
	/* Enable USART_485_HOST DMA Rx and TX request */

	if(g_Dcb.bSendSync)
	{
		OSSemPend(m_pSendSyncEvent, 0, &m_nCommErr);
	} 
}
#endif
		  
/****************************************************************************
* 名	称：ReadComm1Data()
* 功	能：读串口数据
* 入口参数：pData：数据转存的地址；nLen：要读取数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _RECEIVE_COMM1_
uint16 ReadComm1Data(uint8* pData, uint16 nLen)
{
	uint16 nRealRead, i;

	if(m_bReceiveEnd == FALSE)
	{
		OSSemPend(m_pCommEvent, g_Dcb.nRecvTimeout, &m_nCommErr);	 //
	}
	if(nLen <= 0 || nLen>m_nReceiveIndex)
		nRealRead = m_nReceiveIndex;
	for(i=0;i<nRealRead;i++) pData[i] = m_pReceiveBuff[i];

	m_nReceiveIndex = 0;
	
/*	m_nReceiveIndex -= nRealRead;
	if(m_nReceiveIndex > 0)
	{
		//将未读取完的数据移到缓冲区前端
		for(i=0;i<m_nReceiveIndex;i++)
		{
			m_pReceiveBuff[i] = m_pReceiveBuff[i+nRealRead];
		}
	}
	else
	{
		//数据已读取完毕，允许接收新数据
		m_bReceiveEnd = 0;	//指示缓冲区数据已全部读出		
	}*/
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
	return m_bSendEnd;
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
	if(m_bReceiveEnd)
	{
		return m_nReceiveIndex;	
	}
	return 0;
}
#endif


/****************************************************************************
* 名	称：ClearComm1()
* 功	能：清除串口
* 入口参数：无
* 出口参数：无
* 说	明：将串口还原为初始状态
****************************************************************************/
#ifdef _CLEAR_COMM1_
void ClearComm1(void)
{
	m_nReceiveIndex = 0;			//接收索引清零
	m_nSendIndex = 0;				//发送索引清零
	m_nSendSize = 0;				//发送长度清零
	m_TimeCount = 0;				//接收超时计数器清零
	m_bReceiveEnd = 0;				//接收完标志清零
	m_bSendEnd = 1;					//发送完标志置一
	USART_Cmd(USART_485_HOST, DISABLE);		// USART 除能


#if	_COMM1_485_ > 0
	SetLo(SendEn485);		//设置为接收状态
#endif
}
#endif


/****************************************************************************
* 名	称：TIM2_IRQHandler
* 功	能：定时器2中断
* 入口参数：无
* 出口参数：无
* 说	明：定时器2中断服务函数，表示接收数据完毕，进行数据处理
****************************************************************************/
#ifdef _USE_COMM1_
void TIM2_IRQHandler (void) 
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)	
	{
		//-------收到一个字节后，延时一段时间仍没收到，认为是数据收完-----------
		OS_ENTER_CRITICAL();
		TIM_Cmd(TIM2, DISABLE);		//禁止计数器
		TIM_SetCounter(TIM2, 0);	//清零计数器
		TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);	 //禁止中断
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //清除中断标志	      
		OS_EXIT_CRITICAL();			

		OSIntEnter(); 	
		m_bReceiveEnd = 1;			//置接收完标志
		m_bIsReceivedData = 0;  	//清除收到数据标志			
		OSSemPost(m_pCommEvent);	//发送信号
		OSIntExit(); 
	}
}
#endif

/****************************************************************************
* 名	称：DMA1_Channel7_IRQHandler		DMA1_Channel7_IRQn
* 功	能：DMA发送中断
* 入口参数：无
* 出口参数：无
* 说	明：DMA发送中断服务函数，表示发送数据完毕
****************************************************************************/
#ifdef _USART_DMA_TX__
void DMA1_Channel7_IRQHandler(void)
{
	OSIntEnter();	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC7)) //Modify
	{
		m_bSendEnd = 1;					//置发送完标志
		USART_DMACmd(USART_485_HOST, USART_DMAReq_Tx, DISABLE); //除能DMA发送
		DMA_Cmd(USART_485_HOST_Tx_DMA_Channel, DISABLE);	//除能DMA通道                

#if	_COMM1_485_ > 0
		SetLo(SendEn485);					//发送完毕，设置为接收状态
#endif
		if(g_Dcb.bSendSync)
		{
			OSSemPost(m_pSendSyncEvent);
		}
	}
	DMA_ClearFlag(DMA1_FLAG_GL7| DMA1_FLAG_TC7 | DMA1_FLAG_HT7 | DMA1_FLAG_TE7);  //Modify

	OSIntExit();
}
#endif //_USART_DMA_RX__



/**
  * @brief  This function handles USART2 global interrupt request.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART_485_HOST, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART_485_HOST,USART_IT_RXNE);
		if(!m_bIsReceivedData)
		{
			TIM_Cmd(TIM2, DISABLE);		//禁止计数器
			TIM_SetCounter(TIM2, 0);	//清零计数器
			TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);	 //禁止中断
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //清除中断标志	      
	
			m_bIsReceivedData = 1;
			TIM_Cmd(TIM2, ENABLE);					 	 //允许计数器
			TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	 //允许产生更新中断
		}
	
	
		if(m_nReceiveIndex < COMM1_RECEIVE_MAX_LEN)
		{
			TIM_SetCounter(TIM2, 0);	//清零计数器
			/* Read one byte from the receive data register */
			m_pReceiveBuff[m_nReceiveIndex++] = USART_ReceiveData(USART_485_HOST);
		}		
	}
}
