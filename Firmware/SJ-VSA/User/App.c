#include "includes.h"
#include "APP.h"
#include "ADS1246.h"
#include "Sensor.h"

#define  TASK_STK_SIZE                  128      /* Size of each task's stacks (# of WORDs)            */
OS_STK        TaskStartStk[TASK_STK_SIZE];
OS_STK        Task1Stk[TASK_STK_SIZE];
OS_STK        Task2Stk[TASK_STK_SIZE];
OS_STK        PWPStk[TASK_STK_SIZE];
OS_STK        Task3Stk[TASK_STK_SIZE];
OS_STK        PowerStk[TASK_STK_SIZE];
OS_STK        PowerLEDStk[TASK_STK_SIZE];
OS_STK        MCOStk[TASK_STK_SIZE];

OS_EVENT *g_PowerOnEvent;
uint8 g_nPowerOff = 0;

void UART_Print(char *pStr)
{
	char *p;
	for(p = pStr; *p; p++)
	{
		USART_SendData(USART1, (uint8_t)*p);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){};
	} 
}

void Power_IO_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//开机IO
	GPIO_ResetBits(POWER_SWITCH_PORT, POWER_SWITCH_PIN);
	GPIO_InitStructure.GPIO_Pin = POWER_SWITCH_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//关机检测IO
	GPIO_InitStructure.GPIO_Pin = POWER_SWITCH_CHECK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(POWER_SWITCH_CHECK_PORT, &GPIO_InitStructure);
}

#define BUFFER_SIZE         0x400
void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLenght, uint32_t Offset)
{
  uint16_t IndexTmp = 0;

  /* Put in global buffer same values */
  for (IndexTmp = 0; IndexTmp < BufferLenght; IndexTmp++ )
  {
    pBuffer[IndexTmp] = IndexTmp + Offset;
  }
}

