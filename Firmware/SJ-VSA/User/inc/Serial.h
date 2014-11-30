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
#define		_COMM1_485_				0
#define 	COMM1_SEND_MAX_LEN		45
#define		COMM1_RECEIVE_MAX_LEN	64

//function control-------------------------------------
#define _USE_COMM1_
//#define _SET_DCB1_
#define _SET_COMM1_RECV_TIME_OUT_
#define _SEND_COMM1_
#define _RECEIVE_COMM1_
#define _IS_SEND_END1_
//#define _IS_RECEIVE_END1_
//#define _CLEAR_COMM1_
//#define _CLOSE_COMM1_

//#define _USART_DMA_RX__
#define _USART_DMA_TX__
#ifdef _USART_DMA_RX__
#ifdef _USART_DMA_TX__
#define _USART_INTERRUPT_NOT_USE__
#endif
#endif

//data define-------------------------------------------
#define 	NONE	0
#define		MASK	1
			
#define USART_485_HOST                   USART2
#define USART_485_HOST_GPIO              GPIOA
#define USART_485_HOST_CLK               RCC_APB1Periph_USART2
#define USART_485_HOST_GPIO_CLK          RCC_APB2Periph_GPIOA
#define USART_485_HOST_RxPin             GPIO_Pin_3
#define USART_485_HOST_TxPin             GPIO_Pin_2
#define USART_485_HOST_Tx_DMA_Channel    DMA1_Channel7
#define USART_485_HOST_Tx_DMA_FLAG       DMA1_FLAG_TC7
#define USART_485_HOST_Rx_DMA_Channel    DMA1_Channel6
#define USART_485_HOST_Rx_DMA_FLAG       DMA1_FLAG_TC6  
#define USART_485_HOST_DR_Base           0x40004404	

// #define USART_485_SLAVE                   USART2
// #define USART_485_SLAVE_GPIO              GPIOA
// #define USART_485_SLAVE_CLK               RCC_APB1Periph_USART2
// #define USART_485_SLAVE_GPIO_CLK          RCC_APB2Periph_GPIOA
// #define USART_485_SLAVE_RxPin             GPIO_Pin_3
// #define USART_485_SLAVE_TxPin             GPIO_Pin_2
// #define USART_485_SLAVE_Tx_DMA_Channel    DMA1_Channel7
// #define USART_485_SLAVE_Tx_DMA_FLAG       DMA1_FLAG_TC7
// #define USART_485_SLAVE_Rx_DMA_Channel    DMA1_Channel6
// #define USART_485_SLAVE_Rx_DMA_FLAG       DMA1_FLAG_TC6  
// #define USART_485_SLAVE_DR_Base           0x40004404	

#define USART_485_SLAVE                   UART4
#define USART_485_SLAVE_GPIO              GPIOC
#define USART_485_SLAVE_CLK               RCC_APB1Periph_UART4
#define USART_485_SLAVE_GPIO_CLK          RCC_APB2Periph_GPIOC
#define USART_485_SLAVE_RxPin             GPIO_Pin_11
#define USART_485_SLAVE_TxPin             GPIO_Pin_10
#define USART_485_SLAVE_Tx_DMA_Channel    DMA2_Channel5
#define USART_485_SLAVE_Tx_DMA_FLAG       DMA2_FLAG_TC5
#define USART_485_SLAVE_Rx_DMA_Channel    DMA2_Channel3
#define USART_485_SLAVE_Rx_DMA_FLAG       DMA2_FLAG_TC3  
#define USART_485_SLAVE_DR_Base           0x40004C04	

/*
#define USART_485_HOST                   USART1
#define USART_485_HOST_GPIO              GPIOA
#define USART_485_HOST_CLK               RCC_APB2Periph_USART2
#define USART_485_HOST_GPIO_CLK          RCC_APB2Periph_GPIOA
#define USART_485_HOST_RxPin             GPIO_Pin_10
#define USART_485_HOST_TxPin             GPIO_Pin_9
#define USART_485_HOST_Tx_DMA_Channel    DMA1_Channel4
#define USART_485_HOST_Tx_DMA_FLAG       DMA1_FLAG_TC4
#define USART_485_HOST_Rx_DMA_Channel    DMA1_Channel5
#define USART_485_HOST_Rx_DMA_FLAG       DMA1_FLAG_TC5 
#define USART_485_HOST_DR_Base           0x40013804	*/

#define USART_GPRS                   		USART3
#define USART_GPRS_GPIO              		GPIOB
#define USART_GPRS_CLK               		RCC_APB1Periph_USART3
#define USART_GPRS_GPIO_CLK          		RCC_APB2Periph_GPIOB
#define USART_GPRS_RxPin             		GPIO_Pin_11
#define USART_GPRS_TxPin             		GPIO_Pin_10
#define USART_GPRS_Tx_DMA_Channel    		DMA1_Channel2
#define USART_GPRS_Tx_DMA_FLAG       		DMA1_FLAG_TC2
#define USART_GPRS_Rx_DMA_Channel    		DMA1_Channel3
#define USART_GPRS_Rx_DMA_FLAG       		DMA1_FLAG_TC3 
#define USART_GPRS_DR_Base           		0x40004804

