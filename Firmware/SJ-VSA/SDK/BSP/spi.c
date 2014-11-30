/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: spi.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2006��8��6��
**����޸�����: 2006��8��6��
**��        ��: spiԴ�ļ�
********************************************************************************************************/
#define	_SPI_C_

#include "includes.h"
#include "spi.h"
#include "CpuPortAccess.h"

//defines*****************************************************************
#define	SPI_HARDWARE_MODE	1						//Ӳ��SPIģʽ
#define	SPI_SOFTWARE_MODE	2						//���SPIģʽ


//SPI1*******************************************************************
#define SPI1_MODE 			SPI_HARDWARE_MODE
//#define USE_SPI1_DMA
//#define SPI1_DMA_TX
//#define SPI1_DMA_RX

//�˿ڶ���=============================
//ʱ���ź�                                                                                                                                                                                                                                                                                           
#define SPI1_SCK_PORT		SPI1_GPIO
#define SPI1_SCK			5

//�����ź�
//��������
#define SPI1_MISO_PORT	SPI1_GPIO
#define SPI1_MISO		6

//�������
#define SPI1_MOSI_PORT	SPI1_GPIO
#define SPI1_MOSI		7

//���ݶ���==============================
#ifdef USE_SPI1_DMA
uint8 Spi1SendBuff[8];
uint8 Spi1RecvBuff[8];
static OS_EVENT 	*m_pSpi1DmaEvent = NULL;		//ͨ��1�¼����ƿ�
static uint8 		m_nSpi1DmaErr;
#endif


void SPI1_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	
#ifdef USE_SPI1_DMA	
	DMA_InitTypeDef DMA_InitStructure;
#endif

#if	(SPI1_MODE == SPI_HARDWARE_MODE)
	SPI_InitTypeDef  SPI_InitStructure;

	//ʹ��SPI1ʱ��
	RCC_APB2PeriphClockCmd(SPI1_CLK, ENABLE);

#endif

	RCC_APB2PeriphClockCmd(SPI1_GPIO_CLK, ENABLE);
	
	//����SPI1 ʱ�Ӻ������������
	GPIO_InitStructure.GPIO_Pin = SPI1_PIN_SCK | SPI1_PIN_MOSI;

#if	(SPI1_MODE == SPI_SOFTWARE_MODE)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#else
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
#endif


	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI1_GPIO, &GPIO_InitStructure);

	//����SPI1���������
	GPIO_InitStructure.GPIO_Pin = SPI1_PIN_MISO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SPI1_GPIO, &GPIO_InitStructure);

	//����SPI1Ƭѡ��
	GPIO_InitStructure.GPIO_Pin = SPI1_PIN_NSS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

#if	(SPI1_MODE == SPI_HARDWARE_MODE)
	
	//
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;	   //72/16=4.5MHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	
	//����
	SPI_Cmd(SPI1, ENABLE);	
#endif



#ifdef USE_SPI1_DMA

  

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

#ifdef SPI1_DMA_TX
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//
	DMA_DeInit(SPI1_Tx_DMA_Channel);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI1_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Spi1SendBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					   
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(SPI1_Tx_DMA_Channel, &DMA_InitStructure);	 //pDCB->Tx_DMA_Channel
	DMA_ITConfig(SPI1_Tx_DMA_Channel, DMA_IT_TC, ENABLE); //pDCB->Tx_DMA_Channel
	//DMA_Cmd(SPI1_Tx_DMA_Channel, ENABLE);						//ʼ��DMAͨ��
#endif
#ifdef SPI1_DMA_RX
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);//
	DMA_DeInit(SPI1_Rx_DMA_Channel);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI1_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Spi1RecvBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(SPI1_Rx_DMA_Channel, &DMA_InitStructure);
	DMA_ITConfig(SPI1_Rx_DMA_Channel, DMA_IT_TC, ENABLE);    
	//DMA_Cmd(SPI1_Rx_DMA_Channel, ENABLE);						//ʼ��DMAͨ��
#endif


#endif


}

void SPI1_Close(void)
{
	//��ֹSPI1
	SPI_Cmd(SPI1, DISABLE);

	//SPI��ԭĬ������
	SPI_I2S_DeInit(SPI1);
}

void SPI1_Disable(void)
{
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);//
	SPI_Cmd(SPI1, DISABLE);	
	RCC_APB2PeriphClockCmd(SPI1_CLK, DISABLE);
	RCC_APB2PeriphClockCmd(SPI1_GPIO_CLK, DISABLE);
}

void SPI1_Enable(void)
{
	RCC_APB2PeriphClockCmd(SPI1_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(SPI1_CLK, ENABLE);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);//
	SPI_Cmd(SPI1, ENABLE);	
}


#if	(SPI1_MODE == SPI_SOFTWARE_MODE)
void _spi1_nop()
{
	int i = 10;
	while(i--);
}
#endif

uint8_t SPI1_RwByte(uint8_t byte)
{
#if	(SPI1_MODE == SPI_SOFTWARE_MODE)
	unsigned char nShiftOut, nShiftIn;
	unsigned char nCount = 8;

	nShiftOut = byte;	
	while(nCount--)
	{
		if(nShiftOut & 0x80)	SetHi(SPI1_MOSI);	  
		else					SetLo(SPI1_MOSI);
		SetHi(SPI1_SCK);		

		nShiftOut <<= 1;
		_spi1_nop();
		
		
		nShiftIn <<= 1;
		if(GetSignal(SPI1_MISO) == 1)
		{
			nShiftIn |= 0x01;
		}
		
		SetLo(SPI1_SCK);   		
		_spi1_nop();
	}
	return nShiftIn;

#else 

 
  //�ȴ�ԭ�з������
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);		

  //����
  SPI_I2S_SendData(SPI1, byte);

  //�ȴ��������
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  //���ؽ��
  return SPI_I2S_ReceiveData(SPI1);
#endif
}


#ifdef USE_SPI1_DMA
void SPI1_Send(uint8* pData, uint32 nLen)
{
#ifndef SPI1_DMA_TX
	while(nLen--)
	{
		SPI1_RwByte(*pData);
		pData++;
	}				  
#else

	if(m_pSpi1DmaEvent == NULL)
	{
		m_pSpi1DmaEvent = OSSemCreate(0);
	}

	SPI1_Tx_DMA_Channel->CMAR = (uint32)pData;
	SPI1_Tx_DMA_Channel->CNDTR = nLen;

	DMA_Cmd(SPI1_Tx_DMA_Channel, ENABLE);	
#endif

}


void SPI1_Recv(uint8* pData, uint32 nLen)
{
#ifndef SPI1_DMA_RX
	while(nLen--)
	{
		*pData = SPI1_RwByte(0xFF);
		pData++;
	}				  
#else

	if(m_pSpi1DmaEvent == NULL)
	{
		m_pSpi1DmaEvent = OSSemCreate(0);
	}

	SPI1_Rx_DMA_Channel->CMAR = (uint32)pData;
	SPI1_Rx_DMA_Channel->CNDTR = nLen;

	DMA_Cmd(SPI1_Rx_DMA_Channel, ENABLE);	

#endif
}



uint8 SPI1_WaitDMA()
{
	OSSemPend(m_pSpi1DmaEvent, 0, &m_nSpi1DmaErr);

	return m_nSpi1DmaErr;
}
#endif




