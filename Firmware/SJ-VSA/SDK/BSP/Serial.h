/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Seri1.h
**��   ��   ��: ����ǿ
**�� �� ��  ��: 2007��3��15��

**��		��: ��п�(kady1984@163.com)
**����޸�����: 2007��10��9��

**��		��: ������(zmingwang@yahoo.com.cn)
**����޸�����: 2010��9��17��

**��        ��: ����uCosII���첽�����շ� 
********************************************************************************************************/
#ifndef _SERIAL_H_
#define _SERIAL_H_

#ifdef	_SERIAL_C_
#define	SERIAL_EXT
#else
#define	SERIAL_EXT extern
#endif

//user defines-----------------------------------------
//#define		_COMM1_485_				
#define		_COMM2_485_				
//#define		_COMM3_485_				
#define 	COMM1_SEND_MAX_LEN		256
#define		COMM1_RECEIVE_MAX_LEN	256

//function control-------------------------------------
//#define _USE_COMM1_
#define _USE_COMM3_
#define _USE_COMM2_
//#define _SET_DCB1_
#define _SET_COMM1_RECV_TIME_OUT_
#define _SEND_COMM1_
#define _RECEIVE_COMM1_
#define _IS_SEND_END1_
//#define _IS_RECEIVE_END1_
//#define _CLEAR_COMM1_
//#define _CLOSE_COMM1_

#define USE_DMA_MODE
#define _USART_DMA_TX__
//#ifdef _USART_DMA_RX__
//#ifdef _USART_DMA_TX__
//#define _USART_INTERRUPT_NOT_USE__
//#endif
//#endif

//data define-------------------------------------------
#define 	NONE	0
#define		MASK	1

#define USART_485_HOST                   USART1
#define USART_485_HOST_GPIO              GPIOA
#define USART_485_HOST_CLK               RCC_APB2Periph_USART1
#define USART_485_HOST_GPIO_CLK          RCC_APB2Periph_GPIOA
#define USART_485_HOST_RxPin             GPIO_Pin_10
#define USART_485_HOST_TxPin             GPIO_Pin_9
#define USART_485_HOST_Tx_DMA_Channel    DMA1_Channel4
#define USART_485_HOST_Tx_DMA_FLAG       DMA1_FLAG_TC4
#define USART_485_HOST_Rx_DMA_Channel    DMA1_Channel5
#define USART_485_HOST_Rx_DMA_FLAG       DMA1_FLAG_TC5 
#define USART_485_HOST_DR_Base           0x40013804			

#define USART_485_SLAVE1                   USART2
#define USART_485_SLAVE1_GPIO              GPIOA
#define USART_485_SLAVE1_CLK               RCC_APB1Periph_USART2
#define USART_485_SLAVE1_GPIO_CLK          RCC_APB2Periph_GPIOA
#define USART_485_SLAVE1_RxPin             GPIO_Pin_3
#define USART_485_SLAVE1_TxPin             GPIO_Pin_2
#define USART_485_SLAVE1_Tx_DMA_Channel    DMA1_Channel7
#define USART_485_SLAVE1_Tx_DMA_FLAG       DMA1_FLAG_TC7
#define USART_485_SLAVE1_Rx_DMA_Channel    DMA1_Channel6
#define USART_485_SLAVE1_Rx_DMA_FLAG       DMA1_FLAG_TC6  
#define USART_485_SLAVE1_DR_Base           0x40004404	

#define USART_485_SLAVE2                   USART3
#define USART_485_SLAVE2_GPIO              GPIOB
#define USART_485_SLAVE2_CLK               RCC_APB1Periph_USART3
#define USART_485_SLAVE2_GPIO_CLK          RCC_APB2Periph_GPIOB
#define USART_485_SLAVE2_RxPin             GPIO_Pin_11
#define USART_485_SLAVE2_TxPin             GPIO_Pin_10
#define USART_485_SLAVE2_Tx_DMA_Channel    DMA1_Channel2
#define USART_485_SLAVE2_Tx_DMA_FLAG       DMA1_FLAG_TC2
#define USART_485_SLAVE2_Rx_DMA_Channel    DMA1_Channel3
#define USART_485_SLAVE2_Rx_DMA_FLAG       DMA2_FLAG_TC3  
#define USART_485_SLAVE2_DR_Base           0x40004804

#define	BOOL			BOOLEAN

typedef struct 
{
	//ͨ������
	USART_TypeDef		*Usart;
	GPIO_TypeDef		*Port;
	uint32_t			UsartClk;
	uint32_t			PortClk;
	uint16_t			RxPin;
	uint16_t			TxPin;

	//ͨ����Ϣ
	uint32 				nBaudRate;					//������							
	uint8 				nVerify;					//У��λ,0ΪNONE��1ΪżУ��
	
	//������Ϣ
	uint8				*pSendBuff;					//���ͻ�����ָ��	
	uint16				nSendIndex;					//��������
	uint16				nSendSize;					//���ݴ�С	
	BOOL 				bSendEnd;					//�������־
	uint8				bSendSync;					//ͬ������

	//������Ϣ
	uint8				*pRecvBuff;					//���ջ�����ָ��
	int16				nReceiveIndex;				//��������
	BOOL				bReceiving;					//���ڽ���
	BOOL 				bReceiveEnd ;				//�������־
	
	//��ʱ����
	TIM_TypeDef			*Timer;						//��ʱ��ʱ��
	uint32_t			TimerClk;					//��ʱ��ʱ��ʱ��	
	uint16				nRecvTimeout;				//��ʱ������
	uint8 				nCommErr;					//�����־

	//DMA��Ϣ
#ifdef USE_DMA_MODE
	DMA_Channel_TypeDef *Tx_DMA_Channel;
	uint32_t			Tx_DMA_Flag;
	DMA_Channel_TypeDef *Rx_DMA_Channel;
	uint32_t			Rx_DMA_Flag;
	uint32_t			DMA_DR_Base;
#endif
	
} DCB;

