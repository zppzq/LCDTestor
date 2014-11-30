#include "includes.h"
#include "Bsp\LowPower.h"

void RTC_Alarm_IntConfig(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	
	/* Configure EXTI Line17(RTC Alarm) to generate an interrupt on rising edge */
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

/**
  * @brief  Configures the RTC.
  * @param  None
  * @retval None
  */
void RTC_Configuration(void)
{
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	
	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);
	
	/* Reset Backup Domain */
	BKP_DeInit();
	return ;
	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}
	
	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();
	
	/* Set the RTC time base to 1s */
	RTC_SetPrescaler(32767);  
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	
	/* 设置闹钟定时为5秒 */
//	RTC_SetAlarm(5);
	/* Wait until last write operation on RTC registers has finished */
//	RTC_WaitForLastTask();

	/* Enable the RTC Alarm interrupt */
	RTC_ITConfig(RTC_IT_ALR, ENABLE);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

void CLKConfig_Enable(void)
{
	ErrorStatus HSEStartUpStatus;
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);
	
	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	
	if(HSEStartUpStatus == SUCCESS)
	{

#ifdef STM32F10X_CL
	    /* Enable PLL2 */ 
	    RCC_PLL2Cmd(ENABLE);
	
	    /* Wait till PLL2 is ready */
	    while(RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
	    {
	    }
#endif

	    /* Enable PLL */ 
	    RCC_PLLCmd(ENABLE);
	
	    /* Wait till PLL is ready */
	    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	    {
	    }
	
	    /* Select PLL as system clock source */
	    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	
	    /* Wait till PLL is used as system clock source */
	    while(RCC_GetSYSCLKSource() != 0x08)
	    {
	    }
  	}
}

void RTCAlarm_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
	{
	    /* Clear EXTI line17 pending bit */
	    EXTI_ClearITPendingBit(EXTI_Line17);
	
	    /* Check if the Wake-Up flag is set */
	    if(PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)
	    {
	      /* Clear Wake Up flag */
	      PWR_ClearFlag(PWR_FLAG_WU);
	    }
	
	    /* Wait until last write operation on RTC registers has finished */
	    RTC_WaitForLastTask();   
	    /* Clear RTC Alarm interrupt pending bit */
	    RTC_ClearITPendingBit(RTC_IT_ALR);
	    /* Wait until last write operation on RTC registers has finished */
	    RTC_WaitForLastTask();
	}
}


/*****************************************************************************************************************
* 名	称：MCUSleep()
* 功	能：MCU进入休眠模式，休眠时间为WAKE_INTERVAL (ms)
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void MCUSleep() reentrant
{
	/* Wait till RTC Second event occurs */
	RTC_ClearFlag(RTC_FLAG_SEC);
	while(RTC_GetFlagStatus(RTC_FLAG_SEC) == RESET);
	
	//设置闹钟定时为5秒
	RTC_SetAlarm(RTC_GetCounter()+ 5);
	// 等待上一次RTC操作完成
	RTC_WaitForLastTask();

	//程序进入休眠模式===============================
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

	//--------------------------------------------------------------------
	// Device Sleeping until the next smaRTClock alarm
	//--------------------------------------------------------------------

	//系统被唤醒,始能时钟
	CLKConfig_Enable();
}


/*****************************************************************************************************************
* 名	称：MCUSleep()
* 功	能：MCU进入休眠模式，休眠时间为WAKE_INTERVAL (ms)
* 入口参数：无
* 出口参数：无
* 说	明：电源控制信号线保持推挽，无线控制信号保持高电平
*****************************************************************************************************************/

//uint32 g_RCC_AHB;
//uint32 g_RCC_APB2;
//uint32 g_RCC_APB1;
void MCUEnterSleep() reentrant
{
	PowerLightOff();
	SigLightOff();
	Stm32ADCDisable();
	SpiPortShut();
	SPI1_Disable();
	ADClose();
	ADS1246PortShut();

	#ifdef _SI4432_H_
	Si4432PortShut();
	#endif

	//关闭Nand Flash
	DiskDisable();

	//关通信电源
	GPIO_ResetBits(GPIOF, GPIO_Pin_3);

	#ifdef _USE_COMM2_
	CommPortShut(2);
	#endif

	//关端口
	GPIODisable();
	
	//关时钟
	RCC_PLLCmd(DISABLE);	
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

}

/*****************************************************************************************************************
* 名	称：MCUSleep()
* 功	能：MCU进入休眠模式，休眠时间为WAKE_INTERVAL (ms)
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/

void MCUExitSleep() reentrant
{
	//开端口
	GPIOEnable();

	#ifdef _USE_COMM2_
	CommPortOpen(2);
	#endif

	//开通信电源
	GPIO_SetBits(GPIOF, GPIO_Pin_3); 
	
	//开Nand Flash
	DiskEnable();

	#ifdef _SI4432_H_
	Si4432PortOpen();
	#endif
				
	ADS1246PortOpen(); 
	Stm32ADCEnable();
	
	//最后开启SPI
	SPI1_Enable();
	SpiPortOpen();
}

