/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210B-EVAL Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : Brian Nagel
*********************************************************************************************************/

#define  BSP_GLOBALS
#include <includes.h>


static volatile ErrorStatus HSEStartUpStatus = SUCCESS;

static void SysTickConfig(void);
void GPIO_Config(void);
void SPI_Config(void);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void ADC_Configuration(void);
void USART_Configuration(void);

/**
  * @brief  Sets System clock frequency to 72MHz and configure HCLK, PCLK2 
  *   and PCLK1 prescalers. 
  * @param  None
  * @retval None
  */
void  BSP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	SystemInit();
	/* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/   
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();
	
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);
	
	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	
	if (HSEStartUpStatus == SUCCESS)
	{
		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 
		
		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1); 
		
		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);
		
		/* ADCCLK = PCLK2/4 */
		RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
		
		#ifdef STM32F10X_CL
		/* Configure PLLs *********************************************************/
		/* PLL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
		RCC_PREDIV2Config(RCC_PREDIV2_Div5);
		RCC_PLL2Config(RCC_PLL2Mul_8);
		
		/* Enable PLL2 */
		RCC_PLL2Cmd(ENABLE);
		
		/* Wait till PLL2 is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
		{}
		
		/* PLL configuration: PLLCLK = (PLL2 / 5) * 9 = 72 MHz */ 
		RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div5);
		RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);
		#else
		/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
		#endif
		
		/* Enable PLL */ 
		RCC_PLLCmd(ENABLE);
		
		/* Wait till PLL is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		
		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		
		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}
	else
	{ /* If HSE fails to start-up, the application will have wrong clock configuration.
		   User can add here some code to deal with this error */    
		
		/* Go to infinite loop */
		while (1)
		{
		}
	}
	
	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
	  | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG
	  | RCC_APB2Periph_AFIO, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;		//始能NAND Flash,232芯片,485芯片的电源
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_SetBits(GPIOF, GPIO_Pin_3);
	/* Enable peripheral clocks --------------------------------------------------*/
	/* Enable DMA1 clock */
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	/* Enable ADC1 and GPIOC clock */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);

	/* TIM2 clock enable */
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* 打开USART部件的时钟 */
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
/*------------------- Resources Initialization -----------------------------*/
  /* GPIO Configuration */
//	GPIO_Config();

  /* USART Configuration */
// 	USART_Configuration();

  /* Interrupt Configuration */
//	NVIC_Configuration();

  /* ADC Configuration */
//	ADC_Configuration();

  /* Configure the systick */
	SysTickConfig();         /* Initialize the uC/OS-II tick interrupt */

    /* Enable the FSMC that share a pin w/ I2C1 (LBAR) */
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);


//	FSMC_NAND_Init();
}

//void  BSP_Init(void)
//{
//    SystemInit();
//	RCC_Configuration();
//	SysClockInit();

//    InterruptConfig();
   /* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
/*  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
         | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG
         | RCC_APB2Periph_AFIO, ENABLE); */


 // STM3210E_LCD_Init();
   /* Clear the LCD */
 // LCD_Clear(White);
//  GPIO_Config();
//  SPI_Config();
//}

void  SysTickConfig(void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    u32         cnts;

	RCC_GetClocksFreq(&rcc_clocks);
    cnts = (u32)rcc_clocks.HCLK_Frequency/100;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config(cnts);   //OS_TICKS_PER_SEC*(SystemFrequency/1000)
}

#define ADC_PIN		GPIO_Pin_4
void GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure PC.04 (ADC Channel14) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = ADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* 将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_2;		//UART1 & UART2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		//UART3
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		//UART4
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* 将USART Rx的GPIO配置为浮空输入模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;		//UART3
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;		//UART4
	GPIO_Init(GPIOC, &GPIO_InitStructure);
} 
/*
char TxBuffer2[4];
char TxBuffer3[4];
int TxBufferSize2;
int TxBufferSize3;*/
/**
  * @brief  Configures the DMA.
  * @param  None
  * @retval None
  *//*
void DMA_Configuration(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  // USART_GPRS TX DMA1 Channel (triggered by USART_GPRS Tx event) Config 
  DMA_DeInit(USART_GPRS_Tx_DMA_Channel);  
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_GPRS_DR_Base;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBuffer2;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = TxBufferSize2;  
  DMA_Init(USART_GPRS_Tx_DMA_Channel, &DMA_InitStructure);
  
  // USART_GPRS RX DMA1 Channel (triggered by USART_GPRS Rx event) Config 
  DMA_DeInit(USART_GPRS_Rx_DMA_Channel);  
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_GPRS_DR_Base;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RxBuffer2;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = TxBufferSize1;
  DMA_Init(USART_GPRS_Rx_DMA_Channel, &DMA_InitStructure); 
   
  // USART_485_EP TX DMA2 Channel (triggered by USART_GPRS Tx event) Config 
  DMA_DeInit(USART_485_EP_Tx_DMA_Channel);  
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_485_EP_DR_Base;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBuffer3;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = TxBufferSize3;  
  DMA_Init(USART_485_EP_Tx_DMA_Channel, &DMA_InitStructure);
  
  // USART_485_EP RX DMA1 Channel (triggered by USART_GPRS Rx event) Config 
  DMA_DeInit(USART_485_EP_Rx_DMA_Channel);  
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART_485_EP_DR_Base;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RxBuffer3;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = TxBufferSize3;
  DMA_Init(USART_485_EP_Rx_DMA_Channel, &DMA_InitStructure);  
}	 */


