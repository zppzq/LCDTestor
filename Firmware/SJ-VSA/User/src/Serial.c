/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Seri1.c
**��   ��   ��: ����ǿ
**�� �� ��  ��: 2007��3��15��

**��		��: ��п�(kady1984@163.com)
**����޸�����: 2007��10��9��

**��        ��: ����uCosII���첽�����շ� 
********************************************************************************************************/
#define _SERIAL_C_

//��Ҫ��ͷ�ļ�---------------------------------------------------------------
#include "includes.h"
#include <ucos_ii.h>

//�źŶ���-------------------------------------------------------------------
#if	_COMM1_485_ > 0
#define SendEn485_PORT	GPIOA
#define SendEn485		GPIO_Pin_1
#endif

//data define----------------------------------------------------------------
static uint8  	m_pReceiveBuff[COMM1_RECEIVE_MAX_LEN];			//���ջ�����
uint8  			m_pSendBuff[COMM1_SEND_MAX_LEN];				//���ͻ�����
static uint16		m_nReceiveIndex;							//��������
#ifndef _USART_DMA_TX__
static uint16		m_nSendIndex;								//��������
#endif
static uint16		m_nSendSize;								//���ͳ���
//static uint32  		m_TimeCount;							//��ʱ������
//static uint32		m_nTempTimer;
static BOOL 		m_bReceiveEnd ;								//�������־
static BOOL 		m_bSendEnd;									//�������־
static BOOL			m_bIsReceivedData;							//�Ƿ��յ�����
static DCB 			g_Dcb;										//�豸���ƿ�			
static OS_EVENT 	*m_pCommEvent, *m_pSendSyncEvent;	    	//�¼����ƿ�
static uint8 		m_nCommErr;									//�����־


#if	_COMM1_485_ > 0
void MakePushPull(uint16_t Bit)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* ���˿�����Ϊ�������ģʽ */
	GPIO_InitStructure.GPIO_Pin = Bit;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SendEn485_PORT, &GPIO_InitStructure);
}

void MakeOpenDrain(uint16_t Bit)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* ���˿�����Ϊ©����·���ģʽ */
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
* ��	�ƣ�Comm1PortInit()
* ��	�ܣ����ڶ˿ڳ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
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

	/* ��USART Tx��GPIO����Ϊ���츴��ģʽ */
	GPIO_InitStructure.GPIO_Pin = USART_485_SLAVE_TxPin;		//UART4 TxPin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART_485_SLAVE_GPIO, &GPIO_InitStructure);

	/* ��USART Rx��GPIO����Ϊ��������ģʽ */
	GPIO_InitStructure.GPIO_Pin = USART_485_SLAVE_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(USART_485_SLAVE_GPIO, &GPIO_InitStructure);
	
	/* ����UART4����
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

	/* ʹ�� USART�� ������� */
	USART_Cmd(USART_485_SLAVE, ENABLE);
	USART_ClearFlag(USART_485_HOST, USART_FLAG_TC);     /* �巢����Ǳ�־��Transmission Complete flag */

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
* ��	�ƣ�Comm1VariInit()
* ��	�ܣ�����ȫ�ֱ�����ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _USE_COMM1_
void Comm1VariInit(void)
{
	g_Dcb.nBaudRate = 9600;		//��ʼĬ��ֵ
//	g_Dcb.nBaudRate = 115200;		//��ʼĬ��ֵ
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
* ��	�ƣ�SetDCB1()
* ��	�ܣ����ô��ڲ���
* ��ڲ�����pDcb���Ѷ���Ĵ��ڲ���
* ���ڲ�������
* ˵	�������ô��ڲ�����ע�⣺�ڿ�����ǰ����
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
* ��	�ƣ�SetComm1RecvTimeOut()
* ��	�ܣ����賬ʱʱ��
* ��ڲ���������ĳ�ʱʱ�䣬��λ��ms
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�OpenComm1()
* ��	�ܣ��򿪴���
* ��ڲ�������
* ���ڲ�������
* ˵	�������ô��ڲ���,�ڵ��ô˺���֮ǰ���ȳ�ʼ��DCB,��SetDCB()
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
	/* ����USART2����	(485����---�ϲ�˿�)
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

	/* ʹ�� USART�� ������� */
	USART_Cmd(USART_485_HOST, ENABLE);


	/* CPU��Сȱ�ݣ��������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
		�����������1���ֽ��޷���ȷ���ͳ�ȥ������ */
	USART_ClearFlag(USART_485_HOST, USART_FLAG_TC);     /* �巢����Ǳ�־��Transmission Complete flag */

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = (10000000 / g_Dcb.nBaudRate - 1)* 30;  // TIM_Period
	TIM_TimeBaseStructure.TIM_Prescaler = SYSCLK / 10000000;					   //TIM_Prescaler
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SetCounter(TIM2, 0);
//	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_ARRPreloadConfig(TIM2, ENABLE);	  //�Զ���װ��Ԥװ������
	NVIC->ISER[0] |= (1 << (TIM2_IRQn & 0x1F));	
	//���¶�TIM2���õĴ�������ʹTIM2����һ�θ����¼���Ŀ������TIM2
	//Ԥ��Ƶ���Ļ���������Ԥװ�ؼĴ�����ֵ(TIM2_PSC�Ĵ���������);
	//�Զ�װ��Ӱ�ӼĴ�������������Ԥװ�ؼĴ�����ֵ(TIM2_ARR)��
	//֮������ô������ΪTIM2��һ�ζ�ʱʱ�䲻׼ȷ,ͨ��������Ϊ�ǵ�һ
	//�μ���ʱӰ�ӼĴ�����ֵ��ȷ����ɵ�,�����ڴ����Ȳ���һ�θ���
	//�¼��Ը�����Щ���ܱ������ļĴ���.
/*	TIM2->CR1 = (1 << 2) |	(1 << 0);			//������������ж�
	TIM2->EGR = 1 << 0;							//���������¼�
	TIM2->CR1 &= ~(1 << 0);						//��ֹ������
	TIM2->CR1 &= ~(1 << 2);						//��ֹ�ж�
	TIM2->SR &= ~(1<<0);        				//����жϱ�־	*/	

#if	_COMM1_485_ > 0
	SetLo(SendEn485);		//����Ϊ����״̬
#endif

	//�����ź���
	if(m_pCommEvent == NULL) m_pCommEvent = OSSemCreate(0);
	if(m_pSendSyncEvent == NULL) m_pSendSyncEvent = OSSemCreate(0);

	OSTimeDly(OS_TICKS_PER_SEC/50);
	USART_ClearITPendingBit(USART_485_HOST, USART_FLAG_TC);	//����������жϱ�־
}
#endif

