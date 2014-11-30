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

//�źŶ���-------------------------------------------------------------------
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

//���ݶ���
static DCB 			g_Dcb[3];									//�豸���ƿ�			

//USART1
#ifdef _USE_COMM1_
static OS_EVENT 	*m_pComm1Event;		//ͨ��1�¼����ƿ�
static OS_EVENT 	*m_pSendSyncEvent1;	//ͨ��1����ͬ���¼�
#endif

//USART2
#ifdef _USE_COMM2_
static OS_EVENT 	*m_pComm2Event;	    //ͨ��2�¼����ƿ�
static OS_EVENT 	*m_pSendSyncEvent2;	//ͨ��2����ͬ���¼�
#endif

//USART3
#ifdef _USE_COMM3_
static OS_EVENT 	*m_pComm3Event;	    //ͨ��2�¼����ƿ�
static OS_EVENT 	*m_pSendSyncEvent3;	//ͨ��2����ͬ���¼�
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
* ��	�ƣ�Comm1VariInit()
* ��	�ܣ�����ȫ�ֱ�����ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
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

	g_Dcb[0].Timer = TIM2;						//ͨ��1���ն�ʱ��
	g_Dcb[0].TimerClk = RCC_APB1Periph_TIM2;	//ͨ��1��ʱ��ʱ��

	g_Dcb[0].nBaudRate = 9600;					//������							
	g_Dcb[0].nVerify = USART_Parity_No;	    	//У��λ,0ΪNONE��1ΪżУ��
	
	g_Dcb[0].pSendBuff = g_pSendBuff1;
	g_Dcb[0].pRecvBuff = g_pRecvBuff1;
	g_Dcb[0].nReceiveIndex = 0;				//��������
	g_Dcb[0].nSendIndex = 0;				//��������
	g_Dcb[0].bReceiving = 0;				//���ձ�־
	g_Dcb[0].bReceiveEnd =  0;				//�������־
	g_Dcb[0].bSendEnd = 1;					//�������־
	g_Dcb[0].bIsReceivedData = 0; 			//�Ƿ��յ�����
	g_Dcb[0].nCommErr = 0;					//�����־
	g_Dcb[0].nRecvTimeout = 0;				//��ʱ��ʱ��
	g_Dcb[0].bSendSync = 1;
	m_pComm1Event = NULL;		//ͨ��1�¼����ƿ�
	m_pSendSyncEvent1 = NULL;	//ͨ��1����ͬ���¼�
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

	g_Dcb[1].Timer = TIM3;						//ͨ��2���ն�ʱ��
	g_Dcb[1].TimerClk = RCC_APB1Periph_TIM3;	//ͨ��2��ʱ��ʱ��
	
	g_Dcb[1].nBaudRate = 9600;					//������							
	g_Dcb[1].nVerify = USART_Parity_No;	    	//У��λ,0ΪNONE��1ΪżУ��
	
	g_Dcb[1].pSendBuff = g_pSendBuff3;
	g_Dcb[1].pRecvBuff = g_pRecvBuff3;
	g_Dcb[1].nReceiveIndex = 0;				//��������
	g_Dcb[1].nSendIndex = 0;				//��������
	g_Dcb[1].bReceiving = 0;				//���ձ�־
	g_Dcb[1].bReceiveEnd =  0;				//�������־
	g_Dcb[1].bSendEnd = 1;					//�������־
	g_Dcb[1].nCommErr = 0;					//�����־
	g_Dcb[1].nRecvTimeout = 0;				//��ʱ��ʱ��
	g_Dcb[1].bSendSync = 1;
	m_pComm2Event = NULL;	    			//ͨ��3�¼����ƿ�
	m_pSendSyncEvent2 = NULL;				//ͨ��3����ͬ���¼�
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

	g_Dcb[2].Timer = TIM4;						//ͨ��3���ն�ʱ��
	g_Dcb[2].TimerClk = RCC_APB1Periph_TIM4;	//ͨ��3��ʱ��ʱ��

	g_Dcb[2].nBaudRate = 9600;					//������							
	g_Dcb[2].nVerify = USART_Parity_No;	    	//У��λ,0ΪNONE��1ΪżУ��
	
	g_Dcb[2].pSendBuff = g_pSendBuff2;
	g_Dcb[2].pRecvBuff = g_pRecvBuff2;
	g_Dcb[2].nReceiveIndex = 0;					//��������
	g_Dcb[2].nSendIndex = 0;					//��������
	g_Dcb[2].bReceiving = 0;					//���ձ�־
	g_Dcb[2].bReceiveEnd =  0;					//�������־
	g_Dcb[2].bSendEnd = 1;						//�������־
	g_Dcb[2].nCommErr = 0;						//�����־
	g_Dcb[2].nRecvTimeout = 100;				//��ʱ��ʱ��
	g_Dcb[2].bSendSync = 1;
	m_pComm3Event = NULL;	    				//ͨ��3�¼����ƿ�
	m_pSendSyncEvent3 = NULL;					//ͨ��3����ͬ���¼�
#endif

}

/****************************************************************************
* ��	�ƣ�SetCommBaudRate()
* ��	�ܣ����ò�����
* ��ڲ�����nPort���˿ڣ�nBaudRate��������
* ���ڲ�������
* ˵	������Open֮ǰ������Ѿ�Open�ˣ���Ҫ����Open
****************************************************************************/
void SetCommBaudRate(uint8 nPort, uint32 nBaudRate)
{
	//�ڲ�������
	nPort -= 1;
	if(nPort >= 3) return;

	//���ò�����
	g_Dcb[nPort].nBaudRate = nBaudRate;
}

/****************************************************************************
* ��	�ƣ�SetCommRecvTimeOut()
* ��	�ܣ����ý��ճ�ʱʱ��
* ��ڲ�����nPort���˿ڣ�nRecvTimeout����ʱʱ��(ms)
* ���ڲ�������
* ˵	������ʱ������
****************************************************************************/
void SetCommRecvTimeOut(uint8 nPort, uint16 nRecvTimeout)
{
	//�ڲ�������
	nPort -= 1;
	if(nPort >= 3) return;

	//��ʽ������1��9ʱ����10����0
	if((nRecvTimeout >= 1 ) && (nRecvTimeout <= 9 ))
	{
		nRecvTimeout = 10;
	}

	//���ý��ճ�ʱʱ��
	g_Dcb[nPort].nRecvTimeout = nRecvTimeout / 10;
}


/****************************************************************************
* ��	�ƣ�CommPortOpen()
* ��	�ܣ��򿪶˿�
* ��ڲ�����nPort���˿�
* ���ڲ�������
* ˵	������
****************************************************************************/
void CommPortOpen(uint8 nPort)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//�ڲ�������
	nPort -= 1;
	if(nPort >= 3) return;

	//���÷��Ͷ˿�����
	GPIO_InitStructure.GPIO_Pin = g_Dcb[nPort].TxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(g_Dcb[nPort].Port, &GPIO_InitStructure);

	//���ý��ն˿�����
	GPIO_InitStructure.GPIO_Pin = g_Dcb[nPort].RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(g_Dcb[nPort].Port, &GPIO_InitStructure);
}


/****************************************************************************
* ��	�ƣ�CommPortShut()
* ��	�ܣ��رն˿�
* ��ڲ�����nPort���˿�
* ���ڲ�������
* ˵	������
****************************************************************************/
void CommPortShut(uint8 nPort)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//�ڲ�������
	nPort -= 1;
	if(nPort >= 3) return;

	//���÷��Ͷ˿�����
	GPIO_InitStructure.GPIO_Pin = g_Dcb[nPort].TxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(g_Dcb[nPort].Port, &GPIO_InitStructure);

	//���ý��ն˿�����
	GPIO_InitStructure.GPIO_Pin = g_Dcb[nPort].RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(g_Dcb[nPort].Port, &GPIO_InitStructure);
}

