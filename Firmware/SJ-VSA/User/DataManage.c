/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: DataManage.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��7��21��
**����޸�����: 2008��7��21��
**��        ��: �ɼ������ݹ���Դ�ļ�
********************************************************************************************************/
#define		_DATA_MANAGE_C_

#include "includes.h"
#include "DataManage.h"
#include "DataAcq.h"

//���������
#define DATA_CONFIG_KEY				0x12F4095A

//�¼�����
#define DATA_ENENT_NONE				0x00
#define DATA_EVENT_ERASE			0x01
#define DATA_ENENT_PARAM_STORE		0x02

//�ն˲���
STerminalParam g_TerminalParam;

//���̷����ź���
static OS_EVENT* pDataManageDiskSem;

//���ݹ����ź�
static OS_EVENT* pDataManageEvent;
static uint8 m_nDataManageTask;

//�����ݶι���
static uint32 m_nDiskBlockErased;							//�Ѿ��������Ķ�
static uint32 m_nDiskBlockToBeErase;						//��Ҫ�����Ķ�

//���ݹ����ʼ��
void DataManageInit(void)
{
	m_nDiskBlockErased = 0;
	m_nDiskBlockErased = 0;

	m_nDataManageTask = DATA_ENENT_NONE;

	pDataManageDiskSem = OSSemCreate(0);  
	pDataManageEvent = OSSemCreate(0);	   
}

/****************************************************************************
* ��	�ƣ�ParamLoad()
* ��	�ܣ���ȡ����
* ��ڲ�����pdat - ����ȡ���ݵ�ָ�룻nStartAddr - flash�е�������ʼ��ַ��
			nlen - ����ȡ���ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _DATA_MNG_PARAM_CONFIGED_
void ParamLoad(void)
{
	//������
	DiskRead(pDataManageDiskSem, (uint8*)(&g_TerminalParam), 0, sizeof(g_TerminalParam));
}
#endif
																		 
/****************************************************************************
* ��	�ƣ�ParamStore()
* ��	�ܣ��洢����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _DATA_MNG_PARAM_CONFIGED_
void ParamStore(void)
{
	//д֮ǰҪ������Ӧ�Ŀ�
	DiskErase(pDataManageDiskSem, 0, 0);

	//���ñ�־
	g_TerminalParam.nDataCfgKey = DATA_CONFIG_KEY;

	//д����
	DiskWrite(pDataManageDiskSem, (uint8*)(&g_TerminalParam), 0, sizeof(g_TerminalParam));
}											   
#endif

/****************************************************************************
* ��	�ƣ�ParamSetDefault()
* ��	�ܣ�ʹ��Ĭ�ϲ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�IsDataConfiged()
* ��	�ܣ��ж�������û�б����ù�
* ��ڲ�������
* ���ڲ�����TRUE - �����ã�FALSE - δ����
* ˵	������
****************************************************************************/
#ifdef _DATA_MNG_PARAM_CONFIGED_
BOOL IsParamEmpty()
{
	if(g_TerminalParam.nDataCfgKey == DATA_CONFIG_KEY) return FALSE;	    
	
	return TRUE;               
}
#endif