/****************************************************************************
* ��	�ƣ�Comm1PortInit()
* ��	�ܣ���ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void UART_Configuration(DCB* pDCB);
void Timer_Config(DCB *pDCB);
//#define _DEBUG_TIMER2_
#ifdef _DEBUG_TIMER2_
void EnableTimer2(void);
#endif

/****************************************************************************
* ��	�ƣ�void SetSyncMode(uint8 bSync)
* ��	�ܣ����÷���ͬ��ģʽ
* ��ڲ�����bSync  �Ƿ�ͬ������, 0Ϊ�첽����,��0Ϊͬ������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SetSyncMode(uint8 bSync);

/****************************************************************************
* ��	�ƣ�UsartVariInit()
* ��	�ܣ�����ȫ�ֱ�����ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void UsartVariInit(void);

/****************************************************************************
* ��	�ƣ�SetCommBaudRate()
* ��	�ܣ����ò�����
* ��ڲ�����nPort���˿ڣ�nBaudRate��������
* ���ڲ�������
* ˵	������Open֮ǰ������Ѿ�Open�ˣ���Ҫ����Open
****************************************************************************/
void SetCommBaudRate(uint8 nPort, uint32 nBaudRate);

/****************************************************************************
* ��	�ƣ�SetCommRecvTimeOut()
* ��	�ܣ����ý��ճ�ʱʱ��
* ��ڲ�����nPort���˿ڣ�nRecvTimeout����ʱʱ��(ms)
* ���ڲ�������
* ˵	������ʱ������
****************************************************************************/
void SetCommRecvTimeOut(uint8 nPort, uint16 nRecvTimeout);


/****************************************************************************
* ��	�ƣ�CommPortOpen()
* ��	�ܣ��򿪶˿�
* ��ڲ�����nPort���˿�
* ���ڲ�������
* ˵	������
****************************************************************************/
void CommPortOpen(uint8 nPort);


/****************************************************************************
* ��	�ƣ�CommPortShut()
* ��	�ܣ��رն˿�
* ��ڲ�����nPort���˿�
* ���ڲ�������
* ˵	������
****************************************************************************/
void CommPortShut(uint8 nPort);


/****************************************************************************
* ��	�ƣ�OpenComm()
* ��	�ܣ��򿪴���
* ��ڲ�������
* ���ڲ�������
* ˵	�������ô��ڲ���,�ڵ��ô˺���֮ǰ���ȳ�ʼ��DCB
****************************************************************************/
SERIAL_EXT void OpenComm1(void);
SERIAL_EXT void OpenComm2(void);
SERIAL_EXT void OpenComm3(void);


/****************************************************************************
* ��	�ƣ�CloseComm()
* ��	�ܣ��رմ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
SERIAL_EXT void CloseComm1(void);
SERIAL_EXT void CloseComm2(void);
SERIAL_EXT void CloseComm3(void);


/****************************************************************************
* ��	�ƣ�SendComm()
* ��	�ܣ���������
* ��ڲ�����pSendData�Ǵ����͵�����,nDataSize�Ǵ��������ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
SERIAL_EXT void SendComm1(uint8 *pSendData, int16 nDataSize);
SERIAL_EXT void SendComm2(uint8 *pSendData, int16 nDataSize);
SERIAL_EXT void SendComm3(uint8 *pSendData, int16 nDataSize);

/****************************************************************************
* ��	�ƣ�ReadCommData()
* ��	�ܣ�����������
* ��ڲ�����pData������ת��ĵ�ַ��nLen��Ҫ��ȡ���ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
SERIAL_EXT int16 RecvComm1(uint8* pData, int16 nLen);
SERIAL_EXT int16 RecvComm2(uint8* pData, int16 nLen);
SERIAL_EXT int16 RecvComm3(uint8* pData, int16 nLen);


/****************************************************************************
* ��	�ƣ�ClearComm()
* ��	�ܣ������������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void ClearComm1(void);
void ClearComm2(void);
void ClearComm3(void);


/****************************************************************************
* ��	�ƣ�IsSendEnd()
* ��	�ܣ��жϴ��ڷ�����ϱ�־
* ��ڲ�������
* ���ڲ�����0��		δ�������
			1��		������ϱ�־
* ˵	������
****************************************************************************/
SERIAL_EXT BOOL IsComm1SendEnd(void);
SERIAL_EXT BOOL IsComm2SendEnd(void);
SERIAL_EXT BOOL IsComm3SendEnd(void);


/****************************************************************************
* ��	�ƣ�IsReceiveEnd()
* ��	�ܣ��жϴ��ڽ�����ϱ�־
* ��ڲ�������
* ���ڲ�����0��		δ�������
			1��		������ϱ�־
* ˵	������
****************************************************************************/
SERIAL_EXT uint16 IsComm1ReceiveEnd(void);
SERIAL_EXT uint16 IsComm2ReceiveEnd(void);
SERIAL_EXT uint16 IsComm3ReceiveEnd(void);

#endif