uint8_t TxBuffer[BUFFER_SIZE], RxBuffer[BUFFER_SIZE];
uint32 status= 0;
void  Task1 (void *pdata) 
{
	NAND_IDTypeDef NAND_ID;
	NAND_ADDRESS WriteReadAddr;
	char pStr[64];
	u8 nFlag;
	__IO uint32_t PageNumber = 2;

	pdata = pdata;                                         /* Prevent compiler warning                 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	FSMC_NAND_Init();
    /* NAND memory address to write to */
    WriteReadAddr.Zone = 0x00;
    WriteReadAddr.Block = 0x00;
    WriteReadAddr.Page = 0x00;

    /* Erase the NAND first Block */
    status = FSMC_NAND_EraseBlock(WriteReadAddr);
    Fill_Buffer(TxBuffer, BUFFER_SIZE , 0x66);

    status = FSMC_NAND_WriteSmallPage(TxBuffer, WriteReadAddr, PageNumber);

    status = FSMC_NAND_ReadSmallPage (RxBuffer, WriteReadAddr, PageNumber);
	for(;;) 
	{
	   	FSMC_NAND_ReadID(&NAND_ID);
		sprintf(pStr, "Nand Flash ID = %02X,%02X,%02X,%02X  ",
			NAND_ID.Maker_ID, NAND_ID.Device_ID,
			NAND_ID.Third_ID, NAND_ID.Fourth_ID);
//		UART_Print(pStr);
		nFlag = 1;

		if ((NAND_ID.Maker_ID == 0xEC) && (NAND_ID.Device_ID == 0xF1)
			&& (NAND_ID.Third_ID == 0x80) && (NAND_ID.Fourth_ID == 0x15))
		{
//			UART_Print("Type = K9F1G08U0A\n\r");
			nFlag = 2;
		}
		else if ((NAND_ID.Maker_ID == 0xEC) && (NAND_ID.Device_ID == 0xF1)
			&& (NAND_ID.Third_ID == 0x00) && (NAND_ID.Fourth_ID == 0x95))
		{
//			UART_Print("Type = K9F1G08U0B\n\r");
			nFlag = 3;
		}
		else if ((NAND_ID.Maker_ID == 0xEC) && (NAND_ID.Device_ID == 0xF1)
			&& (NAND_ID.Fourth_ID == 0x15))
		{
//			UART_Print("Type = K9F1G08U0M\n\r");
			nFlag = 6;
		}
		else if ((NAND_ID.Maker_ID == 0xAD) && (NAND_ID.Device_ID == 0xF1)
			&& (NAND_ID.Third_ID == 0x80) && (NAND_ID.Fourth_ID == 0x1D))
		{
//			UART_Print("Type = HY27UF081G2A\n\r");
			nFlag = 4;
		}
		else
		{
//			UART_Print("Type = Unknow\n\r");
			nFlag = 5;
		}
		OSTimeDly(500);
	}
}
void  CommHostProc (void *pdata) 
{
//	char pStr[] = "ABCabc12345";
	char pStr[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
	pdata = pdata;                                         /* Prevent compiler warning                 */

	UART_Slave_Configuration();
	USART_SendData(USART_485_SLAVE, 0x0C);
	while (USART_GetFlagStatus(USART_485_SLAVE, USART_FLAG_TC) == RESET);

	for(;;)
	{
		UART_Slave_Send(pStr, sizeof(pStr));
//		OSTimeDly(100);
	}
// 	Comm1PortInit();
// 	Comm1VariInit();
// 	OpenComm1();
// 
// 	for(;;)
// 	{
// 		SendComm1(pStr, 6);
// 		OSTimeDly(100);	/*
// 		if(CBSlaveApsFsm() == TRUE)
// 		{
// 			while(IsComm1SendEnd() == FALSE);
// 		}  */
// 	}
}

void PowerWatchProc(void *pdata)
{
	u32 nVoltage;
	char pStr[32], *p;
	pdata = pdata;
	
	for(;;)
	{
		nVoltage = GetADCValue(ADC_Channel_11);
		sprintf(pStr, "ADC_Value:%d\r\n", nVoltage);
		for(p = pStr; *p; p++)
		{
			USART_SendData(USART1, (uint8_t)*p);
			while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){};
		}
		if(nVoltage < POWER_UV_LIMIT)
		{
			//输入电压过低,应进行保护处理
		}
		OSTimeDly(100);
	}
}

void SPICommProc(void *pdata)
{
// 	__IO uint32_t SFlashID;
// 	char buf[] = "this is a test string!";
// 	char readBuf[32];
// 	uint16 nBufSize = sizeof(buf);
// 	pdata = pdata;
// 	/* Initialize the SPI FLASH driver */	
// 	SPI1_DMA_Configuration();
// //	NVIC_SPI1_Configuration();
// 	SPI_FLASH_Init();
// 
// 	/* Get SPI Flash ID */
// 	SFlashID = SPI_FLASH_ReadID();
// 	SPI_FLASH_BufferWrite(buf, 0x00001000, nBufSize);
// 	SPI_FLASH_BufferRead(readBuf, 0x00001000, nBufSize);
// 	pdata = pdata;

	pdata = pdata;
	ADS1246PortInit();
	ADS1246PortOpen();
	ADS1246VariInit();
	SpiPortInit();
	SpiPortOpen();
	//传感器初始化
//	SensorInit();
	//任务循环
	for(;;)
	{
		//传感器处理	
		SensorProcess();
	}
	
}