/*******************************************************************************
	函数名：USART_Configuration
	输  入:
	输  出:
	功能说明：
	初始化串口硬件设备，未启用中断。
	配置步骤：
	(1)打开GPIO和USART的时钟
	(2)设置USART两个管脚GPIO模式
	(3)配置USART数据格式、波特率等参数
	(4)最后使能USART功能
*/
// void USART_Configuration(void)
// {
// 	USART_InitTypeDef USART_InitStructure; 
// 
// 	/* 配置USART1参数  (调试输出用)
// 	    - BaudRate = 115200 baud
// 	    - Word Length = 8 Bits
// 	    - One Stop Bit
// 	    - No parity
// 	    - Hardware flow control disabled (RTS and CTS signals)
// 	    - Receive and transmit enabled
// 	*/
// 	USART_InitStructure.USART_BaudRate = 115200;
// 	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
// 	USART_InitStructure.USART_StopBits = USART_StopBits_1;
// 	USART_InitStructure.USART_Parity = USART_Parity_No;
// 	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
// 	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
// 	USART_Init(USART1, &USART_InitStructure);
// 
// 	/* 使能 USART， 配置完毕 */
// 	USART_Cmd(USART1, ENABLE);
// 
// 
// 	/* 配置USART3参数	 (GPRS端口)
// 	    - BaudRate = 115200 baud
// 	    - Word Length = 8 Bits
// 	    - One Stop Bit
// 	    - No parity
// 	    - Hardware flow control disabled (RTS and CTS signals)
// 	    - Receive and transmit enabled
// 	*/
// 	USART_InitStructure.USART_BaudRate = 115200;
// 	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
// 	USART_InitStructure.USART_StopBits = USART_StopBits_1;
// 	USART_InitStructure.USART_Parity = USART_Parity_No;
// 	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
// 	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
// 	USART_Init(USART3, &USART_InitStructure);
// 
// 	/* 使能 USART， 配置完毕 */
// 	USART_Cmd(USART3, ENABLE);
// 
// 	/* 配置USART4参数	 (485底层端口----传感器端)
// 	    - BaudRate = 9600 baud
// 	    - Word Length = 8 Bits
// 	    - One Stop Bit
// 	    - No parity
// 	    - Hardware flow control disabled (RTS and CTS signals)
// 	    - Receive and transmit enabled
// 	*/
// 	USART_InitStructure.USART_BaudRate = 9600;
// 	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
// 	USART_InitStructure.USART_StopBits = USART_StopBits_1;
// 	USART_InitStructure.USART_Parity = USART_Parity_No;
// 	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
// 	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
// 	USART_Init(UART4, &USART_InitStructure);
// 
// 	/* 使能 USART， 配置完毕 */
// 	USART_Cmd(UART4, ENABLE);
// 
// 	/* CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
// 		如下语句解决第1个字节无法正确发送出去的问题 */
// 	USART_ClearFlag(USART1, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */
// 	USART_ClearFlag(USART2, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */
// 	USART_ClearFlag(USART3, USART_FLAG_TC);     /* 清发送外城标志，Transmission Complete flag */
// 	USART_ClearFlag(UART4, USART_FLAG_TC);      /* 清发送外城标志，Transmission Complete flag */
//   /* Enable USART_GPRS DMA Rx and TX request */
//   USART_DMACmd(USART_GPRS, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
// 
//   /* Enable USART_485_HOST TX DMA1 Channel */
//   DMA_Cmd(USART_485_HOST_Tx_DMA_Channel, ENABLE);
//   /* Enable USART_485_HOST RX DMA1 Channel */
//   DMA_Cmd(USART_485_HOST_Rx_DMA_Channel, ENABLE);
// 
//   /* Enable USART_GPRS TX DMA1 Channel */
//   DMA_Cmd(USART_GPRS_Tx_DMA_Channel, ENABLE);
//   /* Enable USART_GPRS RX DMA1 Channel */
//   DMA_Cmd(USART_GPRS_Rx_DMA_Channel, ENABLE);
// 
//   /* Enable the USART_485_HOST */
//   USART_Cmd(USART_485_HOST, ENABLE);
//   /* Enable the USART_GPRS */
//   USART_Cmd(USART_GPRS, ENABLE);
// }

void NVIC_Configuration(void)
{
   NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable DMA1 channel7 IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the TIM2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_Init(&NVIC_InitStructure);
}

