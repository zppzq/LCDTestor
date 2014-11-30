/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: M25P80.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2006��8��6��
**����޸�����: 2007��4��8��
**��        ��: FlashԴ�ļ�����ʱ��Ƶ��Ϊ24.5MHz�²���û����
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

//���ݳ�������
#define	MOSTADDR	((uint32)0x40000000)

//���ݻ�����
uint16  nDataLen = DATALEN;

uint8 xdata WriteBuff[DATALEN];
uint8 xdata NandBuff[DATALEN];
uint8 xdata TxBuff[DATALEN];
uint8 xdata RxBuff[DATALEN];

#ifdef NAND_DMA_ACCESS
static OS_EVENT 	*m_pNandDMASyncEvent = NULL;	//DMAͬ���¼�
#endif


//Flash��д��ַ
uint32  FlashAddr;
uint32  ErrorAddr = 0xFFFFFFFF;

//�źŶ���**********************************************************************************************
//дʹ�ܶ˿�
//#define	FlashWrEn_PORT	PORT(4)
//#define	FlashWrEn		BIT(5)

#ifdef _FLASH_M25P80_H_
//Ƭѡ
#if		(PCB_VERSION == SJ_RFQC_SC_4D_3_1)		//3.1���߰�

#define	FlashEn_PORT	PORT(2)
#define	FlashEn			BIT(6)

#elif	(PCB_VERSION == SJ_RFQC_SC_4D_3_3)		//3.2��

#define	FlashEn_PORT	PORT(2)
#define	FlashEn			BIT(4)

#endif

//��Դ
#define	FlashPw_PORT	PORT(2)
#define	FlashPw			BIT(3)

#endif

#ifdef _FLASH_M25P80_H_
//ʹ��Flash
#define FlashEnable() SetLo(FlashEn)

//�ر�Flash
#define FlashDisable() SetHi(FlashEn)
#else
//ʹ��Flash
#define FlashEnable() SetLo(FlashEn)

//�ر�Flash
#define FlashDisable() SetHi(FlashEn)
#endif

/****************************************************************************
* ��	�ƣ�FlashInit()
* ��	�ܣ�M25P80��ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
//�˿����ſ���
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
* ��	�ƣ�FlashOpen()
* ��	�ܣ�M25P80��
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�FlashClose()
* ��	�ܣ�M25P80�ر� 
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	_FALSE_CLOSE_
void FlashClose() reentrant
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, DISABLE);
//
}
#endif
/****************************************************************************
* ��	�ƣ�IsFlashOpen()
* ��	�ܣ�M25P80�Ƿ��
* ��ڲ�������
* ���ڲ�����TRUE���򿪡�
* ˵	������
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
* ��	�ƣ�DMA1_Channel4_IRQHandler		DMA1_Channel4_IRQn
* ��	�ܣ�DMA�����ж�
* ��ڲ�������
* ���ڲ�������
* ˵	����DMA�����жϷ���������ʾ�����������
****************************************************************************/
#ifdef NAND_DMA_ACCESS
void DMA1_Channel1_IRQHandler(void)
{
	OSIntEnter();	  
	if(DMA_GetFlagStatus(DMA1_FLAG_TC1)) //Modify
	{
		DMA_Cmd(DMA1_Channel1, DISABLE);	//����DMAͨ��                
		OSSemPost(m_pNandDMASyncEvent);
	}
	DMA_ClearFlag(DMA1_FLAG_GL1| DMA1_FLAG_TC1 | DMA1_FLAG_HT1 | DMA1_FLAG_TE1);  //Modify

	OSIntExit();
}
#endif