void PowerProc(void *pdata)
{
	uint8 nStatus, nTemp;
	pdata = pdata;
	Power_IO_Config();
	OSTimeDly(OS_TICKS_PER_SEC);
	GPIO_SetBits(POWER_SWITCH_PORT, POWER_SWITCH_PIN);	//打开开关
	GPIO_ResetBits(POWER_LED_PORT, POWER_LED_PIN);
	while(!GPIO_ReadInputDataBit(POWER_SWITCH_CHECK_PORT, POWER_SWITCH_CHECK_PIN));	//等待开关弹起
	OSSemPost(g_PowerOnEvent);
	while(1)
	{
		nStatus = GPIO_ReadInputDataBit(POWER_SWITCH_CHECK_PORT, POWER_SWITCH_CHECK_PIN);
		if(nStatus == 0)
		{
			nTemp = nStatus;
			OSTimeDly(OS_TICKS_PER_SEC);
			nStatus = GPIO_ReadInputDataBit(POWER_SWITCH_CHECK_PORT, POWER_SWITCH_CHECK_PIN);
			if(nTemp == nStatus)
			{
				GPIO_ResetBits(POWER_SWITCH_PORT, POWER_SWITCH_PIN);
				g_nPowerOff = 1;
			}
			else
				nTemp = 0x0F;
		}
	}
}

void PowerLEDProc(void *pdata)
{
	INT8U Err;
	GPIO_InitTypeDef GPIO_InitStructure;
	pdata = pdata;
	GPIO_SetBits(POWER_LED_PORT, POWER_LED_PIN);
	GPIO_InitStructure.GPIO_Pin = POWER_LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(POWER_LED_PORT, &GPIO_InitStructure);
	OSSemPend(g_PowerOnEvent, 0, &Err);
	for(;;)
	{
		if(g_nPowerOff)
		{
			GPIO_SetBits(POWER_LED_PORT, POWER_LED_PIN);
			GPIO_ResetBits(POWER_SWITCH_PORT, POWER_SWITCH_PIN);
			while(1);
		}
		else
		{
			GPIO_ResetBits(POWER_LED_PORT, POWER_LED_PIN);
			OSTimeDly(OS_TICKS_PER_SEC);
			GPIO_SetBits(POWER_LED_PORT, POWER_LED_PIN);
			OSTimeDly(OS_TICKS_PER_SEC);
		}
	}
}

void MCO_Proc(void *pdata)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_ClocksTypeDef RCC_ClockFreq;
	pdata = pdata;
	RCC_GetClocksFreq(&RCC_ClockFreq);
	/* Output HSE clock on MCO pin ---------------------------------------------*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	while(1)
	{
		RCC_MCOConfig(RCC_MCO_PLLCLK_Div2);
		OSTimeDly(OS_TICKS_PER_SEC*10);
		RCC_MCOConfig(RCC_MCO_HSE);
		OSTimeDly(OS_TICKS_PER_SEC*10);
		RCC_MCOConfig(RCC_MCO_PLLCLK_Div2);
		OSTimeDly(OS_TICKS_PER_SEC*10);
	}
}

void  TaskStart (void *pdata)
{
	pdata = pdata;                                         /* Prevent compiler warning                 */

	BSP_Init();  
	g_PowerOnEvent = OSSemCreate(0);
//	OSTaskCreate(Task1, (void *)0, &Task1Stk[TASK_STK_SIZE - 1], 1);
//	OSTaskCreate(MCO_Proc, (void *)0, &MCOStk[TASK_STK_SIZE - 1], 1);
//	OSTaskCreate(CommHostProc, (void *)0, &Task2Stk[TASK_STK_SIZE - 1], 2);
//	OSTaskCreate(SPICommProc, (void *)0, &Task3Stk[TASK_STK_SIZE - 1], 4);
//	OSTaskCreate(PowerWatchProc, (void *)0, &PWPStk[TASK_STK_SIZE - 1], 3);
	OSTaskCreate(PowerLEDProc, (void *)0, &PowerLEDStk[TASK_STK_SIZE - 1], 5);
	OSTaskCreate(PowerProc, (void *)0, &PowerStk[TASK_STK_SIZE - 1], 10);
	OSTaskDel(0);      
}

int main (void) 
{
    OSInit();    
    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
  for(;;) 
  {
  }
}											  

