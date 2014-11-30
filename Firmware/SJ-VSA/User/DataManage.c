/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: DataManage.c
**创   建   人: 杨承凯
**创 建 日  期: 2008年7月21日
**最后修改日期: 2008年7月21日
**描        述: 采集器数据管理源文件
********************************************************************************************************/
#define		_DATA_MANAGE_C_

#include "includes.h"
#include "DataManage.h"
#include "DataAcq.h"

//宏参数定义
#define DATA_CONFIG_KEY				0x12F4095A

//事件定义
#define DATA_ENENT_NONE				0x00
#define DATA_EVENT_ERASE			0x01
#define DATA_ENENT_PARAM_STORE		0x02

//终端参数
STerminalParam g_TerminalParam;

//磁盘访问信号量
static OS_EVENT* pDataManageDiskSem;

//数据管理信号
static OS_EVENT* pDataManageEvent;
static uint8 m_nDataManageTask;

//空数据段管理
static uint32 m_nDiskBlockErased;							//已经擦除过的段
static uint32 m_nDiskBlockToBeErase;						//需要擦除的段

//数据管理初始化
void DataManageInit(void)
{
	m_nDiskBlockErased = 0;
	m_nDiskBlockErased = 0;

	m_nDataManageTask = DATA_ENENT_NONE;

	pDataManageDiskSem = OSSemCreate(0);  
	pDataManageEvent = OSSemCreate(0);	   
}

/****************************************************************************
* 名	称：ParamLoad()
* 功	能：读取参数
* 入口参数：pdat - 待读取数据的指针；nStartAddr - flash中的虚拟起始地址；
			nlen - 待读取数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _DATA_MNG_PARAM_CONFIGED_
void ParamLoad(void)
{
	//读参数
	DiskRead(pDataManageDiskSem, (uint8*)(&g_TerminalParam), 0, sizeof(g_TerminalParam));
}
#endif
																		 
/****************************************************************************
* 名	称：ParamStore()
* 功	能：存储参数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _DATA_MNG_PARAM_CONFIGED_
void ParamStore(void)
{
	//写之前要擦除相应的块
	DiskErase(pDataManageDiskSem, 0, 0);

	//设置标志
	g_TerminalParam.nDataCfgKey = DATA_CONFIG_KEY;

	//写参数
	DiskWrite(pDataManageDiskSem, (uint8*)(&g_TerminalParam), 0, sizeof(g_TerminalParam));
}											   
#endif

/****************************************************************************
* 名	称：ParamSetDefault()
* 功	能：使用默认参数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ParamSetDefault(void)
{
	uint8 i;
	for(i = 0; i < DATA_ACQ_COUNT; i++)
	{
		g_TerminalParam.arrADStep[i] = 0.298f;
		g_TerminalParam.arrZeroVol[i] = 0;		
	}
}

/****************************************************************************
* 名	称：IsDataConfiged()
* 功	能：判断数据有没有被配置过
* 入口参数：无
* 出口参数：TRUE - 已配置，FALSE - 未配置
* 说	明：无
****************************************************************************/
#ifdef _DATA_MNG_PARAM_CONFIGED_
BOOL IsParamEmpty()
{
	if(g_TerminalParam.nDataCfgKey == DATA_CONFIG_KEY) return FALSE;	    
	
	return TRUE;               
}
#endif



