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

#include "includes.h"
//#include "bsp\PowerKey.h"
#include "Bsp\LowPower.h"

#include <os_cfg.h>
#define  BSP_GLOBALS


static volatile ErrorStatus HSEStartUpStatus = SUCCESS;

static void SysTickConfig(void);
void GPIO_Config(void);
void SPI_Config(void);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void ADC_Configuration(void);
void USART_Configuration(void);



uint16 _htons(uint16 n)
{
	char nTmp;

	nTmp = ((char*)&n)[0];
	((char*)&n)[0] = ((char*)&n)[1];
	((char*)&n)[1] = nTmp;

	return n;
}

uint32 _htonl(uint32 n)
{
	char nTmp;

	nTmp = ((char*)&n)[0];
	((char*)&n)[0] = ((char*)&n)[3];
	((char*)&n)[3] = nTmp;

	nTmp = ((char*)&n)[1];
	((char*)&n)[1] = ((char*)&n)[2];
	((char*)&n)[2] = nTmp;	

	return n;
}

float _htonf(float f)
{
	char nTmp;

	nTmp = ((char*)&f)[0];
	((char*)&f)[0] = ((char*)&f)[3];
	((char*)&f)[3] = nTmp;

	nTmp = ((char*)&f)[1];
	((char*)&f)[1] = ((char*)&f)[2];
	((char*)&f)[2] = nTmp;	

	return f;
}


//硬件初始化
void  BSP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	
	//系统时钟初始化
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
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		
		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);
		
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
		/* PLLCLK = 12MHz * 6 = 72 MHz */
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

	//实时时钟中断初始化
//	RTC_Alarm_IntConfig();
	
	//实时时钟初始化
//	RTC_Configuration();
	
	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
	  | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG
	  | RCC_APB2Periph_AFIO, ENABLE);

	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;		//始能NAND Flash,232芯片,485芯片的电源
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_SetBits(GPIOF, GPIO_Pin_3); 
	
	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);
	
	/* Configure the Priority Group to 2 bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
//****************************************************************************************************************
//硬件端口初始化
//****************************************************************************************************************
//电源按键初始化
#ifdef	_POWER_KEY_H_
	PowerKeyInit();
#endif

//实时时钟初始化
#ifdef	_RTC_H_
	smaRTCInit();
#endif

//存储器端口初始化函数
#ifdef	_XMEM_H_
	XMemPortInit();
#endif

//磁盘初始化
#ifdef _DISK_H_
	DiskInit();
#endif

//串口1初始化函数
#ifdef	_SERI1_H_
	Comm1PortInit();
#endif

//串口2初始化函数
#ifdef	_SERI2_H_
	Comm2PortInit();	 
#endif

//无线初始化函数(使用外部中断0)
#ifdef	_WIRELESS_INTERFACE_H_
	WirelessPortInit();
#endif

//存储器初始化
#ifdef _MEMORIZER_INTERFACE_H_
	MemPorInit();	
#endif

//AD&DA初始化函数
#ifdef _AD_DA_INTERFACE_H_
	ADPortInit();
	ADPowerPortInit();
#endif

//PCA0初始化函数
#ifdef _PCA0_H_
	PCA0Init();
#endif

//液晶驱动初始化函数
#ifdef _LCDDriver_H_
	LcdDriverInit();
#endif

//PS2端口初始化函数
#ifdef _PS2_H_
	Ps2Init();
#endif

//USB0端口初始化函数
#ifdef _USB_INCLUDE_H_
	Usb0Init();
#endif

//SD卡端口初始化函数(使用了硬件SPI)
#ifdef _SD_MMC_H_
	SdInterfaceInit();
#endif 

//SPI初始化函数
#ifdef	_SPI_H_
	SpiPortInit();
#endif


	NVIC_Configuration();
	SysTickConfig(); 
}

/*****************************************************************************************************************
* 名	称：BspVariInit()
* 功	能：板级参数初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：根据实际用到的板级驱动用户可自裁减，在参数初始化中创建信号量
*****************************************************************************************************************/
void BspVariInit() reentrant
{

//****************************************************************************************************************
//程序全局变量初始化
//****************************************************************************************************************
#ifdef	_POWER_KEY_H_
	PowerKeyVarInit();
#endif
#ifdef	_SERI1_H_
	Comm1VariInit();
#endif

#ifdef	_SERI2_H_
	Comm2VariInit();
#endif

#ifdef _SERIAL_H_
	UsartVariInit();
#endif

#ifdef	_WIRELESS_INTERFACE_H_
	WirelessVariInit();	
#endif
#ifdef _PS2_H_
	Ps2VariInit(); 
#endif

#ifdef _KEY_DRIVER_H_
	KeyInit(); 
#endif

#ifdef _USB_INCLUDE_H_
	Usb0VariInit();
#endif

#ifdef _AD_DA_INTERFACE_H_
	ADVariInit();			
#endif

#ifdef _LIGHTS_H_
	LightsInit();
#endif
	
}


void  BSP_IntDisAll (void)
{
  // CPU_IntDis();
}

void  SysTickConfig(void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    u32         cnts;

	RCC_GetClocksFreq(&rcc_clocks);
    cnts = (u32)rcc_clocks.HCLK_Frequency/OS_TICKS_PER_SEC;
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

void GPIODisable(void)
{
	/* Disable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
	  | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG
	  | RCC_APB2Periph_AFIO, DISABLE);
}

void GPIOEnable(void)
{
	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
	  | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG
	  | RCC_APB2Periph_AFIO, ENABLE);
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Enable the USARTz Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the RTC Alarm Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable EXTI2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_Init(&NVIC_InitStructure);	
	
	/* Enable the EXIT0 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable DMA1 channel7 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable DMA1 channel7 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable DMA1 channel7 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable DMA1 channel7 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the TIM2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable the TIM3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable the TIM4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable the TIM5 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}


void SPI_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	//GPIOA Periph clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	//SPI1 Periph clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

	//Configure SPI1 pins: SCK, MISO and MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	//Configure PG11 pin: TP_CS pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_Init(GPIOG,&GPIO_InitStructure);

	//Configure PB5 pin: TP_BUSY pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;     //上拉输入
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	/* Configure PC5 as input floating For TP_IRQ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	// SPI1 Config
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   //SPI_NSS_Hard
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	// SPI1 enable
	SPI_Cmd(SPI1,ENABLE);
}