/****************************************************************************
* ��	�ƣ�Comm1PortInit()
* ��	�ܣ����ڶ˿ڳ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void UART_Configuration(DCB* pDCB)
{
	USART_InitTypeDef USART_InitStructure; 

#ifdef USE_DMA_MODE
	DMA_InitTypeDef DMA_InitStructure;
#endif

    GPIO_InitTypeDef GPIO_InitStructure;
	
	//����ʱ��
	RCC_APB2PeriphClockCmd(pDCB->PortClk, ENABLE);
	
	if(pDCB->UsartClk == RCC_APB2Periph_USART1)
	{
		RCC_APB2PeriphClockCmd(pDCB->UsartClk, ENABLE);
	}
	else
	{
		RCC_APB1PeriphClockCmd(pDCB->UsartClk, ENABLE);
	}
	
	//����UART����
	USART_InitStructure.USART_BaudRate = pDCB->nBaudRate; 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(pDCB->Usart, &USART_InitStructure);

	//ʹ�ܴ��ڽ����ж�
	USART_ITConfig(pDCB->Usart, USART_IT_RXNE, ENABLE);
	
	//ʹ�ܴ���
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

	//���÷��Ͷ˿�����
	GPIO_InitStructure.GPIO_Pin = pDCB->TxPin;		//UART4 TxPin
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(pDCB->Port, &GPIO_InitStructure);

	//���ý��ն˿�����
	GPIO_InitStructure.GPIO_Pin = pDCB->RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(pDCB->Port, &GPIO_InitStructure);

	//���������ɱ�־
	USART_ClearFlag(pDCB->Usart, USART_FLAG_TC); 
}

void Timer_Config(DCB *pDCB) 
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(pDCB->TimerClk, ENABLE);

	//��ʱ��ʱ������
	TIM_TimeBaseStructure.TIM_Period = 30;  // TIM_Period
	TIM_TimeBaseStructure.TIM_Prescaler = SYSCLK / pDCB->nBaudRate - 1;	//TIM_Prescaler
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	
	TIM_TimeBaseInit(pDCB->Timer, &TIM_TimeBaseStructure);

	TIM_SetCounter(pDCB->Timer, 0);
	TIM_ARRPreloadConfig(pDCB->Timer, ENABLE);	  //�Զ���װ��Ԥװ������	
	
	//���¶�TIM2���õĴ�������ʹTIM2����һ�θ����¼���Ŀ������TIM2
	//Ԥ��Ƶ���Ļ���������Ԥװ�ؼĴ�����ֵ(TIM2_PSC�Ĵ���������);
	//�Զ�װ��Ӱ�ӼĴ�������������Ԥװ�ؼĴ�����ֵ(TIM2_ARR)��
	//֮������ô������ΪTIM2��һ�ζ�ʱʱ�䲻׼ȷ,ͨ��������Ϊ�ǵ�һ
	//�μ���ʱӰ�ӼĴ�����ֵ��ȷ����ɵ�,�����ڴ����Ȳ���һ�θ���
	//�¼��Ը�����Щ���ܱ������ļĴ���.
//	TIM_ITConfig(pDCB->Timer, TIM_IT_Update, ENABLE);	 //������������ж�	
	TIM_Cmd(pDCB->Timer, ENABLE);					 	 //����ʱ��	
	TIM_GenerateEvent(pDCB->Timer, TIM_EventSource_Update);//���������¼�
	TIM_Cmd(pDCB->Timer, DISABLE);					 	 //��ֹ��ʱ��	
	TIM_ITConfig(pDCB->Timer, TIM_IT_Update, DISABLE);	 //��ֹ�ж�	
	TIM_ClearITPendingBit(pDCB->Timer, TIM_IT_Update);		//����жϱ�־	      
	/*	TIM2->CR1 = (1 << 2) |	(1 << 0);			//����ʱ�����ж�
	TIM2->EGR = 1 << 0;							//���������¼�
	TIM2->CR1 &= ~(1 << 0);						//��ֹ��ʱ��
	TIM2->CR1 &= ~(1 << 2);						//��ֹ�ж�
	TIM2->SR &= ~(1<<0);        				//����жϱ�־	*/	
}

/****************************************************************************
* ��	�ƣ�OpenComm1()
* ��	�ܣ��򿪴���
* ��ڲ�������
* ���ڲ�������
* ˵	�������ô��ڲ���,�ڵ��ô˺���֮ǰ���ȳ�ʼ��DCB,��SetDCB()
****************************************************************************/
#ifdef _USE_COMM1_

//����1
void OpenComm1(void)
{
	//����
	UART_Configuration(&g_Dcb[0]);
	Timer_Config(&g_Dcb[0]);

	//����485����
#ifdef	_COMM1_485_
	SetLo(COMM1_485);		
#endif

	//�����ź���
	if(m_pComm1Event == NULL) m_pComm1Event = OSSemCreate(0);
	if(m_pSendSyncEvent1 == NULL) m_pSendSyncEvent1 = OSSemCreate(0);

	//��ʱ�ȴ���ʼ��״̬
	OSTimeDly(OS_TICKS_PER_SEC/50);

	//����������жϱ�־
	USART_ClearITPendingBit(g_Dcb[0].Usart, USART_FLAG_TC);	
}
#endif	//_USE_COMM1_


