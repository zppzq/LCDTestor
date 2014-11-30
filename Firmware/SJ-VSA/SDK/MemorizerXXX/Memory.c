/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: M25P80.c
**创   建   人: 杨承凯
**创 建 日  期: 2006年8月6日
**最后修改日期: 2007年4月8日
**描        述: Flash源文件，在时钟频率为24.5MHz下测试没问题
********************************************************************************************************/
#define		_FLASH_C_

#include "includes.h"
#include "Memory.h"


//micro defines**************************************************************
#define		WREN		0x06
#define		WRDI		0x04
#define		RDSR		0x05
#define		WRSR		0x01
#define		READ		0x03
#define		FAST_READ	0x0B
#define		PP			0x02

#define		DP			0xB9
#define		RES			0xAB


#define 	FLASH_NOP 		_nop_();_nop_();_nop_()


#define NAND_DMA_ACCESS

//数据长度设置
#define	MOSTADDR	((uint32)0x40000000)

//数据缓冲区
uint16  nDataLen = DATALEN;

uint8 xdata WriteBuff[DATALEN];
uint8 xdata NandBuff[DATALEN];
uint8 xdata TxBuff[DATALEN];
uint8 xdata RxBuff[DATALEN];

#ifdef NAND_DMA_ACCESS
static OS_EVENT 	*m_pNandDMASyncEvent = NULL;	//DMA同步事件
#endif


//Flash读写地址
uint32  FlashAddr;
uint32  ErrorAddr = 0xFFFFFFFF;

//信号定义**********************************************************************************************
//写使能端口
//#define	FlashWrEn_PORT	PORT(4)
//#define	FlashWrEn		BIT(5)

#ifdef _FLASH_M25P80_H_
//片选
#if		(PCB_VERSION == SJ_RFQC_SC_4D_3_1)		//3.1改线板

#define	FlashEn_PORT	PORT(2)
#define	FlashEn			BIT(6)

#elif	(PCB_VERSION == SJ_RFQC_SC_4D_3_3)		//3.2板

#define	FlashEn_PORT	PORT(2)
#define	FlashEn			BIT(4)

#endif

//电源
#define	FlashPw_PORT	PORT(2)
#define	FlashPw			BIT(3)

#endif

#ifdef _FLASH_M25P80_H_
//使能Flash
#define FlashEnable() SetLo(FlashEn)

//关闭Flash
#define FlashDisable() SetHi(FlashEn)
#else
//使能Flash
#define FlashEnable() SetLo(FlashEn)

//关闭Flash
#define FlashDisable() SetHi(FlashEn)
#endif

/****************************************************************************
* 名	称：FlashInit()
* 功	能：M25P80初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _FLASH_INIT_
void FlashInit() reentrant
{
#ifdef NAND_DMA_ACCESS
	DMA_InitTypeDef  DMA_InitStructure;
#endif

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	FSMC_NAND_Init();

#ifdef NAND_DMA_ACCESS
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(Bank_NAND_ADDR | DATA_AREA);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)NandBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = DATALEN;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);    

#endif

}
#endif	 //_FLASH_INIT_

#if 0
//端口引脚控制
void FlashPortShut() reentrant
{
#ifdef _FLASH_M25P80_H_
	MakeOpenDrain(FlashEn);
 	SetLo(FlashEn);
#endif	 //_FLASH_M25P80_H_
	//
}

void FlashPortOpen() reentrant
{
#ifdef _FLASH_M25P80_H_
 	SetHi(FlashEn);
	MakePushPull(FlashEn);
#endif	 //_FLASH_M25P80_H_
	//
}
#endif

/****************************************************************************
* 名	称：FlashOpen()
* 功	能：M25P80打开
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_FALSE_OPEN_
void Fill_Buffer(uint8* buf, uint32 len, uint8 ch)
{
	int i;
	buf[0] = 0;
	for(i=1; i<len; i++)
	{
		buf[i] = ch;
	}
}

uint16 nTimeGone1, nTimeGone2;
uint32 nTimeOver1, nTimeOver2;
uint8* p1, *p2;
extern uint32 Timer3Counter;
void FlashOpen()
{
#if 1
	uint8 nFlag;
	uint32 i, status = 0;
	NAND_ADDRESS WriteReadAddr;
	NAND_IDTypeDef NAND_ID;
   	FSMC_NAND_ReadID(&NAND_ID);

#ifdef NAND_DMA_ACCESS
	if(m_pNandDMASyncEvent == NULL) 
		m_pNandDMASyncEvent = OSSemCreate(0);
#endif

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
    /* NAND memory address to write to */
    WriteReadAddr.Zone = 0x00;
    WriteReadAddr.Block = 0x01;
    WriteReadAddr.Page = 0x00;

    /* Erase the NAND first Block */