/****************************************************************************
* ��	�ƣ�CloseComm1()
* ��	�ܣ��رմ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	_CLOSE_COMM1_
void CloseComm1(void)
{
	USART_DeInit(USART_485_HOST);
	USART_Cmd(USART_485_HOST, DISABLE);
}
#endif


/****************************************************************************
* ��	�ƣ�SendComm1()
* ��	�ܣ���������
* ��ڲ�����pSendData�Ǵ����͵����ݣ�nDataSize�����ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SEND_COMM1_
void SendComm1(uint8 *pSendData,uint16 nDataSize)
{
	uint16 i;

	while(m_bSendEnd == FALSE);	//�ȴ��������

	//�����������ݷ��뷢�ͻ�����
	m_nSendSize = nDataSize;
	for(i=0; i<m_nSendSize; i++) m_pSendBuff[i] = pSendData[i];
	
#if	_COMM1_485_ > 0
	SetHi(SendEn485);			//����Ϊ����״̬
#endif
	
	//��������
	m_bSendEnd = 0;

	USART_485_HOST_Tx_DMA_Channel->CNDTR = m_nSendSize;					// �����ֽ�����
	USART_DMACmd(USART_485_HOST, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(USART_485_HOST_Tx_DMA_Channel, ENABLE);	//ʼ��DMAͨ��
//	USART_485_HOST_Tx_DMA_Channel->CCR  |= (1 << 0);					// ʼ��DMAͨ��                
	/* Enable DMA Channel5 complete transfer interrupt */ 
	/* Enable USART_485_HOST DMA Rx and TX request */

	if(g_Dcb.bSendSync)
	{
		OSSemPend(m_pSendSyncEvent, 0, &m_nCommErr);
	} 
}
#endif
		  
/****************************************************************************
* ��	�ƣ�ReadComm1Data()
* ��	�ܣ�����������
* ��ڲ�����pData������ת��ĵ�ַ��nLen��Ҫ��ȡ���ݵĳ���
* ���ڲ�������
* ˵	������
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
		//��δ��ȡ��������Ƶ�������ǰ��
		for(i=0;i<m_nReceiveIndex;i++)
		{
			m_pReceiveBuff[i] = m_pReceiveBuff[i+nRealRead];
		}
	}
	else
	{
		//�����Ѷ�ȡ��ϣ��������������
		m_bReceiveEnd = 0;	//ָʾ������������ȫ������		
	}*/
	return nRealRead;
}
#endif