//����2
#ifdef _USE_COMM2_
void OpenComm2(void)
{
	//����
	UART_Configuration(&g_Dcb[1]);
	Timer_Config(&g_Dcb[1]);

	//����485����
#ifdef	_COMM2_485_
	SetLo(COMM2_485);		
#endif
	
	//�����ź���
	if(m_pComm2Event == NULL) m_pComm2Event = OSSemCreate(0);
	if(m_pSendSyncEvent2 == NULL) m_pSendSyncEvent2 = OSSemCreate(0);
	
	//��ʱ�ȴ���ʼ��״̬
	OSTimeDly(OS_TICKS_PER_SEC/50);
	
	//����������жϱ�־
	USART_ClearITPendingBit(g_Dcb[1].Usart, USART_FLAG_TC);
}
#endif	//_USE_COMM2_

//����3
#ifdef _USE_COMM3_
void OpenComm3(void)
{
	//����
	UART_Configuration(&g_Dcb[2]);
	Timer_Config(&g_Dcb[2]);

	//����485����
#ifdef	_COMM3_485_
	SetLo(COMM3_485);
#endif
	
	//�����ź���
	if(m_pComm3Event == NULL) m_pComm3Event = OSSemCreate(0);
	if(m_pSendSyncEvent3 == NULL) m_pSendSyncEvent3 = OSSemCreate(0);
	
	//��ʱ�ȴ���ʼ��״̬
	OSTimeDly(OS_TICKS_PER_SEC/50);

	//����������жϱ�־
	USART_ClearITPendingBit(g_Dcb[2].Usart, USART_FLAG_TC);	
}
#endif	//_USE_COMM3_

/****************************************************************************
* ��	�ƣ�CloseComm1()
* ��	�ܣ��رմ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/

//����1
#ifdef _USE_COMM1_
void CloseComm1(void)
{
	USART_DeInit(g_Dcb[0].Usart);
	USART_Cmd(g_Dcb[0].Usart, DISABLE);

#ifdef	_COMM1_485_
	SetLo(COMM1_485);		//����Ϊ����״̬
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


//����2
#ifdef _USE_COMM2_
void CloseComm2(void)
{
	USART_DeInit(g_Dcb[1].Usart);
	USART_Cmd(g_Dcb[1].Usart, DISABLE);

#ifdef	_COMM2_485_
	SetLo(COMM2_485);		//����Ϊ����״̬
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


//����3
#ifdef _USE_COMM3_
void CloseComm3(void)
{
	USART_DeInit(g_Dcb[2].Usart);
	USART_Cmd(g_Dcb[2].Usart, DISABLE);
#ifdef	_COMM3_485_
	SetLo(COMM3_485);		//����Ϊ����״̬
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
* ��	�ƣ�SendComm1()
* ��	�ܣ���������
* ��ڲ�����pSendData�Ǵ����͵����ݣ�nDataSize�����ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
//����1
#ifdef _USE_COMM1_
void SendComm1(uint8 *pSendData, int16 nDataSize)
{
	if(nDataSize == 0) return;

	//�ȴ��������
	while(g_Dcb[0].bSendEnd == FALSE);

	//�����������ݷ��뷢�ͻ�����
	g_Dcb[0].nDataSize = nDataSize;
	memcpy(g_Dcb[0].pSendBuff, pSendData, nDataSize);
	
#ifdef	_COMM1_485_
	SetHi(COMM1_485);			//����Ϊ����״̬
#endif
	
	//��������
	g_Dcb[0].bSendEnd = FALSE;

#ifdef USE_DMA_MODE		//DMA��ʽ����
	g_Dcb[0].Tx_DMA_Channel->CNDTR = g_Dcb[0].nDataSize;			// �����ֽ�����
	USART_DMACmd(g_Dcb[0].Usart, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(g_Dcb[0].Tx_DMA_Channel, ENABLE);						//ʼ��DMAͨ��

	//�ȴ��������
	OSSemPend(m_pSendSyncEvent1, 0, &g_Dcb[0].nCommErr);

	//��ʱ������ֽ����ݻ����ڴ��ͣ��ȴ������
	while (USART_GetFlagStatus(g_Dcb[0].Usart, USART_FLAG_TC) == RESET);

	//��RS485
#ifdef	_COMM1_485_	   	
	SetLo(COMM1_485);			
#endif

#else		//USART��ʽ����

	USART_ClearFlag(USART1, USART_FLAG_TC | USART_IT_TXE);     /* �巢����ɱ�־��Transmission Complete flag */
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



