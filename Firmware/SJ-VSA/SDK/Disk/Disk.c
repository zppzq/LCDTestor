/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: M25P80.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2011��3��23��
**����޸�����: 2011��3��23��
**��        ��: FlashԴ�ļ�����ʱ��Ƶ��Ϊ24.5MHz�²���û����
********************************************************************************************************/
#define		_FLASH_C_

#include "includes.h"
#include "Disk.h"


#define NAND_DMA_ACCESS


//���ݳ�������
#define DATALEN		NAND_PAGE_SIZE

//���̷��ʽṹ
#define DISK_READ	1
#define DISK_WRITE	2
#define DISK_ERASE	3
typedef struct _DiskParamS
{
	uint8	oper;
	uint8*  pData;
	uint32	addr;	
	uint32	len;
	OS_EVENT* pPendTask;
}DiskParamS;


//��Ϣ����
#define DISK_MSG_COUNT		16
OS_EVENT* pDiskMsg = NULL;
INT8U	nDiskErr;
void*	arrDiskMsg[16];

//���ݻ�����
uint16  nDataLen = DATALEN;	 
uint8 xdata WriteBuff[DATALEN];
uint8 xdata NandBuff[DATALEN];
uint8 xdata TxBuff[DATALEN];
uint8 xdata RxBuff[DATALEN];

#ifdef NAND_DMA_ACCESS
static OS_EVENT 	*m_pNandDMASyncEvent = NULL;	//DMAͬ���¼�
#endif


//�ڲ���������
void FlashWrite(uint8* pdat,uint32 nStartAddr,uint32 nlen);
void FlashRead(uint8* pdat,uint32 nStartAddr,uint32 nlen);
void FlashErase(uint32 Addr, uint32 nLen);
BOOL FlashBusy(void);
BOOL IsFlashOpen(void);
void FlashCheck(void);


