/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H_
#define __SPI_H_

#ifdef		_SPI_C_
#define		SPI_EXT
#else
#define		SPI_EXT		extern
#endif

#define _SPI_INIT_
#define _SPI_RW_

#define SPI_NOP	_nop()	

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Define the STM32F10x hardware depending on the used evaluation board */
  #define SPI1_CLK               RCC_APB2Periph_SPI1
  #define SPI1_GPIO              GPIOA
  #define SPI1_GPIO_CLK          RCC_APB2Periph_GPIOA  
  #define SPI1_PIN_NSS           GPIO_Pin_4		//PC4
  #define SPI1_PIN_SCK           GPIO_Pin_5		//PA5
  #define SPI1_PIN_MISO          GPIO_Pin_6		//PA6
  #define SPI1_PIN_MOSI          GPIO_Pin_7		//PA7
  #define SPI1_DMA                DMA1
  #define SPI1_DMA_CLK            RCC_AHBPeriph_DMA1  
  #define SPI1_Rx_DMA_Channel     DMA1_Channel2
  #define SPI1_Rx_DMA_FLAG        DMA1_FLAG_TC2
  #define SPI1_Tx_DMA_Channel     DMA1_Channel3
  #define SPI1_Tx_DMA_FLAG        DMA1_FLAG_TC3  
  #define SPI1_DR_Base            0x4001300C
  
  #define SPI2_CLK                RCC_APB1Periph_SPI2
  #define SPI2_GPIO               GPIOB
  #define SPI2_GPIO_CLK           RCC_APB2Periph_GPIOB 
  #define SPI2_PIN_NSS            GPIO_Pin_12
  #define SPI2_PIN_SCK            GPIO_Pin_13
  #define SPI2_PIN_MISO           GPIO_Pin_14
  #define SPI2_PIN_MOSI           GPIO_Pin_15 
  #define SPI2_DMA                DMA1
  #define SPI2_DMA_CLK            RCC_AHBPeriph_DMA1  
  #define SPI2_Rx_DMA_Channel     DMA1_Channel4
  #define SPI2_Rx_DMA_FLAG        DMA1_FLAG_TC4
  #define SPI2_Tx_DMA_Channel     DMA1_Channel5
  #define SPI2_Tx_DMA_FLAG        DMA1_FLAG_TC5  
  #define SPI2_DR_Base            0x4000380C
          
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void SpiPortInit(void);
unsigned char spi_rw(unsigned char dat);
//¶Ë¿ÚÒý½Å¿ØÖÆ
void SpiPortShut(void);
void SpiPortOpen(void);
void _nop(void);
void SPI1_Configuration(void);
void SPI1_Close(void);
uint8_t SPI1_RwByte(uint8_t byte);
void SPI1_Close(void);
void SPI1_Disable(void);
void SPI1_Enable(void);

void SPI1_Send(uint8* pData, uint32 nLen);
void SPI1_Recv(uint8* pData, uint32 nLen);
uint8 SPI1_WaitDMA(void);

#endif /* __SPI_H_ */ 