//����2		  
#ifdef _USE_COMM2_
void SendComm2(uint8 *pSendData, int16 nDataSize)
{
	if(nDataSize == 0) return;

	//�ȴ��������
	while(g_Dcb[1].bSendEnd == FALSE);

	//�����������ݷ��뷢�ͻ�����
	g_Dcb[1].nSendSize = nDataSize;	 
	memcpy(g_Dcb[1].pSendBuff, pSendData, nDataSize);

	//����485����
#ifdef _COMM2_485_	   	
	SetHi(COMM2_485);			
#endif
	
	//���÷���״̬
	g_Dcb[1].bSendEnd = FALSE;

	//DMA��ʽ����
#ifdef USE_DMA_MODE	
	
	//��DMA���Ͳ�����DMA�ж����485�����ߣ���DMA�������жϵ�ʱ�򴮿����ݲ�û�з���
	g_Dcb[1].Tx_DMA_Channel->CNDTR = g_Dcb[1].nSendSize;			// �����ֽ�����
	USART_DMACmd(g_Dcb[1].Usart, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(g_Dcb[1].Tx_DMA_Channel, ENABLE);						//ʼ��DMAͨ��

	//�ȴ���������ź���
	OSSemPend(m_pSendSyncEvent2, 0, &g_Dcb[1].nCommErr);  
	
	//��ʱ������ֽ����ݻ����ڴ��ͣ��ȴ������
	while (USART_GetFlagStatus(g_Dcb[1].Usart, USART_FLAG_TC) == RESET);

	//��RS485
#ifdef	_COMM2_485_	   	
	SetLo(COMM2_485);			
#endif

	
#else
	//ѭ������		
	USART_ClearFlag(g_Dcb[1].Usart, USART_FLAG_TC | USART_IT_TXE);
	for(i=0; i<g_Dcb[1].nDataSize; i++)
	{
		USART_SendData(g_Dcb[1].Usart, g_Dcb[1].pSendBuff[i]);
		while (USART_GetFlagStatus(g_Dcb[1].Usart, USART_FLAG_TC) == RESET);
	}

	g_Dcb[1].bSendEnd = TRUE;

	//��RS485
#ifdef	_COMM2_485_
	SetLo(COMM2_485);
#endif
	

#endif	//USE_DMA_MODE

}
#endif	//_USE_COMM2_


//����3
#ifdef _USE_COMM3_
void SendComm3(uint8 *pSendData, int16 nDataSize)
{
	if(nDataSize == 0) return;

	//�ȴ��������
	while(g_Dcb[2].bSendEnd == FALSE);

	//�����������ݷ��뷢�ͻ�����
	g_Dcb[2].nSendSize = nDataSize;
	memcpy(g_Dcb[2].pSendBuff, pSendData, nDataSize);

#ifdef	_COMM3_485_
	SetHi(COMM3_485);			//����Ϊ����״̬
#endif
	
	//��������
	g_Dcb[2].bSendEnd = FALSE;


#ifdef USE_DMA_MODE		//DMA��ʽ����
	
	g_Dcb[2].Tx_DMA_Channel->CNDTR = g_Dcb[2].nSendSize;			// �����ֽ�����
	USART_DMACmd(g_Dcb[2].Usart, USART_DMAReq_Tx, ENABLE);
	DMA_Cmd(g_Dcb[2].Tx_DMA_Channel, ENABLE);						//ʼ��DMAͨ��

	//�ȴ��������
	OSSemPend(m_pSendSyncEvent3, 0, &g_Dcb[2].nCommErr);

	//��ʱ������ֽ����ݻ����ڴ��ͣ��ȴ������
	while (USART_GetFlagStatus(g_Dcb[2].Usart, USART_FLAG_TC) == RESET);

	//��RS485
#ifdef	_COMM3_485_	   	
	SetLo(COMM3_485);			
#endif

#else		//USART��ʽ����
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
* ��	�ƣ�ReadComm1Data()
* ��	�ܣ�����������
* ��ڲ�����pData������ת��ĵ�ַ��nLen��Ҫ��ȡ���ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/

//����1
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
			//���ճ�ʱ
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

//����2
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
		//�ȴ��������		
		OSSemPend(m_pComm2Event, g_Dcb[1].nRecvTimeout, &g_Dcb[1].nCommErr);

		if(g_Dcb[1].nCommErr != 0)
		{
			//���ճ�ʱ
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

//����3
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
			//���ճ�ʱ
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
* ��	�ƣ�TIM2_IRQHandler
* ��	�ܣ���ʱ��2�ж�
* ��ڲ�������
* ���ڲ�������
* ˵	������ʱ��2�жϷ���������ʾ����������ϣ��������ݴ���
****************************************************************************/

//����1���ն�ʱ���ж�
#ifdef _USE_COMM1_
void TIM2_IRQHandler (void) 
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//�����ж�
	OSIntEnter(); 
	
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)	
	{
		//-------�յ�һ���ֽں���ʱһ��ʱ����û�յ�����Ϊ����������-----------
		OS_ENTER_CRITICAL();
		TIM_Cmd(TIM2, DISABLE);		//��ֹ��ʱ��
		TIM_SetCounter(TIM2, 0);	//���㶨ʱ��
		TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);	 //��ֹ�ж�
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //����жϱ�־	      
		OS_EXIT_CRITICAL();			

			
		g_Dcb[0].bReceiving = 0;
		g_Dcb[0].bReceiveEnd = 1;			//�ý������־
		OSSemPost(m_pComm1Event);			//�����ź�
		
	}

	//�˳��ж�
	OSIntExit(); 
}
#endif