#define USART_485_EP                   	USART4
#define USART_485_EP_GPIO              	GPIOC
#define USART_485_EP_CLK               	RCC_APB1Periph_USART4
#define USART_485_EP_GPIO_CLK          	RCC_APB2Periph_GPIOC
#define USART_485_EP_RxPin             	GPIO_Pin_11
#define USART_485_EP_TxPin             	GPIO_Pin_10
#define USART_485_EP_Tx_DMA_Channel    	DMA2_Channel5
#define USART_485_EP_Tx_DMA_FLAG       	DMA2_FLAG_TC5
#define USART_485_EP_Rx_DMA_Channel    	DMA2_Channel3
#define USART_485_EP_Rx_DMA_FLAG       	DMA2_FLAG_TC3 
#define USART_485_EP_DR_Base           	0x40004C04

#define TxBufferSize1   (countof(TxBuffer1) - 1)
#define TxBufferSize2   (countof(TxBuffer2) - 1)
#define TxBufferSize3   (countof(TxBuffer3) - 1)

//data type define--------------------------------------
typedef INT8U   BYTE ;          //������ǰ�汾����������
typedef INT16U  WORD ;
typedef INT32U  LONG;
typedef	INT32U  DWORD;

typedef	INT8U	uint8;		
typedef	INT8S	int8;		
typedef	INT16U	uint16;		
typedef	INT16S	int16;		
typedef	INT32U	uint32;	
typedef	INT32S	int32;		
typedef	FP32	fp32;		
typedef	FP64	fp64;		

#define	BOOL			BOOLEAN

typedef struct 
{
	uint32 	nBaudRate;		//�����ʣ���ѡ�������У�
							//	1200,2400,9600,14400,28800,57600,115200,230400
	uint8 	nVerify;			//У��λ,0ΪNONE��1ΪżУ��

	uint16	nRecvTimeout;
	uint8	bSendSync;
} DCB;
/****************************************************************************
* ��	�ƣ�Comm1PortInit()
* ��	�ܣ���ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void UART_Slave_Configuration(void);
void UART_Slave_Send(u8 *pData, u16 nSize);

/****************************************************************************
* ��	�ƣ�void SetSyncMode(uint8 bSync)
* ��	�ܣ����÷���ͬ��ģʽ
* ��ڲ�����bSync  �Ƿ�ͬ������, 0Ϊ�첽����,��0Ϊͬ������
* ���ڲ�������
* ˵	������
****************************************************************************/
void SetSyncMode(uint8 bSync);

/****************************************************************************
* ��	�ƣ�Comm1VariInit()
* ��	�ܣ�����ȫ�ֱ�����ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void Comm1VariInit(void);

/****************************************************************************
* ��	�ƣ�SetDCB()
* ��	�ܣ����ô��ڲ���
* ��ڲ�����pDcb���Ѷ���Ĵ��ڲ���
* ���ڲ�������
* ˵	�������ô��ڲ���
****************************************************************************/
SERIAL_EXT void SetDCB1(DCB *pDcb);


/****************************************************************************
* ��	�ƣ�SetRecvTimeOut(uint16 nTimeOut)
* ��	�ܣ����賬ʱʱ��
* ��ڲ���������ĳ�ʱʱ�䣬��λ��ms
* ���ڲ�������
* ˵	������
****************************************************************************/


SERIAL_EXT void SetComm1RecvTimeOut(uint16 nTimeOut);
/****************************************************************************
* ��	�ƣ�OpenComm()
* ��	�ܣ��򿪴���
* ��ڲ�������
* ���ڲ�������
* ˵	�������ô��ڲ���,�ڵ��ô˺���֮ǰ���ȳ�ʼ��DCB
****************************************************************************/
SERIAL_EXT void OpenComm1(void);


/****************************************************************************
* ��	�ƣ�CloseComm()
* ��	�ܣ��رմ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
SERIAL_EXT void CloseComm1(void);


/****************************************************************************
* ��	�ƣ�SendComm()
* ��	�ܣ���������
* ��ڲ�����pSendData�Ǵ����͵�����,nDataSize�Ǵ��������ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
SERIAL_EXT void SendComm1(uint8 *pSendData,uint16 nDataSize);
#define SendStr(x,y) SendComm1(x,y)

/****************************************************************************
* ��	�ƣ�ReceiveComm()
* ��	�ܣ���������
* ��ڲ�������
* ���ڲ�����0��		��ʱ
			��0��	���յ������ݸ���
* ˵	������
****************************************************************************/
//SERIAL_EXT uint16 ReceiveComm1(void);


/****************************************************************************
* ��	�ƣ�ReadCommData()
* ��	�ܣ�����������
* ��ڲ�����pData������ת��ĵ�ַ��nLen��Ҫ��ȡ���ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
SERIAL_EXT uint16 ReadComm1Data(uint8* pData, uint16 nLen);


/****************************************************************************
* ��	�ƣ�IsSendEnd()
* ��	�ܣ��жϴ��ڷ�����ϱ�־
* ��ڲ�������
* ���ڲ�����0��		δ�������
			1��		������ϱ�־
* ˵	������
****************************************************************************/
SERIAL_EXT BOOL IsComm1SendEnd(void);


/****************************************************************************
* ��	�ƣ�IsReceiveEnd()
* ��	�ܣ��жϴ��ڽ�����ϱ�־
* ��ڲ�������
* ���ڲ�����0��		δ�������
			1��		������ϱ�־
* ˵	������
****************************************************************************/
SERIAL_EXT uint16 IsComm1ReceiveEnd(void);


/****************************************************************************
* ��	�ƣ�ClearComm()
* ��	�ܣ��������
* ��ڲ�������
* ���ڲ�������
* ˵	���������ڻ�ԭΪ��ʼ״̬
****************************************************************************/
SERIAL_EXT void ClearComm1(void);

#endif


