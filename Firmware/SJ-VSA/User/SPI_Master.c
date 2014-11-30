#include "includes.h"
#include <ucos_ii.h>

uint8 SPI1_Buffer_Tx[128];
uint8 SPI2_Buffer_Tx[128];
uint8 SPI1_Buffer_Rx[128];
uint8 SPI2_Buffer_Rx[128];
uint16 BufferSize = 128;
uint16 m_nSendSize = 128;

#define SPI1_RECEIVE_MAX_LEN 128

typedef struct _COMMCTRL
{
	uint16		nSendSize;			   	//发送长度
	BOOL 		bSendEnd;			   	//发送完标志
	uint16		nSendIndex;			   	//发送索引
	BOOL 		bReceiveEnd;		   	//接收完标志
	uint16		nReceiveIndex;	       	//接收索引		
	BOOL		bIsReceivedData;	   	//是否收到数据
	uint8 		nCommErr;			   	//错误标志
}CommCtrl;

CommCtrl SPI1_CommCtrl, SPI2_CommCtrl;
void SPI1_DMA_Configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	/* SPI1_Tx_DMA_Channel configuration ---------------------------------------------*/
	DMA_DeInit(SPI1_Tx_DMA_Channel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SPI1_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI1_Buffer_Tx;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(SPI1_Tx_DMA_Channel, &DMA_InitStructure);
	
	/* SPI1_Rx_DMA_Channel configuration ---------------------------------------------*/
/*	DMA_DeInit(SPI1_Rx_DMA_Channel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SPI1_DR_Base;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI1_Buffer_Rx;
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	
	DMA_Init(SPI1_Rx_DMA_Channel, &DMA_InitStructure);*/
} 

void SPI1_Configuration(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable SPI and GPIO clocks */
	RCC_APB2PeriphClockCmd(SPI1_CLK | SPI1_GPIO_CLK | RCC_APB2Periph_GPIOC, ENABLE);
	
	/* Configure SPI pins: SCKand MOSI */
	GPIO_InitStructure.GPIO_Pin = SPI1_PIN_SCK | SPI1_PIN_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI1_GPIO, &GPIO_InitStructure);

	/* Configure SPI pins: MISO  */
	GPIO_InitStructure.GPIO_Pin = SPI1_PIN_MISO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SPI1_GPIO, &GPIO_InitStructure);

	/* Configure SPI pins: NCE */
	GPIO_InitStructure.GPIO_Pin = SPI1_PIN_NSS;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* SPI1 configuration ------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);
	/* Enable SPI1 */ 
	SPI_Cmd(SPI1, ENABLE);	
}
 
void NVIC_SPI1_Configuration(void)
{
   NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the SPI1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable DMA1 channel3 IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  /* Enable DMA1 channel2 IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
  NVIC_Init(&NVIC_InitStructure);
}

void SPI2_DMA_Configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	/* SPI2_Tx_DMA_Channel configuration ---------------------------------------------*/
	DMA_DeInit(SPI2_Tx_DMA_Channel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SPI2_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI2_Buffer_Tx;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(SPI2_Tx_DMA_Channel, &DMA_InitStructure);
	
	/* SPI2_Rx_DMA_Channel configuration ---------------------------------------------*/
	DMA_DeInit(SPI2_Rx_DMA_Channel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SPI2_DR_Base;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI2_Buffer_Rx;
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	
	DMA_Init(SPI2_Rx_DMA_Channel, &DMA_InitStructure);   
}

void SPI2_Configuration(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable SPI and GPIO clocks */
	RCC_APB1PeriphClockCmd(SPI2_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(SPI2_GPIO_CLK, ENABLE);
	
	/* Configure SPI pins: SCKand MOSI */
	GPIO_InitStructure.GPIO_Pin = SPI2_PIN_SCK | SPI2_PIN_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI2_GPIO, &GPIO_InitStructure);

	/* Configure SPI pins: MISO  */
	GPIO_InitStructure.GPIO_Pin = SPI2_PIN_MISO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SPI2_GPIO, &GPIO_InitStructure);

	/* SPI2 configuration ------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	/* Enable SPI1 */ 
	SPI_Cmd(SPI2, ENABLE);	
}

void OpenSPI1Comm(void)
{
	SPI1_DMA_Configuration();
	SPI1_Configuration();
}

void CloseSPI1Comm(void)
{
	/* Enable SPI1 */ 
	SPI_Cmd(SPI1, DISABLE);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);
	DMA_Cmd(SPI1_Tx_DMA_Channel, DISABLE);
	DMA_Cmd(SPI1_Rx_DMA_Channel, DISABLE);
	/* Deinitialize the SPI2 */ 
	SPI_I2S_DeInit(SPI1);
}
  