/****************************************************************************
* ��	�ƣ�DataLoad()
* ��	�ܣ���ȡ����
* ��ڲ�����pdat - ����ȡ���ݵ�ָ�룻nStartAddr - flash�е�������ʼ��ַ��
			nlen - ����ȡ���ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _DATA_MNG_DATA_LOAD_
BOOL MemDataLoad(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint32 nlen,uint8 nChannelID) reentrant
{
	uint32 nAddrOffset;

	//�������
	if((nStartAddr + nlen) > 0x08000000) return FALSE;

	//ȷ�����˵�ĵ�ַƫ����
	switch(nChannelID)
	{
		case 0 : nAddrOffset = 0x00020000; break;
		case 1 : nAddrOffset = 0x02020000; break;
		case 2 : nAddrOffset = 0x04020000; break;
		case 3 : nAddrOffset = 0x06020000; break;
		default : break;
	}

	//��ȡ����
	DiskRead(pSem, pdat, nStartAddr + nAddrOffset, nlen);

	return TRUE;
}
#endif


/****************************************************************************
* ��	�ƣ�DataStore()
* ��	�ܣ��洢����
* ��ڲ�����pdat - ��д�����ݵ�ָ�룻nStartAddr - flash�е�������ʼ��ַ��
			nlen - ��д�����ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _DATA_MNG_DATA_STORE_
BOOL MemDataStore(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint32 nlen,uint8 nChannelID) reentrant
{
	uint32 nAddrOffset;

	//�������
	if((nStartAddr + nlen) > 0x08000000) return FALSE;

	//ȷ�����˵�ĵ�ַƫ����
	switch(nChannelID)
	{
		case 0 : nAddrOffset = 0x00020000; break;
		case 1 : nAddrOffset = 0x02020000; break;
		case 2 : nAddrOffset = 0x04020000; break;				 
		case 3 : nAddrOffset = 0x06020000; break;
		default : break;
	}
	
	//�洢����
	DiskWrite(pSem, pdat, nStartAddr + nAddrOffset, nlen);

	return TRUE;								   
}											   
#endif

/****************************************************************************
* ��	�ƣ�MemorizerErase()
* ��	�ܣ����������洢��
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�MemorizerEraseToSect()
* ��	�ܣ�����ѡ��Ĵ洢��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
#ifdef _DATA_MNG_MEMORIZER_ERASE_TO_SECT_  
void MemorizerEraseToSect(uint32 nSectStart, uint32 nSectEnd) reentrant
{
	uint32 i;
	uint32 nAddrEaseBase;
	uint32 nAddrEase; 	
	uint8 nEpIndex;

	//������һ�飬ÿ���εĵ�һ�������洢������������
	nSectStart++;
	nSectEnd++;
	
	//ȷ��Ҫ��������������(��ʱ�������3��Block����)
	nSectEnd = (nSectEnd > 252) ? 252 : nSectEnd;

	//�������
	if(nSectStart >= nSectEnd) return;

	//����ÿ��ͨ������Ӧ����
	for(nEpIndex = 0; nEpIndex < DATA_ACQ_COUNT; nEpIndex++)
	{
		//����ÿ��ͨ�������ݻ�ַ
		nAddrEaseBase = nEpIndex;
		nAddrEaseBase <<= 25;
		
		//����ѡ�������
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
* ��	�ƣ�PostParamStoreEvent()
* ��	�ܣ����Ͳ����洢�¼�
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void PostParamStoreEvent(void)
{
	m_nDataManageTask = DATA_ENENT_PARAM_STORE;
	OSSemPost(pDataManageEvent);
}

/****************************************************************************
* ��	�ƣ�PostDataEraseEvent()
* ��	�ܣ��������ݲ����¼�
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void PostDataEraseEvent(uint32 nBlockToBeErase)
{
	m_nDataManageTask = DATA_EVENT_ERASE;
	m_nDiskBlockToBeErase = nBlockToBeErase;  	
	OSSemPost(pDataManageEvent);
}


/****************************************************************************
* ��	�ƣ�ResetDataEraseBlock()
* ��	�ܣ���λ���ݲ�������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void ResetDataEraseBlock(void)
{
	m_nDiskBlockErased = 0;
}

/****************************************************************************
* ��	�ƣ�DataManagerProcess()
* ��	�ܣ����ݹ����߳�
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void DataManagerProcess(void) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	uint8 nErr;
	uint8 nTaskCache;

	//�ȴ��洢�ź�
	OSSemPend(pDataManageEvent, 0, &nErr);

	//��ȡ�洢����
	OS_ENTER_CRITICAL();
	nTaskCache = m_nDataManageTask;
	m_nDataManageTask = DATA_ENENT_NONE;
	OS_EXIT_CRITICAL();

	//��������
	switch(nTaskCache)
	{
		//�����������
		case DATA_ENENT_PARAM_STORE:
			ParamStore();
			break;

		//����������
		case DATA_EVENT_ERASE:
			
			//�ж�Ҫ��Ĵ洢���Ƿ��Ѿ�����
			if(m_nDiskBlockErased < m_nDiskBlockToBeErase)
			{
				//����ָ������������
				MemorizerEraseToSect(m_nDiskBlockErased, m_nDiskBlockToBeErase);
	
				//��¼��Ϣ
				m_nDiskBlockErased = m_nDiskBlockToBeErase;		//��¼��������
			}
			
			//ָʾ����ʾ
			PostLightOn(20);
			break;

		//Ĭ���޲���
		default:
			break;
	}

	OS_ENTER_CRITICAL();
	g_bTaskBusy = FALSE;
	OS_EXIT_CRITICAL();	  
}