//����2���ն�ʱ���ж�
#ifdef _USE_COMM2_
void TIM3_IRQHandler (void) 
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//�����ж�
	OSIntEnter(); 

	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)	
	{
		//-------�յ�һ���ֽں���ʱһ��ʱ����û�յ�����Ϊ����������-----------  
		OS_ENTER_CRITICAL();
		TIM_Cmd(TIM3, DISABLE);							//��ֹ��ʱ��
		TIM_SetCounter(TIM3, 0);						//���㶨ʱ��	 
		TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);	 	//��ֹ�ж�
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	 	//����жϱ�־	 
		OS_EXIT_CRITICAL();			

		
		g_Dcb[1].bReceiving = 0;	
		g_Dcb[1].bReceiveEnd = 1;						//�ý������־
		OSSemPost(m_pComm2Event);						//�����ź�
		 
	}

	//�˳��ж�
	OSIntExit(); 
}
#endif

//����3���ն�ʱ���ж�
#ifdef _USE_COMM3_
void TIM4_IRQHandler (void) 
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//�����ж�
	OSIntEnter();

	if(TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)	
	{
		//-------�յ�һ���ֽں���ʱһ��ʱ����û�յ�����Ϊ����������-----------
		OS_ENTER_CRITICAL();
		TIM_Cmd(TIM4, DISABLE);							//��ֹ��ʱ��
		TIM_SetCounter(TIM4, 0);						//���㶨ʱ��
		TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);	 	//��ֹ�ж�
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	 	//����жϱ�־	      
		OS_EXIT_CRITICAL();			

		 	
		g_Dcb[2].bReceiving = 0;
		g_Dcb[2].bReceiveEnd = 1;			//�ý������־			
		OSSemPost(m_pComm3Event);			//�����ź�
		
	}

	//�˳��ж�
	OSIntExit(); 
}
#endif

/****************************************************************************
* ��	�ƣ�DMA1_Channel4_IRQHandler		DMA1_Channel4_IRQn
* ��	�ܣ�DMA�����ж�
* ��ڲ�������
* ���ڲ�������
* ˵	����DMA�����жϷ���������ʾ�����������
****************************************************************************/

//����1 DMA
#ifdef _USART_DMA_TX__
#ifdef _USE_COMM1_
void DMA1_Channel4_IRQHandler(void)
{
	//�����ж�
	OSIntEnter();	
	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC4)) //Modify
	{
		//����״̬
		g_Dcb[0].bSendEnd = 1;			
		USART_DMACmd(g_Dcb[0].Usart, USART_DMAReq_Tx, DISABLE);
		DMA_Cmd(g_Dcb[0].Tx_DMA_Channel, DISABLE);         

		//�����ź���
		OSSemPost(m_pSendSyncEvent1);
	}

	//���DMA״̬
	DMA_ClearFlag(DMA1_FLAG_GL4| DMA1_FLAG_TC4 | DMA1_FLAG_HT4 | DMA1_FLAG_TE4);

	//�˳��ж�
	OSIntExit();
}
#endif //_USE_COMM1_
#endif //_USART_DMA_RX__