void OpenSPI2Comm(void)
{
	SPI2_DMA_Configuration();
	SPI2_Configuration();
}

void CloseSPI2Comm(void)
{
	/* Enable SPI1 */ 
	SPI_Cmd(SPI2, DISABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);
	DMA_Cmd(SPI2_Tx_DMA_Channel, DISABLE);
	DMA_Cmd(SPI2_Rx_DMA_Channel, DISABLE);
	/* Deinitialize the SPI2 */ 
	SPI_I2S_DeInit(SPI2);
}

void SPI1SendData(uint8 *pSendData,uint16 nDataSize)
{
	uint16 i;

//	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));	//等待TXE=1
//	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));	//等待BSY=0
	SPI1_Buffer_Tx;
	//将待发送数据放入发送缓冲区
	SPI1_CommCtrl.nSendSize = nDataSize;
	for(i=0; i<SPI1_CommCtrl.nSendSize; i++) SPI1_Buffer_Tx[i] = pSendData[i];
	//发送数据
	SPI1_CommCtrl.bSendEnd = 0;
	SPI1_Tx_DMA_Channel->CNDTR = SPI1_CommCtrl.nSendSize;// 发送字节数量
	/* Enable SPI1_Tx buffer DMA transfer request */ 
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	/* Enable DMA1 interrupt */
//	DMA_ITConfig(SPI1_Tx_DMA_Channel, DMA_IT_TC, ENABLE);
	/* Enable DMA1 Channel */
	DMA_Cmd(SPI1_Tx_DMA_Channel, ENABLE);
}

uint16 SPI1RecvData(uint8* pData, uint16 nLen) 
{
	uint16 nRealRead, i;

/*	if(SPI1_CommCtrl.bReceiveEnd == FALSE)
	{
		OSSemPend(m_pCommEvent, g_Dcb.nRecvTimeout, &m_nCommErr);	 //
	}*/
	if(nLen <= 0 || nLen>SPI1_CommCtrl.nReceiveIndex)
		nRealRead = SPI1_CommCtrl.nReceiveIndex;
	for(i=0;i<nRealRead;i++) pData[i] = SPI1_Buffer_Rx[i];

	SPI1_CommCtrl.nReceiveIndex = 0;
	return nRealRead;
} 

void SPI2SendData(uint8 *pSendData,uint16 nDataSize)
{
}

uint16 SPI2RecvData(uint8* pData, uint16 nLen) 
{
	return 0;
}

void DMA1_Channel3_IRQHandler(void)
{
	OSIntEnter();	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC3)) //Modify
	{
		SPI1_CommCtrl.bSendEnd = 1;					//置发送完标志
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE); 	//除能DMA发送
		DMA_Cmd(SPI1_Tx_DMA_Channel, DISABLE);		//除能DMA通道                
	}
	DMA_ClearFlag(DMA1_FLAG_GL3| DMA1_FLAG_TC3 | DMA1_FLAG_HT3 | DMA1_FLAG_TE3);  //Modify

	OSIntExit();
}

void SPI1_IRQHandler(void)
{
	if(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) != RESET)
	{
		SPI_I2S_ClearFlag(SPI1,SPI_I2S_FLAG_RXNE);
		/*if(!SPI1_CommCtrl.bIsReceivedData)
		{
			TIM_Cmd(TIM2, DISABLE);		//禁止计数器
			TIM_SetCounter(TIM2, 0);	//清零计数器
			TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);	 //禁止中断
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //清除中断标志	      
	
			m_bIsReceivedData = 1;
			TIM_Cmd(TIM2, ENABLE);					 	 //允许计数器
			TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	 //允许产生更新中断
		}*/
	
	
		if(SPI1_CommCtrl.nReceiveIndex < SPI1_RECEIVE_MAX_LEN)
		{
			TIM_SetCounter(TIM2, 0);	//清零计数器
			/* Read one byte from the receive data register */
			SPI1_Buffer_Rx[SPI1_CommCtrl.nReceiveIndex++] = SPI_I2S_ReceiveData(SPI1);
		}		
	}
}
 