//    status = FSMC_NAND_EraseBlock(WriteReadAddr);
	FlashErase(0x00020000);
	nTimeOver1 = Timer3Counter;
	nTimeGone1 = TIM5->CNT;	  
	FlashRead(RxBuff, 0x00020000, NAND_PAGE_SIZE);		
	nTimeOver2 = Timer3Counter;
	nTimeGone2 = TIM5->CNT;	 


	nTimeOver2 = Timer3Counter;
	nTimeGone2 = TIM5->CNT;	 
 	memset(RxBuff, 0, DATALEN);
 	memcpy(RxBuff, TxBuff, DATALEN);
	nTimeOver1 = Timer3Counter;
	nTimeGone1 = TIM5->CNT;
	

    Fill_Buffer(TxBuff, DATALEN , 0x01);

	nTimeOver2 = Timer3Counter;
	nTimeGone2 = TIM5->CNT;
	memset(TxBuff, 0x55, DATALEN);
	FlashWrite(TxBuff, 0x00020000, NAND_PAGE_SIZE);

	FlashRead(RxBuff, 0x00020000, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

	memset(TxBuff, 0x50, DATALEN);
	FlashWrite(TxBuff, 0x00020000, NAND_PAGE_SIZE);

	nTimeOver1 = Timer3Counter;
	nTimeGone1 = TIM5->CNT;
	
	
	FlashRead(RxBuff, 0x00020000, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

    Fill_Buffer(TxBuff, DATALEN , 0xA0);
	FlashWrite(TxBuff, 0x00020000+NAND_PAGE_SIZE, NAND_PAGE_SIZE);
	FlashRead(RxBuff, 0x00020000+NAND_PAGE_SIZE, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

	FlashErase(0x02000000);
	FlashRead(RxBuff, 0x02000000, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

    Fill_Buffer(TxBuff, DATALEN , 0x20);
	FlashWrite(TxBuff, 0x02000000, NAND_PAGE_SIZE);
	FlashRead(RxBuff, 0x02000000, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

    Fill_Buffer(TxBuff, DATALEN , 0x2A);
	FlashWrite(TxBuff, 0x02000000+NAND_PAGE_SIZE, NAND_PAGE_SIZE);
	FlashRead(RxBuff, 0x02000000+NAND_PAGE_SIZE, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

	FlashErase(0x04000000);
	FlashRead(RxBuff, 0x04000000, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

    Fill_Buffer(TxBuff, DATALEN , 0x40);
	FlashWrite(TxBuff, 0x04000000, NAND_PAGE_SIZE);
	FlashRead(RxBuff, 0x04000000, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

    Fill_Buffer(TxBuff, DATALEN , 0x4A);
	FlashWrite(TxBuff, 0x04000000+NAND_PAGE_SIZE, NAND_PAGE_SIZE);
	FlashRead(RxBuff, 0x04000000+NAND_PAGE_SIZE, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

	FlashErase(0x06000000);
	FlashRead(RxBuff, 0x06000000, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

    Fill_Buffer(TxBuff, DATALEN , 0x40);
	FlashWrite(TxBuff, 0x06000000, NAND_PAGE_SIZE);
	FlashRead(RxBuff, 0x06000000, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

    Fill_Buffer(TxBuff, DATALEN , 0x4A);
	FlashWrite(TxBuff, 0x06000000+NAND_PAGE_SIZE, NAND_PAGE_SIZE);
	FlashRead(RxBuff, 0x06000000+NAND_PAGE_SIZE, NAND_PAGE_SIZE);
 	memset(RxBuff, 0, DATALEN);

#endif
}
#endif
/****************************************************************************
* 名	称：FlashClose()
* 功	能：M25P80关闭 
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_FALSE_CLOSE_
void FlashClose() reentrant
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, DISABLE);
//
}
#endif
/****************************************************************************
* 名	称：IsFlashOpen()
* 功	能：M25P80是否打开
* 入口参数：无
* 出口参数：TRUE：打开。
* 说	明：无
****************************************************************************/
#ifdef	_IS_FALSE_OPEN_
BOOL IsFlashOpen() reentrant
{
#ifdef _FLASH_M25P80_H_
	if(1 == GetSignal(FlashPw)) return TRUE;
	else return FALSE;						
#endif	 //_FLASH_M25P80_H_
	//
	return 0;
}
#endif


/****************************************************************************
* 名	称：DMA1_Channel4_IRQHandler		DMA1_Channel4_IRQn
* 功	能：DMA发送中断
* 入口参数：无
* 出口参数：无
* 说	明：DMA发送中断服务函数，表示发送数据完毕
****************************************************************************/
#ifdef NAND_DMA_ACCESS
void DMA1_Channel1_IRQHandler(void)
{
	OSIntEnter();	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC1)) //Modify
	{
		DMA_Cmd(DMA1_Channel1, DISABLE);	//除能DMA通道                
		OSSemPost(m_pNandDMASyncEvent);
	}
	DMA_ClearFlag(DMA1_FLAG_GL1| DMA1_FLAG_TC1 | DMA1_FLAG_HT1 | DMA1_FLAG_TE1);  //Modify

	OSIntExit();
}
#endif


/****************************************************************************
* 名	称：FlashWrite()
* 功	能：M25P80写函数
* 入口参数：pdat 待写入数据的指针；nStartAddr flash中的起始地址；
			nlen 待写入数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
static int nPrePageIndex = -1;
#ifdef _FLASH_WRITE_
void FlashWrite(uint8* pdat, uint32 nStartAddr, uint16 nlen) reentrant
{
#ifdef NAND_DMA_ACCESS
	NAND_ADDRESS Address;
	u32 numpagewritten = 0x00, addressstatus = NAND_VALID_ADDRESS;
	u32 status = NAND_READY;
	u32 NumPageToWrite;
	int nPage;
	u8 nCommErr;	 

	if(nlen <= 0)
		return;

	nPage = nStartAddr / NAND_PAGE_SIZE;
	if(nPrePageIndex == nPage)
		nPrePageIndex = -1;

	Address.Zone = 0;
	Address.Block = nPage / NAND_BLOCK_SIZE;
	Address.Page  = nPage % NAND_BLOCK_SIZE;
	NumPageToWrite =  nlen % NAND_PAGE_SIZE ? nlen  / NAND_PAGE_SIZE + 1 : NAND_PAGE_SIZE / nlen;
	while((NumPageToWrite != 0x00) && (addressstatus == NAND_VALID_ADDRESS) && (status == NAND_READY))
	{
	    /* Page write command and address */
	    *(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE0;
	
	    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
	    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0X00;
	    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

	    /* Write data */
		DMA1_Channel1->CMAR = (uint32)pdat;
		DMA1_Channel1->CNDTR = NAND_PAGE_SIZE;
		DMA1_Channel1->CCR |= DMA_DIR_PeripheralDST; 	
		DMA_Cmd(DMA1_Channel1, ENABLE);	

	    OSSemPend(m_pNandDMASyncEvent, 0, &nCommErr);
	    
	    *(vu8 *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE_TRUE1;
	
	    /* Check status for successful operation */
	    status = FSMC_NAND_GetStatus();
	    
	    if(status == NAND_READY)
	    {
			numpagewritten++;
			
			NumPageToWrite--;
			
			/* Calculate Next small page Address */
			addressstatus = FSMC_NAND_AddressIncrement(&Address);    
	    }    
	}
  
#else	

	int i;//, nIndex;
	NAND_ADDRESS addr;
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif
	addr.Zone = 0;
	addr.Block = nStartAddr / NAND_PAGE_SIZE / NAND_BLOCK_SIZE;
	addr.Page  = (nStartAddr / NAND_PAGE_SIZE) % NAND_BLOCK_SIZE;
	if(nlen < NAND_PAGE_SIZE)
	{
		for(i=0; i < nlen; i++)   //填充Buf
			NandBuff[i] = pdat[i];
		OS_ENTER_CRITICAL(); 
		FSMC_NAND_WriteSmallPage(NandBuff, addr, 1);	     //写一页 
		OS_EXIT_CRITICAL();
	}
	else
	{
		OS_ENTER_CRITICAL(); 
		FSMC_NAND_WriteSmallPage(pdat, addr, 1);	     //写一页 
		OS_EXIT_CRITICAL();
	}
#endif

}
#endif


/****************************************************************************
* 名	称：FlashRead()
* 功	能：M25P80读函数
* 入口参数：pdat 待读出数据的指针；nStartAddr flash中的起始地址；
			nlen 待读出数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _FLASH_READ_
void FlashRead(uint8* pdat, uint32 nStartAddr, uint16 nlen) reentrant
{
//#ifdef NAND_DMA_ACCESS
#if 0
	NAND_ADDRESS Address;
	u32 addressstatus = NAND_VALID_ADDRESS;
	u32 NumPageToRead;
	u8 nCommErr;

	if(nlen <= 0)
		return;

	Address.Zone = 0;
	Address.Block = nStartAddr / NAND_PAGE_SIZE / NAND_BLOCK_SIZE;
	Address.Page  = (nStartAddr / NAND_PAGE_SIZE) % NAND_BLOCK_SIZE;
	NumPageToRead =  nlen % NAND_PAGE_SIZE ? nlen  / NAND_PAGE_SIZE + 1 : NAND_PAGE_SIZE / nlen; 

	while((NumPageToRead != 0x0) && (addressstatus == NAND_VALID_ADDRESS))
	{	   
		/* Page Read command and page address */
		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_A;
		
		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0X00;
		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);
		
		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_AREA_TRUE1;
		
		 /* 必须等待，否则读出数据异常 */
		 while( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 );
		
		/* Get Data into Buffer */    
		DMA1_Channel1->CMAR = (uint32)pdat;
		DMA1_Channel1->CNDTR = NAND_PAGE_SIZE;
		DMA1_Channel1->CCR |= DMA_DIR_PeripheralSRC; 	
		DMA_Cmd(DMA1_Channel1, ENABLE);	

	    OSSemPend(m_pNandDMASyncEvent, 0, &nCommErr);
    
		NumPageToRead--;

		/* Calculate page address */           			 
		addressstatus = FSMC_NAND_AddressIncrement(&Address);
	}

#else
	int nPageIndex, nSaddr, nDataIndex = 0;
	NAND_ADDRESS Address;
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif
	nPageIndex = nStartAddr / NAND_PAGE_SIZE;
	Address.Zone = 0;
	Address.Block = nPageIndex / NAND_BLOCK_SIZE;
	Address.Page  = nPageIndex % NAND_BLOCK_SIZE;

	nSaddr = nStartAddr%NAND_PAGE_SIZE;
	while(nDataIndex < nlen)
	{
		if(nPrePageIndex != nPageIndex)
		{
			//OS_ENTER_CRITICAL(); 
			FSMC_NAND_ReadSmallPage(NandBuff, Address, 1);
			//OS_EXIT_CRITICAL();
			nPrePageIndex = nPageIndex;
		}

		nTimeOver1 = Timer3Counter;
		nTimeGone1 = TIM5->CNT;

		nTimeOver1 = Timer3Counter;
		nTimeGone1 = TIM5->CNT;

		for(; nSaddr < NAND_PAGE_SIZE && nDataIndex < nlen; nSaddr++)   //填充Buf
			pdat[nDataIndex++] = NandBuff[nSaddr];

		nTimeOver2 = Timer3Counter;
		nTimeGone2 = TIM5->CNT;
		
		nSaddr %= NAND_PAGE_SIZE;
		FSMC_NAND_AddressIncrement(&Address);
		nPageIndex++;
	}
//	OS_ENTER_CRITICAL(); 
//	FSMC_NAND_ReadSmallPage(pdat, addr, 1);
//	OS_EXIT_CRITICAL();
#endif
}
#endif

/****************************************************************************
* 名	称：FlashErase()
* 功	能：M25P80擦除函数
* 入口参数：Select 擦除类型；Addr 待擦除的段起始地址
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _FLASH_ERASE_
void FlashErase(uint32 Addr) reentrant
{
	int status;
#ifdef _FLASH_M25P80_H_
	uint8 i;
	
	//获取地址
	uint8* pByte = (uint8*)(&Addr);

	//检查是否忙
	while(FlashBusy()) 
	{
		OSTimeDly(1);
	}
	
	EA = 0;EA = 0;
	//写使能
	FlashEnable();
	spi_rw(WREN);
	FlashDisable();
	//擦除
	pByte++;
	FlashEnable();
	spi_rw(Select);
	if(Select == SE)
	{
		for(i=0;i<3;i++) spi_rw(*(pByte+i));
	}
	FlashDisable();
	EA = 1;EA = 1;
#endif	 //_FLASH_M25P80_H_
	NAND_ADDRESS addr;
	addr.Zone = 0;
	addr.Block = Addr / NAND_PAGE_SIZE / NAND_BLOCK_SIZE;
	addr.Page  = (Addr / NAND_PAGE_SIZE) % NAND_BLOCK_SIZE;
    /* Erase the NAND first Block */
    status = FSMC_NAND_EraseBlock(addr);

}
#endif

/****************************************************************************
* 名	称：FlashBusy()
* 功	能：M25P80忙判断函数
* 入口参数：无
* 出口参数：1 忙；0 空闲
* 说	明：无
****************************************************************************/
#ifdef _FLASH_BUSY_
BOOL FlashBusy()  reentrant
{
	BOOL bBusy;
#ifdef _FLASH_M25P80_H_
	BOOL bBusy;
	uint8 Result;

	EA = 0;EA = 0;
	
	//读状态寄存器
	FlashEnable();
	spi_rw(RDSR);
	Result = spi_rw(0xFF);
	FlashDisable();
	EA = 1;EA = 1;

	//获取状态位
	bBusy = Result & 0x01;
	return bBusy;
#endif	 //_FLASH_M25P80_H_
	bBusy = FSMC_NAND_ReadStatus() == NAND_BUSY ? 0 : 1;
	return bBusy;
}
#endif


#ifdef	_FALSH_TEST_

//************************************************************
//功能描述：M25P80测试程序，将数据写入M25P80，然后读出，
//			看读写数据是否一致
//测试平台：桥测采集器V3.0

//说    明：如果读写有异常，首先检查读写时钟的频率是否过高
//************************************************************

void FlashCheck() reentrant
{
	uint16 i, nEndCheck = 0;

	//清空Flash
	FlashErase(0);
												 
	//擦除某一扇区
	//FlashErase(SE,0x00000);

	//将数据区清零
	for(i=0;i<nDataLen;i++) WriteBuff[i] = i;

	//开始循环读写数据
	FlashAddr = 0;
	while(1)
	{	
		//地址保护，防止读写越界
		if(FlashAddr+nDataLen > MOSTADDR)
		{
			nEndCheck = 1;
			nDataLen = MOSTADDR - FlashAddr;
		}
		
		//将数据写入Flash
		FlashWrite(WriteBuff,FlashAddr,nDataLen);

		//将Flash数据读出
		FlashRead(NandBuff,FlashAddr,nDataLen);
	
		//校验数据
		for(i=0;i<nDataLen;i++)
		{
			if(WriteBuff[i] != NandBuff[i]) break;
		}

		//判断
		if(i == nDataLen)
		{
			if(nEdnCheck)
				break;
			//更新待写数据
			for(i=0;i<nDataLen;i++) WriteBuff[i] += 1;
			//Flash读写地址更新
			FlashAddr += nDataLen;
		}
		else
		{
			ErrorAddr = FlashAddr;
			while(1)					//停止读写
			{
				//读写出错的地方
				_nop_();				//For break point
			}
		}
		//判断是否整个Flash已写完
		if(FlashAddr >= MOSTADDR)
		{
			while(1)					//停止读写
			{
				_nop_();				//For break point
			}
		}
	}
//#endif	 //_FLASH_M25P80_H_
	//
}
#endif