/****************************************************************************
* ��	�ƣ�FlashWrite()
* ��	�ܣ�M25P80д����
* ��ڲ�����pdat ��д�����ݵ�ָ�룻nStartAddr flash�е���ʼ��ַ��
			nlen ��д�����ݵĳ���
* ���ڲ�������
* ˵	������
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
		for(i=0; i < nlen; i++)   //���Buf
			NandBuff[i] = pdat[i];
		OS_ENTER_CRITICAL(); 
		FSMC_NAND_WriteSmallPage(NandBuff, addr, 1);	     //дһҳ 
		OS_EXIT_CRITICAL();
	}
	else
	{
		OS_ENTER_CRITICAL(); 
		FSMC_NAND_WriteSmallPage(pdat, addr, 1);	     //дһҳ 
		OS_EXIT_CRITICAL();
	}
#endif

}
#endif


/****************************************************************************
* ��	�ƣ�FlashRead()
* ��	�ܣ�M25P80������
* ��ڲ�����pdat ���������ݵ�ָ�룻nStartAddr flash�е���ʼ��ַ��
			nlen ���������ݵĳ���
* ���ڲ�������
* ˵	������
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
		
		 /* ����ȴ���������������쳣 */
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

		for(; nSaddr < NAND_PAGE_SIZE && nDataIndex < nlen; nSaddr++)   //���Buf
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
* ��	�ƣ�FlashErase()
* ��	�ܣ�M25P80��������
* ��ڲ�����Select �������ͣ�Addr �������Ķ���ʼ��ַ
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _FLASH_ERASE_
void FlashErase(uint32 Addr) reentrant
{
	int status;
#ifdef _FLASH_M25P80_H_
	uint8 i;
	
	//��ȡ��ַ
	uint8* pByte = (uint8*)(&Addr);

	//����Ƿ�æ
	while(FlashBusy()) 
	{
		OSTimeDly(1);
	}
	
	EA = 0;EA = 0;
	//дʹ��
	FlashEnable();
	spi_rw(WREN);
	FlashDisable();
	//����
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
* ��	�ƣ�FlashBusy()
* ��	�ܣ�M25P80æ�жϺ���
* ��ڲ�������
* ���ڲ�����1 æ��0 ����
* ˵	������
****************************************************************************/
#ifdef _FLASH_BUSY_
BOOL FlashBusy()  reentrant
{
	BOOL bBusy;
#ifdef _FLASH_M25P80_H_
	BOOL bBusy;
	uint8 Result;

	EA = 0;EA = 0;
	
	//��״̬�Ĵ���
	FlashEnable();
	spi_rw(RDSR);
	Result = spi_rw(0xFF);
	FlashDisable();
	EA = 1;EA = 1;

	//��ȡ״̬λ
	bBusy = Result & 0x01;
	return bBusy;
#endif	 //_FLASH_M25P80_H_
	bBusy = FSMC_NAND_ReadStatus() == NAND_BUSY ? 0 : 1;
	return bBusy;
}
#endif


#ifdef	_FALSH_TEST_

//************************************************************
//����������M25P80���Գ��򣬽�����д��M25P80��Ȼ�������
//			����д�����Ƿ�һ��
//����ƽ̨���Ų�ɼ���V3.0

//˵    ���������д���쳣�����ȼ���дʱ�ӵ�Ƶ���Ƿ����
//************************************************************

void FlashCheck() reentrant
{
	uint16 i, nEndCheck = 0;

	//���Flash
	FlashErase(0);
												 
	//����ĳһ����
	//FlashErase(SE,0x00000);

	//������������
	for(i=0;i<nDataLen;i++) WriteBuff[i] = i;

	//��ʼѭ����д����
	FlashAddr = 0;
	while(1)
	{	
		//��ַ��������ֹ��дԽ��
		if(FlashAddr+nDataLen > MOSTADDR)
		{
			nEndCheck = 1;
			nDataLen = MOSTADDR - FlashAddr;
		}
		
		//������д��Flash
		FlashWrite(WriteBuff,FlashAddr,nDataLen);

		//��Flash���ݶ���
		FlashRead(NandBuff,FlashAddr,nDataLen);
	
		//У������
		for(i=0;i<nDataLen;i++)
		{
			if(WriteBuff[i] != NandBuff[i]) break;
		}

		//�ж�
		if(i == nDataLen)
		{
			if(nEdnCheck)
				break;
			//���´�д����
			for(i=0;i<nDataLen;i++) WriteBuff[i] += 1;
			//Flash��д��ַ����
			FlashAddr += nDataLen;
		}
		else
		{
			ErrorAddr = FlashAddr;
			while(1)					//ֹͣ��д
			{
				//��д����ĵط�
				_nop_();				//For break point
			}
		}
		//�ж��Ƿ�����Flash��д��
		if(FlashAddr >= MOSTADDR)
		{
			while(1)					//ֹͣ��д
			{
				_nop_();				//For break point
			}
		}
	}
//#endif	 //_FLASH_M25P80_H_
	//
}
#endif