/****************************************************************************
* 名	称：DataLoad()
* 功	能：读取数据
* 入口参数：pdat - 待读取数据的指针；nStartAddr - flash中的虚拟起始地址；
			nlen - 待读取数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _DATA_MNG_DATA_LOAD_
BOOL MemDataLoad(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint32 nlen,uint8 nChannelID) reentrant
{
	uint32 nAddrOffset;

	//参数检查
	if((nStartAddr + nlen) > 0x08000000) return FALSE;

	//确定各端点的地址偏移量
	switch(nChannelID)
	{
		case 0 : nAddrOffset = 0x00020000; break;
		case 1 : nAddrOffset = 0x02020000; break;
		case 2 : nAddrOffset = 0x04020000; break;
		case 3 : nAddrOffset = 0x06020000; break;
		default : break;
	}

	//读取数据
	DiskRead(pSem, pdat, nStartAddr + nAddrOffset, nlen);

	return TRUE;
}
#endif


/****************************************************************************
* 名	称：DataStore()
* 功	能：存储数据
* 入口参数：pdat - 待写入数据的指针；nStartAddr - flash中的虚拟起始地址；
			nlen - 待写入数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _DATA_MNG_DATA_STORE_
BOOL MemDataStore(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint32 nlen,uint8 nChannelID) reentrant
{
	uint32 nAddrOffset;

	//参数检查
	if((nStartAddr + nlen) > 0x08000000) return FALSE;

	//确定各端点的地址偏移量
	switch(nChannelID)
	{
		case 0 : nAddrOffset = 0x00020000; break;
		case 1 : nAddrOffset = 0x02020000; break;
		case 2 : nAddrOffset = 0x04020000; break;				 
		case 3 : nAddrOffset = 0x06020000; break;
		default : break;
	}
	
	//存储数据
	DiskWrite(pSem, pdat, nStartAddr + nAddrOffset, nlen);

	return TRUE;								   
}											   
#endif

/****************************************************************************
* 名	称：MemorizerErase()
* 功	能：擦除整个存储器
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
#ifdef _DATA_MNG_MEMORIZER_ERASE_
void MemorizerErase() reentrant
{
	uint32 i;
	uint32 j;

	for(i=1; i<NAND_ZONE_SIZE; i++)
	{
		j = NAND_BLOCK_SIZE * NAND_ZONE_SIZE * i;
		DiskErase(pDataManageDiskSem, j, 0);
	}
}
#endif

/****************************************************************************
* 名	称：MemorizerEraseToSect()
* 功	能：擦除选择的存储端
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
#ifdef _DATA_MNG_MEMORIZER_ERASE_TO_SECT_  
void MemorizerEraseToSect(uint32 nSectStart, uint32 nSectEnd) reentrant
{
	uint32 i;
	uint32 nAddrEaseBase;
	uint32 nAddrEase; 	
	uint8 nEpIndex;

	//跳过第一块，每个段的第一块用来存储参数而非数据
	nSectStart++;
	nSectEnd++;
	
	//确定要擦除的扇区长度(暂时保留最后3个Block不用)
	nSectEnd = (nSectEnd > 252) ? 252 : nSectEnd;

	//参数检查
	if(nSectStart >= nSectEnd) return;

	//擦除每个通道的相应扇区
	for(nEpIndex = 0; nEpIndex < DATA_ACQ_COUNT; nEpIndex++)
	{
		//计算每个通道的数据基址
		nAddrEaseBase = nEpIndex;
		nAddrEaseBase <<= 25;
		
		//擦除选择的区域
		for(i = nSectStart; i < nSectEnd; i++)
		{
			nAddrEase = i * NAND_PAGE_SIZE * NAND_BLOCK_SIZE;
			nAddrEase += nAddrEaseBase;
			DiskErase(pDataManageDiskSem, nAddrEase, 0);
		}
	}
}											   
#endif

/****************************************************************************
* 名	称：PostParamStoreEvent()
* 功	能：发送参数存储事件
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void PostParamStoreEvent(void)
{
	m_nDataManageTask = DATA_ENENT_PARAM_STORE;
	OSSemPost(pDataManageEvent);
}

/****************************************************************************
* 名	称：PostDataEraseEvent()
* 功	能：发送数据擦除事件
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void PostDataEraseEvent(uint32 nBlockToBeErase)
{
	m_nDataManageTask = DATA_EVENT_ERASE;
	m_nDiskBlockToBeErase = nBlockToBeErase;  	
	OSSemPost(pDataManageEvent);
}


/****************************************************************************
* 名	称：ResetDataEraseBlock()
* 功	能：复位数据擦除区域
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void ResetDataEraseBlock(void)
{
	m_nDiskBlockErased = 0;
}

/****************************************************************************
* 名	称：DataManagerProcess()
* 功	能：数据管理线程
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void DataManagerProcess(void) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	uint8 nErr;
	uint8 nTaskCache;

	//等待存储信号
	OSSemPend(pDataManageEvent, 0, &nErr);

	//获取存储任务
	OS_ENTER_CRITICAL();
	nTaskCache = m_nDataManageTask;
	m_nDataManageTask = DATA_ENENT_NONE;
	OS_EXIT_CRITICAL();

	//解析任务
	switch(nTaskCache)
	{
		//擦除整个零点
		case DATA_ENENT_PARAM_STORE:
			ParamStore();
			break;

		//擦除数据区
		case DATA_EVENT_ERASE:
			
			//判断要求的存储段是否已经擦除
			if(m_nDiskBlockErased < m_nDiskBlockToBeErase)
			{
				//擦除指定扇区的数据
				MemorizerEraseToSect(m_nDiskBlockErased, m_nDiskBlockToBeErase);
	
				//记录信息
				m_nDiskBlockErased = m_nDiskBlockToBeErase;		//记录擦除长度
			}
			
			//指示灯显示
			PostLightOn(20);
			break;

		//默认无操作
		default:
			break;
	}

	OS_ENTER_CRITICAL();
	g_bTaskBusy = FALSE;
	OS_EXIT_CRITICAL();	  
}