/****************************************************************************
* ��	�ƣ�FlashInit()
* ��	�ܣ�M25P80��ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _DISK_INIT_
void DiskInit() reentrant
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


/****************************************************************************
* ��	�ƣ�FlashOpen()
* ��	�ܣ�M25P80��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void FlashOpen()
{
#ifdef NAND_DMA_ACCESS
	if(m_pNandDMASyncEvent == NULL) 
	{
		m_pNandDMASyncEvent = OSSemCreate(0);		 
	}
#endif

	if(pDiskMsg == NULL)
	{
		pDiskMsg = OSQCreate(arrDiskMsg, DISK_MSG_COUNT);
	}	
}

void DiskOpen(void)
{
	FlashOpen();
}

void DiskClose(void)
{

}

void DiskDisable(void)
{
	FSMC_NANDCmd(FSMC_Bank2_NAND, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, DISABLE);	
}

void DiskEnable(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);	
	FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
}


//�����д���ݣ����û��̵߳���
uint8 DiskWrite(OS_EVENT* pPendTask, uint8* pData, uint32 nAddr, uint32 nLen)
{
	uint8 nErr;
	DiskParamS DiskParam;
	
	DiskParam.pPendTask = pPendTask;
	DiskParam.pData = pData;
	DiskParam.addr = nAddr;
	DiskParam.len = nLen;
	DiskParam.oper = DISK_WRITE;

	//�����̹����̷߳�����Ϣ
	OSQPost(pDiskMsg, &DiskParam);

	//�ȴ����̹����̴߳������
	OSSemPend(pPendTask, 0, &nErr);
	
	//���ز������
	return nErr;
}

//�Ӵ��̶����ݣ����û��̵߳���
uint8 DiskRead(OS_EVENT* pPendTask, uint8* pData, uint32 nAddr, uint32 nLen)
{
	uint8 nErr;
	DiskParamS DiskParam;
	
	DiskParam.pPendTask = pPendTask;
	DiskParam.pData = pData;
	DiskParam.addr = nAddr;
	DiskParam.len = nLen;
	DiskParam.oper = DISK_READ;
	
	//�����̹����̷߳�����Ϣ
	OSQPost(pDiskMsg, &DiskParam);
		
	//�ȴ����̹����̴߳������
	OSSemPend(pPendTask, 0, &nErr);
		
	//���ز������
	return nErr;
}

//�����������ݣ����û��̵߳���
uint8 DiskErase(OS_EVENT* pPendTask, uint32 nAddr, uint32 nLen)
{
	uint8 nErr;
	DiskParamS DiskParam;
	
	DiskParam.pPendTask = pPendTask;
	DiskParam.pData = NULL;
	DiskParam.addr = nAddr;
	DiskParam.len = nLen;
	DiskParam.oper = DISK_ERASE;
	
	//�����̹����̷߳�����Ϣ
	OSQPost(pDiskMsg, &DiskParam);
		
	//�ȴ����̹����̴߳������
	OSSemPend(pPendTask, 0, &nErr);
		
	//���ز������
	return nErr;
}



void DiskProcess(void)
{
	DiskParamS* pDiskParam;

	pDiskParam = (DiskParamS*)OSQPend(pDiskMsg, 0, &nDiskErr);

	if(pDiskParam->oper == DISK_READ)
	{
		//������
		FlashRead(pDiskParam->pData, pDiskParam->addr, pDiskParam->len);
	}
	else if(pDiskParam->oper == DISK_WRITE)
	{
		//д����
		FlashWrite(pDiskParam->pData, pDiskParam->addr, pDiskParam->len);

	}
	else if(pDiskParam->oper == DISK_ERASE)
	{
		//д����
		FlashErase(pDiskParam->addr, pDiskParam->len);

	}

	//֪ͨ�û��̴߳�ȡ�������
	OSSemPost(pDiskParam->pPendTask);
}

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
static uint32 nPrePageIndex = 0xFFFFFFFF;
void FlashWrite(uint8* pdat, uint32 nStartAddr, uint32 nlen) reentrant
{
#ifdef NAND_DMA_ACCESS

	NAND_ADDRESS Address;
	u32 addressstatus = NAND_VALID_ADDRESS;
	u32 status = NAND_READY;
	u32 NumPageToWrite;
	u32 nWriteLen;
	int nPage;
	u8 nCommErr;	 

	if(nlen <= 0) return;

	nPage = nStartAddr / NAND_PAGE_SIZE;
	
	//���¶�����
	if(nPrePageIndex == nPage)
	{
		nPrePageIndex = 0xFFFFFFFF;
	}

	Address.Zone = 0;
	Address.Block = nPage / NAND_BLOCK_SIZE;
	Address.Page  = nPage % NAND_BLOCK_SIZE;
	
	
	NumPageToWrite =  (nlen % NAND_PAGE_SIZE != 0) ? (nlen  / NAND_PAGE_SIZE + 1) : (nlen / NAND_PAGE_SIZE);
	
	
	while((NumPageToWrite != 0x00) && (addressstatus == NAND_VALID_ADDRESS) && (status == NAND_READY))
	{
	    //����дFlash��ַ��ע�⣬��ʼ��ַʼ��ҳ���룡����
		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE0;
	
	    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0x00;
	    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = 0X00;
	    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_1st_CYCLE(ROW_ADDRESS);
	    *(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA) = ADDR_2nd_CYCLE(ROW_ADDRESS);

		//����ҳ�ڳ���
		nWriteLen = (nlen > NAND_PAGE_SIZE) ? NAND_PAGE_SIZE : nlen;
		
	    //����DMA����
		DMA1_Channel1->CMAR = (uint32)pdat;
		DMA1_Channel1->CNDTR = nWriteLen;
		DMA1_Channel1->CCR |= DMA_DIR_PeripheralDST; 	
		DMA_Cmd(DMA1_Channel1, ENABLE);	

		//�ȴ�DMA�������
	    OSSemPend(m_pNandDMASyncEvent, 0, &nCommErr);

		//����д����
		pdat += nWriteLen;
		nlen -= nWriteLen;
	    
		//
	    *(vu8 *)(Bank_NAND_ADDR | CMD_AREA) = NAND_CMD_WRITE_TRUE1;
	
	    //��ȡNandFlash״̬
		status = FSMC_NAND_GetStatus();
	    

	    if(status == NAND_READY)
	    {
			NumPageToWrite--;
			
			//������һҳ
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

/****************************************************************************
* ��	�ƣ�FlashRead()
* ��	�ܣ�M25P80������
* ��ڲ�����pdat ���������ݵ�ָ�룻nStartAddr flash�е���ʼ��ַ��
			nlen ���������ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
void FlashRead(uint8* pdat, uint32 nStartAddr, uint32 nlen) reentrant
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


	uint32 nPageIndex;
	uint32 nSaddr;
	uint32 nCopyLen;
	BOOL bUseBuff;
	NAND_ADDRESS Address;

	if(nlen <= 0) return;


	nPageIndex = nStartAddr / NAND_PAGE_SIZE;
	Address.Zone = 0;
	Address.Block = nPageIndex / NAND_BLOCK_SIZE;
	Address.Page  = nPageIndex % NAND_BLOCK_SIZE;

	nSaddr = nStartAddr % NAND_PAGE_SIZE;
	while(1)
	{
		bUseBuff = TRUE;
		
		if(nPrePageIndex != nPageIndex)
		{
			if((nSaddr == 0) && (nlen >= NAND_PAGE_SIZE))
			{
				FSMC_NAND_ReadSmallPage(pdat, Address, 1);
				pdat += NAND_PAGE_SIZE;
				nlen -= NAND_PAGE_SIZE;
				bUseBuff = FALSE;
			}
			else
			{
				FSMC_NAND_ReadSmallPage(NandBuff, Address, 1);
				nPrePageIndex = nPageIndex;

				bUseBuff = TRUE;	  				
			}				
		}

		if(bUseBuff == TRUE)
		{
			nCopyLen = NAND_PAGE_SIZE - nSaddr;
			if(nCopyLen > nlen) nCopyLen = nlen;
			
			memcpy(pdat, NandBuff + nSaddr, nCopyLen);
			pdat += nCopyLen;
			nlen -= nCopyLen;
		}

		if(nlen == 0) break;

		//ִ����һ��
		FSMC_NAND_AddressIncrement(&Address);
		nSaddr = 0;
		nPageIndex++;
	}


#endif
}

/****************************************************************************
* ��	�ƣ�FlashErase()
* ��	�ܣ�M25P80��������
* ��ڲ�����Select �������ͣ�Addr �������Ķ���ʼ��ַ
* ���ڲ�������
* ˵	������
****************************************************************************/
void FlashErase(uint32 Addr, uint32 nLen) reentrant
{
	uint32 nBlockSize, nEraseSize;
	NAND_ADDRESS addr;
	addr.Zone = 0;
	addr.Block = Addr / NAND_PAGE_SIZE / NAND_BLOCK_SIZE;
	addr.Page  = (Addr / NAND_PAGE_SIZE) % NAND_BLOCK_SIZE;

	nBlockSize = NAND_BLOCK_SIZE * NAND_PAGE_SIZE;
	do
	{
	    FSMC_NAND_EraseBlock(addr);
		nEraseSize = addr.Block * nBlockSize - Addr;
		nLen -= nEraseSize;
		Addr = 0;
		addr.Block++;
	}while(nLen > nEraseSize);

}

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
