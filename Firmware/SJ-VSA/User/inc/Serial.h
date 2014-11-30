/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Seri1.h
**创   建   人: 罗仕强
**创 建 日  期: 2007年3月15日

**改		编: 杨承凯(kady1984@163.com)
**最后修改日期: 2007年10月9日

**改		编: 张明望(zmingwang@yahoo.com.cn)
**最后修改日期: 2010年9月17日

**描        述: 基于uCosII的异步串口收发 
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
typedef INT8U   BYTE ;          //兼容以前版本的数据类型
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
	uint32 	nBaudRate;		//波特率，可选波特率有：
							//	1200,2400,9600,14400,28800,57600,115200,230400
	uint8 	nVerify;			//校验位,0为NONE，1为偶校验

	uint16	nRecvTimeout;
	uint8	bSendSync;
} DCB;
/****************************************************************************
* 名	称：Comm1PortInit()
* 功	能：初始化串口
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void UART_Slave_Configuration(void);
void UART_Slave_Send(u8 *pData, u16 nSize);

/****************************************************************************
* 名	称：void SetSyncMode(uint8 bSync)
* 功	能：设置发送同步模式
* 入口参数：bSync  是否同步发送, 0为异步发送,非0为同步发送
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetSyncMode(uint8 bSync);

/****************************************************************************
* 名	称：Comm1VariInit()
* 功	能：串口全局变量初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void Comm1VariInit(void);

/****************************************************************************
* 名	称：SetDCB()
* 功	能：设置串口参数
* 入口参数：pDcb是已定义的串口参数
* 出口参数：无
* 说	明：设置串口参数
****************************************************************************/
SERIAL_EXT void SetDCB1(DCB *pDcb);


/****************************************************************************
* 名	称：SetRecvTimeOut(uint16 nTimeOut)
* 功	能：重设超时时间
* 入口参数：重设的超时时间，单位：ms
* 出口参数：无
* 说	明：无
****************************************************************************/


SERIAL_EXT void SetComm1RecvTimeOut(uint16 nTimeOut);
/****************************************************************************
* 名	称：OpenComm()
* 功	能：打开串口
* 入口参数：无
* 出口参数：无
* 说	明：设置串口参数,在调用此函数之前请先初始化DCB
****************************************************************************/
SERIAL_EXT void OpenComm1(void);


/****************************************************************************
* 名	称：CloseComm()
* 功	能：关闭串口
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
SERIAL_EXT void CloseComm1(void);


/****************************************************************************
* 名	称：SendComm()
* 功	能：发送数据
* 入口参数：pSendData是待发送的数据,nDataSize是待发送数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
SERIAL_EXT void SendComm1(uint8 *pSendData,uint16 nDataSize);
#define SendStr(x,y) SendComm1(x,y)

/****************************************************************************
* 名	称：ReceiveComm()
* 功	能：接收数据
* 入口参数：无
* 出口参数：0：		超时
			非0：	接收到的数据个数
* 说	明：无
****************************************************************************/
//SERIAL_EXT uint16 ReceiveComm1(void);


/****************************************************************************
* 名	称：ReadCommData()
* 功	能：读串口数据
* 入口参数：pData：数据转存的地址；nLen：要读取数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
SERIAL_EXT uint16 ReadComm1Data(uint8* pData, uint16 nLen);


/****************************************************************************
* 名	称：IsSendEnd()
* 功	能：判断串口发送完毕标志
* 入口参数：无
* 出口参数：0：		未发送完毕
			1：		发送完毕标志
* 说	明：无
****************************************************************************/
SERIAL_EXT BOOL IsComm1SendEnd(void);


/****************************************************************************
* 名	称：IsReceiveEnd()
* 功	能：判断串口接收完毕标志
* 入口参数：无
* 出口参数：0：		未接收完毕
			1：		接收完毕标志
* 说	明：无
****************************************************************************/
SERIAL_EXT uint16 IsComm1ReceiveEnd(void);


/****************************************************************************
* 名	称：ClearComm()
* 功	能：清除串口
* 入口参数：无
* 出口参数：无
* 说	明：将串口还原为初始状态
****************************************************************************/
SERIAL_EXT void ClearComm1(void);

#endif