/****************************************************************************
* ��	�ƣ�IsComm1SendEnd()
* ��	�ܣ��жϴ��ڷ�����ϱ�־
* ��ڲ�������
* ���ڲ�����0��		δ�������
			1��		������ϱ�־
* ˵	������
****************************************************************************/
#ifdef	_IS_SEND_END1_
BOOL IsComm1SendEnd(void)
{
	return m_bSendEnd;
}
#endif


/****************************************************************************
* ��	�ƣ�IsComm1ReceiveEnd()
* ��	�ܣ��жϴ��ڽ�����ϱ�־
* ��ڲ�������
* ���ڲ�����0��		δ�������
			1��		������ϱ�־
* ˵	������
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
* ��	�ƣ�ClearComm1()
* ��	�ܣ��������
* ��ڲ�������
* ���ڲ�������
* ˵	���������ڻ�ԭΪ��ʼ״̬
****************************************************************************/
#ifdef _CLEAR_COMM1_
void ClearComm1(void)
{
	m_nReceiveIndex = 0;			//������������
	m_nSendIndex = 0;				//������������
	m_nSendSize = 0;				//���ͳ�������
	m_TimeCount = 0;				//���ճ�ʱ����������
	m_bReceiveEnd = 0;				//�������־����
	m_bSendEnd = 1;					//�������־��һ
	USART_Cmd(USART_485_HOST, DISABLE);		// USART ����


#if	_COMM1_485_ > 0
	SetLo(SendEn485);		//����Ϊ����״̬
#endif
}
#endif


/****************************************************************************
* ��	�ƣ�TIM2_IRQHandler
* ��	�ܣ���ʱ��2�ж�
* ��ڲ�������
* ���ڲ�������
* ˵	������ʱ��2�жϷ���������ʾ����������ϣ��������ݴ���
****************************************************************************/
#ifdef _USE_COMM1_
void TIM2_IRQHandler (void) 
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)	
	{
		//-------�յ�һ���ֽں���ʱһ��ʱ����û�յ�����Ϊ����������-----------
		OS_ENTER_CRITICAL();
		TIM_Cmd(TIM2, DISABLE);		//��ֹ������
		TIM_SetCounter(TIM2, 0);	//���������
		TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);	 //��ֹ�ж�
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //����жϱ�־	      
		OS_EXIT_CRITICAL();			

		OSIntEnter(); 	
		m_bReceiveEnd = 1;			//�ý������־
		m_bIsReceivedData = 0;  	//����յ����ݱ�־			
		OSSemPost(m_pCommEvent);	//�����ź�
		OSIntExit(); 
	}
}
#endif

/****************************************************************************
* ��	�ƣ�DMA1_Channel7_IRQHandler		DMA1_Channel7_IRQn
* ��	�ܣ�DMA�����ж�
* ��ڲ�������
* ���ڲ�������
* ˵	����DMA�����жϷ���������ʾ�����������
****************************************************************************/
#ifdef _USART_DMA_TX__
void DMA1_Channel7_IRQHandler(void)
{
	OSIntEnter();	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC7)) //Modify
	{
		m_bSendEnd = 1;					//�÷������־
		USART_DMACmd(USART_485_HOST, USART_DMAReq_Tx, DISABLE); //����DMA����
		DMA_Cmd(USART_485_HOST_Tx_DMA_Channel, DISABLE);	//����DMAͨ��                

#if	_COMM1_485_ > 0
		SetLo(SendEn485);					//������ϣ�����Ϊ����״̬
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
			TIM_Cmd(TIM2, DISABLE);		//��ֹ������
			TIM_SetCounter(TIM2, 0);	//���������
			TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);	 //��ֹ�ж�
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //����жϱ�־	      
	
			m_bIsReceivedData = 1;
			TIM_Cmd(TIM2, ENABLE);					 	 //���������
			TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	 //������������ж�
		}
	
	
		if(m_nReceiveIndex < COMM1_RECEIVE_MAX_LEN)
		{
			TIM_SetCounter(TIM2, 0);	//���������
			/* Read one byte from the receive data register */
			m_pReceiveBuff[m_nReceiveIndex++] = USART_ReceiveData(USART_485_HOST);
		}		
	}
}
