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
	//通道配置
	USART_TypeDef		*Usart;
	GPIO_TypeDef		*Port;
	uint32_t			UsartClk;
	uint32_t			PortClk;
	uint16_t			RxPin;
	uint16_t			TxPin;

	//通信信息
	uint32 				nBaudRate;					//波特率							
	uint8 				nVerify;					//校验位,0为NONE，1为偶校验
	
	//发送信息
	uint8				*pSendBuff;					//发送缓冲区指针	
	uint16				nSendIndex;					//发送索引
	uint16				nSendSize;					//数据大小	
	BOOL 				bSendEnd;					//发送完标志
	uint8				bSendSync;					//同步发送

	//接收信息
	uint8				*pRecvBuff;					//接收缓冲区指针
	int16				nReceiveIndex;				//接收索引
	BOOL				bReceiving;					//正在接收
	BOOL 				bReceiveEnd ;				//接收完标志
	
	//超时控制
	TIM_TypeDef			*Timer;						//超时定时器
	uint32_t			TimerClk;					//超时定时器时钟	
	uint16				nRecvTimeout;				//超时计数器
	uint8 				nCommErr;					//错误标志

	//DMA信息
#ifdef USE_DMA_MODE
	DMA_Channel_TypeDef *Tx_DMA_Channel;
	uint32_t			Tx_DMA_Flag;
	DMA_Channel_TypeDef *Rx_DMA_Channel;
	uint32_t			Rx_DMA_Flag;
	uint32_t			DMA_DR_Base;
#endif
	
} DCB;

/****************************************************************************
* 名	称：Comm1PortInit()
* 功	能：初始化串口
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void UART_Configuration(DCB* pDCB);
void Timer_Config(DCB *pDCB);
//#define _DEBUG_TIMER2_
#ifdef _DEBUG_TIMER2_
void EnableTimer2(void);
#endif

/****************************************************************************
* 名	称：void SetSyncMode(uint8 bSync)
* 功	能：设置发送同步模式
* 入口参数：bSync  是否同步发送, 0为异步发送,非0为同步发送
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetSyncMode(uint8 bSync);

/****************************************************************************
* 名	称：UsartVariInit()
* 功	能：串口全局变量初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void UsartVariInit(void);

/****************************************************************************
* 名	称：SetCommBaudRate()
* 功	能：设置波特率
* 入口参数：nPort，端口；nBaudRate，波特率
* 出口参数：无
* 说	明：在Open之前，如果已经Open了，则要重启Open
****************************************************************************/
void SetCommBaudRate(uint8 nPort, uint32 nBaudRate);

/****************************************************************************
* 名	称：SetCommRecvTimeOut()
* 功	能：设置接收超时时间
* 入口参数：nPort，端口；nRecvTimeout，超时时间(ms)
* 出口参数：无
* 说	明：随时可设置
****************************************************************************/
void SetCommRecvTimeOut(uint8 nPort, uint16 nRecvTimeout);


/****************************************************************************
* 名	称：CommPortOpen()
* 功	能：打开端口
* 入口参数：nPort，端口
* 出口参数：无
* 说	明：无
****************************************************************************/
void CommPortOpen(uint8 nPort);


/****************************************************************************
* 名	称：CommPortShut()
* 功	能：关闭端口
* 入口参数：nPort，端口
* 出口参数：无
* 说	明：无
****************************************************************************/
void CommPortShut(uint8 nPort);


/****************************************************************************
* 名	称：OpenComm()
* 功	能：打开串口
* 入口参数：无
* 出口参数：无
* 说	明：设置串口参数,在调用此函数之前请先初始化DCB
****************************************************************************/
SERIAL_EXT void OpenComm1(void);
SERIAL_EXT void OpenComm2(void);
SERIAL_EXT void OpenComm3(void);


/****************************************************************************
* 名	称：CloseComm()
* 功	能：关闭串口
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
SERIAL_EXT void CloseComm1(void);
SERIAL_EXT void CloseComm2(void);
SERIAL_EXT void CloseComm3(void);


/****************************************************************************
* 名	称：SendComm()
* 功	能：发送数据
* 入口参数：pSendData是待发送的数据,nDataSize是待发送数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
SERIAL_EXT void SendComm1(uint8 *pSendData, int16 nDataSize);
SERIAL_EXT void SendComm2(uint8 *pSendData, int16 nDataSize);
SERIAL_EXT void SendComm3(uint8 *pSendData, int16 nDataSize);

/****************************************************************************
* 名	称：ReadCommData()
* 功	能：读串口数据
* 入口参数：pData：数据转存的地址；nLen：要读取数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
SERIAL_EXT int16 RecvComm1(uint8* pData, int16 nLen);
SERIAL_EXT int16 RecvComm2(uint8* pData, int16 nLen);
SERIAL_EXT int16 RecvComm3(uint8* pData, int16 nLen);


/****************************************************************************
* 名	称：ClearComm()
* 功	能：清除串口数据
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ClearComm1(void);
void ClearComm2(void);
void ClearComm3(void);


/****************************************************************************
* 名	称：IsSendEnd()
* 功	能：判断串口发送完毕标志
* 入口参数：无
* 出口参数：0：		未发送完毕
			1：		发送完毕标志
* 说	明：无
****************************************************************************/
SERIAL_EXT BOOL IsComm1SendEnd(void);
SERIAL_EXT BOOL IsComm2SendEnd(void);
SERIAL_EXT BOOL IsComm3SendEnd(void);


/****************************************************************************
* 名	称：IsReceiveEnd()
* 功	能：判断串口接收完毕标志
* 入口参数：无
* 出口参数：0：		未接收完毕
			1：		接收完毕标志
* 说	明：无
****************************************************************************/
SERIAL_EXT uint16 IsComm1ReceiveEnd(void);
SERIAL_EXT uint16 IsComm2ReceiveEnd(void);
SERIAL_EXT uint16 IsComm3ReceiveEnd(void);

#endif