//����2 DMA
#ifdef _USART_DMA_TX__
#ifdef _USE_COMM2_
//#ifndef _COMM2_485_
void DMA1_Channel7_IRQHandler(void)
{
	//�����ж�
	OSIntEnter();	  
	
	//DMA�������
	if(DMA_GetFlagStatus(DMA1_FLAG_TC7))
	{
		//����״̬��Ϣ
		g_Dcb[1].bSendEnd = 1;										//�÷������־
		USART_DMACmd(g_Dcb[1].Usart, USART_DMAReq_Tx, DISABLE); 	//����DMA����
		DMA_Cmd(g_Dcb[1].Tx_DMA_Channel, DISABLE);					//����DMAͨ��  

		//�����ź���
		OSSemPost(m_pSendSyncEvent2);
	}

	//���DMA��־
	DMA_ClearFlag(DMA1_FLAG_GL7| DMA1_FLAG_TC7 | DMA1_FLAG_HT7 | DMA1_FLAG_TE7);  //Modify

	//�˳��ж�
	OSIntExit();
}
//#endif
#endif //_USE_COMM3_
#endif //_USART_DMA_RX__


//����1 �����ж�
#ifdef _USE_COMM1_
void USART1_IRQHandler(void)
{
	//�����ж�
	OSIntEnter();

	if(USART_GetITStatus(g_Dcb[0].Usart, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(g_Dcb[0].Usart,USART_IT_RXNE);

		//���㶨ʱ��
		TIM_SetCounter(TIM2, 0);	

		//������ʱ��ʱ��
		if(g_Dcb[0].bReceiving == 0)
		{
			g_Dcb[0].bReceiving = 1;
						
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //����жϱ�־	  
			TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	 //������������ж�    
			TIM_Cmd(TIM2, ENABLE);					 	 //����ʱ��
		}
		
		//��������
		if(g_Dcb[0].nReceiveIndex < COMM1_RECEIVE_MAX_LEN)
		{
			g_Dcb[0].pRecvBuff[g_Dcb[0].nReceiveIndex++] = USART_ReceiveData(g_Dcb[0].Usart);
		}		
	}

	//�˳��ж�
	OSIntExit();
}
#endif //_USE_COMM1_


//����2 �����ж�
#ifdef _USE_COMM2_
void USART2_IRQHandler(void)
{
	//�����ж�
	OSIntEnter();

	if(USART_GetITStatus(g_Dcb[1].Usart, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(g_Dcb[1].Usart,USART_IT_RXNE);

		//���㶨ʱ��
		TIM_SetCounter(TIM3, 0);

		//������ʱ��ʱ��
		if(g_Dcb[1].bReceiving == 0)
		{
			g_Dcb[1].bReceiving = 1;
						
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	 //����жϱ�־	  
			TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);	 //������������ж�    
			TIM_Cmd(TIM3, ENABLE);					 	 //����ʱ��
		}
			
		//��������	
		if(g_Dcb[1].nReceiveIndex < COMM1_RECEIVE_MAX_LEN)
		{
			g_Dcb[1].pRecvBuff[g_Dcb[1].nReceiveIndex++] = USART_ReceiveData(g_Dcb[1].Usart);
		}		
	} 

	//�˳��ж�
	OSIntExit(); 
}
#endif //_USE_COMM2_


//����3 �����ж�
#ifdef _USE_COMM3_
void USART3_IRQHandler(void)
{
	//�����ж�
	OSIntEnter();


	if(USART_GetITStatus(g_Dcb[2].Usart, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(g_Dcb[2].Usart,USART_IT_RXNE);

		//���㶨ʱ��
		TIM_SetCounter(TIM4, 0);	

		//������ʱ��ʱ��
		if(g_Dcb[2].bReceiving == 0)
		{
			g_Dcb[2].bReceiving = 1;
						
			TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	 //����жϱ�־	  
			TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);	 //������������ж�    
			TIM_Cmd(TIM4, ENABLE);					 	 //����ʱ��
		}
	
		//��������
		if(g_Dcb[2].nReceiveIndex < COMM1_RECEIVE_MAX_LEN)
		{
			g_Dcb[2].pRecvBuff[g_Dcb[2].nReceiveIndex++] = USART_ReceiveData(g_Dcb[2].Usart);
		}		
	}

	//�˳��ж�
	OSIntExit();
}
#endif //_USE_COMM3_