/****************************************************************************
* ��	�ƣ�SPI1 DMA�����ж�
* ��	�ܣ�DMA�����ж�
* ��ڲ�������
* ���ڲ�������
* ˵	����DMA�����жϷ���������ʾ�����������
****************************************************************************/
#ifdef USE_SPI1_DMA
#ifdef SPI1_DMA_TX
void DMA1_Channel3_IRQHandler(void)
{
	OSIntEnter();	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC3)) //Modify
	{
	//	USART_DMACmd(g_Dcb[0].Usart, USART_DMAReq_Tx, DISABLE); //����DMA����
		DMA_Cmd(SPI1_Tx_DMA_Channel, DISABLE);	//����DMAͨ��                


		OSSemPost(m_pSpi1DmaEvent);

	}
	DMA_ClearFlag(DMA1_FLAG_GL3| DMA1_FLAG_TC3 | DMA1_FLAG_HT3 | DMA1_FLAG_TE3);  //Modify

	OSIntExit();
}
#endif //SPI1_DMA_TX
#endif //USE_SPI1_DMA

/****************************************************************************
* ��	�ƣ�SPI1 DMA�����ж�
* ��	�ܣ�DMA�����ж�
* ��ڲ�������
* ���ڲ�������
* ˵	����DMA�����жϷ���������ʾ�����������
****************************************************************************/
#ifdef USE_SPI1_DMA
#ifdef SPI1_DMA_RX
void DMA1_Channel2_IRQHandler(void)
{
	OSIntEnter();	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC2)) //Modify
	{
//		USART_DMACmd(g_Dcb[0].Usart, USART_DMAReq_Tx, DISABLE); //����DMA����
		DMA_Cmd(SPI1_Rx_DMA_Channel, DISABLE);	//����DMAͨ��                

		OSSemPost(m_pSpi1DmaEvent);


	}
	DMA_ClearFlag(DMA1_FLAG_GL2| DMA1_FLAG_TC2 | DMA1_FLAG_HT2 | DMA1_FLAG_TE2);  //Modify

	OSIntExit();
}
#endif //SPI1_DMA_RX
#endif //USE_SPI1_DMA



//SPI1 END***************************************************************

//
//
//
//
//
//
//
//
//
//
//
//
//
//
//


//SPI2 *******************************************************************

#define	SPI2_MODE			SPI_HARDWARE_MODE

//�źŶ���
//�˿�
#define SPI0SKIP	P0SKIP

//����ź�=================================================================
//ʱ���ź�                                                                                                                                                                                                                                                                                           
#define SPI2_SCK_PORT	GPIOB
#define SPI2_SCK			13

//�����ź�==================================================================
//��������
#define SPI2_MISO_PORT	GPIOB
#define SPI2_MISO		14

#define SPI2_TEST_PORT	GPIOB
#define SPI2_TEST		1
//�������
#define SPI2_MOSI_PORT	GPIOB
#define SPI2_MOSI		15
													



/****************************************************************************
* ��	�ƣ�SpiPortInit()
* ��	�ܣ����spi�˿ڳ�ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SPI2_CONFIGURATION__
void SPI2_Configuration(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
#ifdef USE_SPI2_DMA
	DMA_InitTypeDef DMA_InitStructure;
#endif
	/* Enable SPI and GPIO clocks */
#if	(SPI2_MODE == SPI_SOFTWARE_MODE)
	RCC_APB1PeriphClockCmd(SPI2_CLK, ENABLE);
#endif
	RCC_APB2PeriphClockCmd(SPI2_GPIO_CLK, ENABLE);
	
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
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	/* Enable SPI1 */ 
	SPI_Cmd(SPI2, ENABLE);
	
#ifdef USE_SPI2_DMA  

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

#ifdef SPI2_DMA_TX
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);//	SPI2 DMA��ʹ��

	DMA_DeInit(SPI1_Tx_DMA_Channel);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI1_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Spi1SendBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					   
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(SPI1_Tx_DMA_Channel, &DMA_InitStructure);	 //pDCB->Tx_DMA_Channel
	DMA_ITConfig(SPI1_Tx_DMA_Channel, DMA_IT_TC, ENABLE); //pDCB->Tx_DMA_Channel
	//DMA_Cmd(SPI1_Tx_DMA_Channel, ENABLE);						//ʼ��DMAͨ��
#endif	//SPI2_DMA_TX
#ifdef SPI2_DMA_RX
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);// SPI2 DMA��ʹ��

	DMA_DeInit(SPI1_Rx_DMA_Channel);  
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI1_DR_Base;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Spi1RecvBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(SPI1_Rx_DMA_Channel, &DMA_InitStructure);
	DMA_ITConfig(SPI1_Rx_DMA_Channel, DMA_IT_TC, ENABLE);    
	//DMA_Cmd(SPI1_Rx_DMA_Channel, ENABLE);						//ʼ��DMAͨ��
#endif	  //SPI2_DMA_RX
#endif	  //USE_SPI2_DMA
	
}
#endif //_SPI2_CONFIGURATION__

#ifdef _SPI_INIT_
void SpiPortInit()
{ 
    GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable SPI and GPIO clocks */
	RCC_APB1PeriphClockCmd(SPI2_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(SPI2_GPIO_CLK, ENABLE);
	
	GPIO_ResetBits(SPI2_GPIO, SPI2_PIN_SCK);
	/* Configure SPI pins: SCKand MOSI */
	GPIO_InitStructure.GPIO_Pin = SPI2_PIN_SCK | SPI2_PIN_MOSI;
#if	(SPI2_MODE == SPI_SOFTWARE_MODE)	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#else
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
#endif
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI2_GPIO, &GPIO_InitStructure);

	/* Configure SPI pins: MISO  */
	GPIO_InitStructure.GPIO_Pin = SPI2_PIN_MISO | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SPI2_GPIO, &GPIO_InitStructure);
}
#endif

void _nop()
{
	int i = 30;
	while(i--);
}
/****************************************************************************
* ��	�ƣ�spi_rw()
* ��	�ܣ�spi��д����
* ��ڲ�����dat ��д������
* ���ڲ���������������
* ˵	����L to H
****************************************************************************/
#ifdef  _SPI_RW_
//static unsigned char nSpiTemp;

#define SPI_ADC SPI2


unsigned char spi_rw(unsigned char byte)
{
#if	(SPI2_MODE == SPI_SOFTWARE_MODE)
	unsigned char nShiftOut, nShiftIn;
	unsigned char nCount = 8;

	nShiftOut = byte;
	while(nCount--)
	{
		if(nShiftOut & 0x80)	SetHi(SPI2_MOSI);	  
		else					SetLo(SPI2_MOSI);
		SetHi(SPI2_SCK);		

		nShiftOut <<= 1;
		SPI_NOP;
		
												
		nShiftIn <<= 1;
		if(GetSignal(SPI2_MISO) == 1)
		{
			nShiftIn |= 0x01;
		}
		
		SetLo(SPI2_SCK);
		
		SPI_NOP;
	}
	return nShiftIn;

#else

	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI_ADC, SPI_I2S_FLAG_TXE) == RESET);
	
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI_ADC, byte);
	
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI_ADC, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI_ADC);

#endif
}
#endif //_SPI_RW_


void CloseSPI2Comm(void)
{
#if	(SPI2_MODE == SPI_HARDWARE_MODE)
	/* Enable SPI1 */ 
	SPI_Cmd(SPI2, DISABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);
	DMA_Cmd(SPI2_Tx_DMA_Channel, DISABLE);
	DMA_Cmd(SPI2_Rx_DMA_Channel, DISABLE);
	/* Deinitialize the SPI2 */ 
	SPI_I2S_DeInit(SPI2);

#endif
}

//�ر�SPI��
void SpiPortShut()
{
#if	(SPI2_MODE == SPI_HARDWARE_MODE)
	CloseSPI2Comm();
#endif
}

//����SPI��
void SpiPortOpen()
{

#if	(SPI2_MODE == SPI_HARDWARE_MODE)

	SPI_InitTypeDef  SPI_InitStructure;
	/* SPI2 configuration ------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	/* Enable SPI1 */ 
	SPI_Cmd(SPI2, ENABLE);
#endif	
	
}


