/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : ColBusSlaveAps.c
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/

#include <string.h>
#include "..\includes.h"
#include "ColBusDef.h"
#include "ColBusSlaveMac.h"
#include "ColBusSlaveDescribe.h"
#include "..\User\PowerManager.h"
#include "..\User\DataManage.h"
#include "..\User\SyncTimer.h"
#include "..\User\DataAcq.h"

//�����豸�ͺ�
#define COLLECTOR_MODEL			201


//���ݻ�����
#define CB_BUFF_APS_SEND_MAX	256
#define CB_BUFF_APS_RECV_MAX	256
uint8 buffCBApsRecv[CB_BUFF_APS_RECV_MAX];
uint8 buffCBApsSend[CB_BUFF_APS_SEND_MAX];


//������Ϣ
uint8 nCltFunc;										//�ɼ���������
uint8 nCltCfg;										//�ɼ���������
float fCltK[4];										//��ʱ����

//״̬��Ϣ
BOOL g_bPrepareEnable = FALSE;						//��¼�Ƿ�ʹ��Ԥ�ɼ�
BOOL g_bLocalSetZero = FALSE;						//��¼�Ƿ�ʹ�ñ��ص���
BOOL g_nAllowedSample = FALSE;						//�Ƿ��������
BOOL g_bDisableDynamic = TRUE;						//��ֹ��̬�ɼ�
BOOL g_bDyncEnd = FALSE;							//����AD������̬�ɼ�ʱ����һ�����ݴ洢

//ͬ����ʱ������
uint32 nHostTime = 0;								//��¼��������ʱ�䣬��λ100us
uint32 nCycleTimeLeft = 0;							//��¼����Ӧ�ø���ʱ��Ļ�������
																				
//ͬ�����ݲɼ���Ϣ
uint8 	nCbSyncCode;								//ͬ��������   
uint32 	nRequireAddr = 0;							//��̬�ɼ�ʱ�����е����ݵ�ַ
uint16 	nHostReadBytes = 0;							//�ϼ�Ҫ��ȡ�����ݸ���
uint32 	nHostDeviceByteDiff = 0;					//�����ͱ��ص����ݸ�����ֵ
uint32 nHostCount;									//Զ�̴洢����ֵ
float  fHostCount;									//Զ�̴洢����ֵ������
uint16 nBrgRefSampleSplt = SAMPLE_SLIP_SYNC_RT;		//��̬�ɼ��ο�������(ͬ���ź�Ƶ��)
float  fBrgSampleDelt;								//�����ο�����������������͵��ڲɼ�����

//У��ʱҪ�õĲ���
static float TmpFloat;
static float TmpFloatZero; 
static float fMesureCur;

//�������ݵĳ���
static uint32 FlashEraseBlockCount;

//�¼�
static OS_EVENT* pCommDiskSem;

//�ⲿ��������
extern uint8 g_bTaskBusy;							//BUSY����
extern uint8 g_nState;								//�ɼ���״̬��

extern float g_fADStoreParam[DATA_ACQ_COUNT];		//����ת��ϵ��
extern float g_fMesureZeroVol[DATA_ACQ_COUNT];		//�洢���洢���еĵ����ѹ
extern DT_STORE g_sMesureZero[DATA_ACQ_COUNT];		//�������ݣ����ڼ���Ĳ���

extern float fBrgSetSampleRate;
extern uint16 nBrgSetSampleRate;					//Ҫ���õĲ�����
extern uint32 nLocalCount;							//���ش洢����ֵ
extern uint32 nDynLocalAddr[DATA_ACQ_COUNT];		//���ݴ洢����ַ����
extern float g_fADStep[DATA_ACQ_COUNT];

//���߲�������
extern uint16	g_nSleepPoint;					    //���֮������(��λΪ5S)
extern uint16	g_nSleepTicketMax;					//����ʱ��Ϊ���(��λΪ5S)

//��ʼ��
void CBSlaveApsInit(void)
{
	//�������̷����ź�
	pCommDiskSem = OSSemCreate(0);

	//ͬ����ʱ����ʼ��
	SyncTimerInit();
}

//����״̬����
int16 CBSlaveApsFsm(void)
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	int16 nApsRecv;
	uint8 nCmd, nSubCmd;
	uint8 i, nEpIndex, nByteIndex;
	uint8* pAckBuff = buffCBApsSend;

	//ColBus�豸ʹ�á�����-Ӧ��ģʽ
	nApsRecv = CBSlaveNetRecv(buffCBApsRecv, CB_BUFF_APS_RECV_MAX);
	if(nApsRecv <= 0) return nApsRecv;

	//��ȡ����
	nCmd = buffCBApsRecv[0];
	pAckBuff[0] = nCmd;	//���÷���ԭָ��
	
	//����ָ��
	switch(nCmd)
	{
	//��λ
	case CB_RESET:
		if((buffCBApsRecv[1] == 0xA5) && (buffCBApsRecv[1] == 0xA5))
		{
			//Ӧ��
			CBSlaveNetSend(pAckBuff, 1);
			
			//ִ�и�λ����
			//...
		}
		else
		{
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_KEY;
			CBSlaveNetSend(pAckBuff, 2);			
		}
		break;

	//��ȡ�汾
	case CB_VERSION:
		pAckBuff[1] = nCBVersion[0];
		pAckBuff[2] = nCBVersion[1];
		CBSlaveNetSend(pAckBuff, 3);	
		break;

	//�ɼ���ʼ
	case CB_SAMPLE_START:
		pAckBuff[1] = buffCBApsRecv[1];
		if(buffCBApsRecv[1] == CB_OP_STATIC)
		{
			//����״̬
			g_nState |= COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
			g_nState &= ~COLLECTOR_DYNAMIC;					//ȡ����̬�ɼ���־λ

			//Ӧ��
			CBSlaveNetSend(pAckBuff, 2);   
		}
		else if(buffCBApsRecv[1] == CB_OP_STATICS)
		{
			nByteIndex = 0;
			pAckBuff[1] = buffCBApsRecv[1];	//������ָ��
			pAckBuff[2] = buffCBApsRecv[2];	//���ö˵����

			for(i = 0; i < buffCBApsRecv[2]; i++)
			{
				nEpIndex = buffCBApsRecv[3 + i];		//�˵�����
				
				//��ֹ�˵����
				if(nEpIndex >= DEVICE_EP_COUNT)
				{
					pAckBuff[2]--;	//���ض˵������һ	
					continue;
				}
				
				//װ������
				pAckBuff[3 + nByteIndex] = nEpIndex;
				nByteIndex++;	
			}

			//����״̬
			g_nState |= COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
			g_nState &= ~COLLECTOR_DYNAMIC;					//ȡ����̬�ɼ���־λ

			//Ӧ��
			CBSlaveNetSend(pAckBuff, 3 + nByteIndex); 

		}
		else if(buffCBApsRecv[1] == CB_OP_DYNAMIC)
		{
			//Ӧ��
			CBSlaveNetSend(pAckBuff, 2); 

			//ʹ�����ݲɼ�
			if(g_bDisableDynamic == FALSE)
			{
				//����״̬
				g_nState &= ~COLLECTOR_STATIC;
				g_nState |= COLLECTOR_DYNAMIC;
	
				//�����������
				fBrgSetSampleRate = ntohf(*((float*)(buffCBApsRecv+2)));
				nBrgRefSampleSplt = ntohl(*((uint32*)(buffCBApsRecv+6)));

				nBrgSetSampleRate = (uint16)fBrgSetSampleRate; 						
				fBrgSampleDelt = (float)nBrgSetSampleRate;
				fBrgSampleDelt *= (float)nBrgRefSampleSplt;
				fBrgSampleDelt /= 1000000.0f;
	
				nLocalCount = 0;						//����������
				nHostCount = 0;							//������������
	
				//���ò�����
				SetSyncTimerRate(nBrgSetSampleRate);
				
				//����״̬
				ResetDataEraseBlock();
				
				//�������ݲɼ�
				DataAcqStart();							//�������ݲɼ�
	
				//������ʱ��
				g_bDyncEnd = FALSE;						//ȡ��������־
				g_nAllowedSample = FALSE;				//���������(�յ�ͬ���ź�ʱ���������)
				SyncTimerResetRun();					//����ͬ����ʱ��
				OpenSyncTimerISR();						//�ر�ͬ����ʱ��
			}	
	
		}
		else
		{
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 2);		
		}
		break;

	//��ͣ
	case CB_SAMPLE_PAUSE:
		pAckBuff[1] = buffCBApsRecv[1];
		if(buffCBApsRecv[1] == CB_OP_DYNAMIC)
		{

			//�ظ���ǰ����
			*((uint32*)(pAckBuff+2)) = ntohl(nLocalCount);
		
			//Ӧ��
			CBSlaveNetSend(pAckBuff, 6); 

			g_nAllowedSample = 0;						//���������
			SyncTimerStop();							//��ͬ����ʱ��
			CloseSyncTimerISR();						//��ͬ����ʱ���ж�
			DataAcqStop();								//ֹͣ���ݲɼ�
		}
		else if(buffCBApsRecv[1] == CB_OP_SYNC)
		{
			//Ӧ��
			CBSlaveNetSend(pAckBuff, 1); 

			nHostCount = ntohl(*((uint32*)(buffCBApsRecv+2)));				//�������ݸ���

			//�洢����==========================
			if(nLocalCount < nHostCount)
			{
				//�洢����
				PostSampleSem();			
			}
			
			//�洢���һ������
			DataAcqDataStore();				
		}
		else
		{
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 2);		
		}
		break;

	//�ָ���ͣ
	case CB_SAMPLE_RESUME:
		pAckBuff[1] = buffCBApsRecv[1];
		if(buffCBApsRecv[1] == CB_OP_DYNAMIC)
		{ 
			//Ӧ��
			CBSlaveNetSend(pAckBuff, 2); 		

			//�ָ����ݲɼ�
			DataAcqPauseResume();

			//������ʱ��
			g_bDyncEnd = FALSE;						//ȡ��������־
			g_nAllowedSample = 0;					//���������
			SyncTimerResetRun();					//����ͬ����ʱ��
			OpenSyncTimerISR();						//����ͬ����ʱ���ж�  

		}
		else
		{
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 2);		
		}
		break;

	//�ɼ�ֹͣ
	case CB_SAMPLE_STOP:
		//��¼��ָ��
		pAckBuff[1] = buffCBApsRecv[1];
		if(buffCBApsRecv[1] == CB_OP_STATIC)
		{
			//����״̬
			g_nState &= ~COLLECTOR_STATIC;

			//Ӧ��
			CBSlaveNetSend(pAckBuff, 2);
		}
		else if(buffCBApsRecv[1] == CB_OP_STATICS)
		{
			nByteIndex = 0;
			pAckBuff[1] = buffCBApsRecv[1];	//������ָ��
			pAckBuff[2] = buffCBApsRecv[2];	//���ö˵����

			for(i = 0; i < buffCBApsRecv[2]; i++)
			{
				nEpIndex = buffCBApsRecv[3 + i];		//�˵�����
				
				//��ֹ�˵����
				if(nEpIndex >= DEVICE_EP_COUNT)
				{
					pAckBuff[2]--;	//���ض˵������һ	
					continue;
				}
				
				//װ������
				pAckBuff[3 + nByteIndex] = nEpIndex;
				nByteIndex++;
				
			}

			//����״̬
			g_nState &= ~COLLECTOR_STATIC;					//ȡ����̬�ɼ���־λ

			//�ظ�����
			CBSlaveNetSend(pAckBuff, 3 + nByteIndex); 

		}
		else if(buffCBApsRecv[1] == CB_OP_DYNAMIC)
		{
			//Ӧ��
			CBSlaveNetSend(pAckBuff, 2); 

			//����״̬
			g_nState &= ~COLLECTOR_DYNAMIC;

			//������̬�ɼ�
			g_nAllowedSample = 0;						//���������
			SyncTimerStop();							//�ض�ʱ��3
			CloseSyncTimerISR();						//�ض�ʱ��3�ж�
			DataAcqStop();								//�ش�����

			//�����ظ�����
			if(g_bDyncEnd == FALSE)
			{
				//�����Ѿ�ֹͣ�ɼ�
				g_bDyncEnd = TRUE;

				//��ֹ��̬�ɼ�(ֻ�в����ɼ�����֮��Ż���Ӧ)
				g_bDisableDynamic = TRUE;					
				
				//�洢���һ������
				DataAcqDataStore();
			}		
		}
		else
		{
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 2);		
		}
		break;

	//�豸���÷�
	case CB_DEVICE_CFG:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		case CB_DEVICE_MANUFACTURER:
			memcpy(pAckBuff+2, arrFactoryInfo, sizeof(arrFactoryInfo));
			CBSlaveNetSend(pAckBuff, 2 + sizeof(arrFactoryInfo));
			break;

		case CB_DEVICE_MODEL:
			memcpy(pAckBuff+2, arrDeviceInfo, sizeof(arrDeviceInfo));
			CBSlaveNetSend(pAckBuff, 2 + sizeof(arrDeviceInfo));
			break;

		case CB_DEVICE_EP_NUM:
			pAckBuff[2] = DEVICE_EP_COUNT;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		
		case CB_DEVICE_ENDIAN:
			pAckBuff[2] = DEVICE_ENDIAN_MODE;
			CBSlaveNetSend(pAckBuff, 3);
			break;

		default:
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//�豸����
	case CB_DEVICE_PARA:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		case CB_DEVICE_PARA_NUM:
			pAckBuff[2] = 0;	//�����������
			CBSlaveNetSend(pAckBuff, 3);
			break;

		default:
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//�˵����÷�
	case CB_EP_CFG:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		//�˵���������������ȡ
		case CB_EP_TYPE_GETC:	 			
			//�����ȡ���ж˵��������ͣ�������ʵ�ʶ˵����
			if(buffCBApsRecv[3] == 0xFF) buffCBApsRecv[3] = DEVICE_EP_COUNT;
			
			//�������Ƿ�����Ч��Χ��
			if((buffCBApsRecv[2] < DEVICE_EP_COUNT) && (buffCBApsRecv[2] + buffCBApsRecv[3] <=DEVICE_EP_COUNT))
			{
				pAckBuff[2] = buffCBApsRecv[2];	//���ö˵���ʼ
				pAckBuff[3] = buffCBApsRecv[3]; //���ö˵����
				memcpy(pAckBuff + 4, arrEpType + buffCBApsRecv[2], buffCBApsRecv[3]);
				CBSlaveNetSend(pAckBuff, 4 + buffCBApsRecv[3]);
			}
			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//�˵��������ͳ�����ȡ
		case CB_EP_TYPE_GETS:
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//���ö˵����
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//�˵�����
					
					//��ֹ�˵����
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//���ض˵������һ	
						continue;
					}
					
					//װ������
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;

					pAckBuff[3 + nByteIndex] = arrEpType[nEpIndex];  //ȷ��nEpIndex < DEVICE_EP_COUNT
					nByteIndex++;
				}

				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);
			}
			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//�˵�����������ȡ
		case CB_EP_PROP_GETC:
			//�����ȡ���ж˵����ԣ�������ʵ�ʶ˵����
			if(buffCBApsRecv[3] == 0xFF) buffCBApsRecv[3] = DEVICE_EP_COUNT;
			
			//�������Ƿ�����Ч��Χ��
			if((buffCBApsRecv[2] < DEVICE_EP_COUNT) && (buffCBApsRecv[2] + buffCBApsRecv[3] <=DEVICE_EP_COUNT))
			{
				pAckBuff[2] = buffCBApsRecv[2];	//���ö˵���ʼ
				pAckBuff[3] = buffCBApsRecv[3]; //���ö˵����
				memcpy(pAckBuff + 4, arrEpProp + buffCBApsRecv[2], buffCBApsRecv[3]);
				CBSlaveNetSend(pAckBuff, 4 + buffCBApsRecv[3]);
			}
			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//�˵����Գ�����ȡ
		case CB_EP_PROP_GETS:
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//���ö˵����
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//�˵�����
					
					//��ֹ�˵����
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//���ض˵������һ	
						continue;
					}
					
					//װ������
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;

					pAckBuff[3 + nByteIndex] = arrEpProp[nEpIndex];
					nByteIndex++;
				}

				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);
			}
			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//�˵�����λ������ȡ
		case CB_EP_UNIT_GETC:
			//�����ȡ���ж˵�����λ��������ʵ�ʶ˵����
			if(buffCBApsRecv[3] == 0xFF) buffCBApsRecv[3] = DEVICE_EP_COUNT;
			
			//�������Ƿ�����Ч��Χ��
			if((buffCBApsRecv[2] < DEVICE_EP_COUNT) && (buffCBApsRecv[2] + buffCBApsRecv[3] <=DEVICE_EP_COUNT))
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//���ö˵���ʼ
				pAckBuff[3] = buffCBApsRecv[3]; //���ö˵����
				for(i = 0; i < buffCBApsRecv[3]; i++)
				{
					nEpIndex = buffCBApsRecv[2] + i;	//�˵�����
					memcpy(pAckBuff + 4 + nByteIndex, arrEpUnitPt[nEpIndex], arrEpUnitLen[nEpIndex]);
					nByteIndex += arrEpUnitLen[nEpIndex];
				}
			
				CBSlaveNetSend(pAckBuff, 4 + nByteIndex);
			}
			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//�˵�����λ������ȡ
		case CB_EP_UNIT_GETS:
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//���ö˵����
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//�˵�����
					
					//��ֹ�˵����
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//���ض˵������һ	
						continue;
					}
					
					//װ������
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;
					
					memcpy(pAckBuff + 3 + nByteIndex, arrEpUnitPt[nEpIndex], arrEpUnitLen[nEpIndex]);
					nByteIndex += arrEpUnitLen[nEpIndex];
				}

				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);
			}

			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		case CB_EP_PARA_SETC:
			nEpIndex = buffCBApsRecv[2];
			if(nEpIndex < DEVICE_EP_COUNT)
			{
				//У���豸����
				if(COLLECTOR_MODEL == ntohs(*((uint16*)(buffCBApsRecv+3))))
				{
					nCltFunc = *(buffCBApsRecv+5);						//���ܺ�
					nCltCfg = *(buffCBApsRecv+6);						//������
	
					//Ԥ�ɼ���־
					if(0 < (nCltCfg & 0x02)) g_bPrepareEnable = TRUE;
					else g_bPrepareEnable = FALSE;						
			
					//ɨƵ��Χ 
					fCltK[0] = ntohf(*((float*)(buffCBApsRecv+7)));			//����
					fCltK[1] = ntohf(*((float*)(buffCBApsRecv+11)));		//������ϵ��(K)
					fCltK[2] = ntohf(*((float*)(buffCBApsRecv+15)));		//�����͵�ѹ
	
					//��ѹ������
					if(fCltK[2] > 3.6f)
					{
						SetPowerRange(fCltK[2], fCltK[2] + 1.0f);
					}
	
					//���ò���
					OnHostParamSet(buffCBApsRecv[2], nCltFunc, fCltK[1], fCltK[0]);
	
					if(nCltCfg & 0x01) 
					{
						//��ʼ�����ص���ֵ
						for(i=0;i<DATA_ACQ_COUNT;i++)
						{
							g_sMesureZero[i] = (DT_STORE)(g_fMesureZeroVol[i] * g_fADStoreParam[i]);
						}
						
						//�����־��λ
						g_bLocalSetZero = TRUE;
					}
					else 
					{
						//���������ֵ
						DataInitZero(g_sMesureZero);
						
						//�����־��λ
						g_bLocalSetZero = FALSE;
					}
	
					
					//�ظ�
					pAckBuff[2] = buffCBApsRecv[2];
					*((uint16*)(pAckBuff+3)) = (uint16)COLLECTOR_MODEL;
					pAckBuff[5] = GetPowerRate();
	
					CBSlaveNetSend(pAckBuff, 6);  
				}
	
				//�ɼ������ʹ���
				else
				{
					pAckBuff[0] |= 0x80;
					pAckBuff[2] = CB_ERR_MODEL;
					CBSlaveNetSend(pAckBuff, 3);
				}
			}
			else
			{
				//�˵��ַ����ȷ
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}

			break;

		case CB_EP_CALI_SETC:

			//У��ؼ���
			if(ntohl(*((uint32*)(buffCBApsRecv+2))) != BRG_CALI_KEY)
			{
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_KEY;
				CBSlaveNetSend(pAckBuff, 3);			
			}
			else
			{
				nEpIndex = 	buffCBApsRecv[6];

				if(nEpIndex < DEVICE_EP_COUNT)
				{
					//�ظ���
					CBSlaveNetSend(pAckBuff, 2);

					//У����
					TmpFloat = ntohf(*((float*)(buffCBApsRecv+8)));

					//(����*10000)/0��10000�ĵ�ѹ��ֵ
					g_fADStep[nEpIndex] = TmpFloat;

					//�洢У��ֵ
					DataAcqParamStore();
					ParamStore();

					//���¼���ADϵ��
					DataAcqParamCacul();
				}
				else
				{
					pAckBuff[0] |= 0x80;
					pAckBuff[2] = CB_ERR_EP_ADDR;
					CBSlaveNetSend(pAckBuff, 3);
				}
			}
			break;

		case CB_EP_CALI_REFC:

			//У��ؼ���
			if(ntohl(*((uint32*)(buffCBApsRecv+2))) != BRG_CALI_KEY)
			{
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_KEY;
				CBSlaveNetSend(pAckBuff, 3);			
			}
			else
			{
				nEpIndex = 	buffCBApsRecv[6];

				if(nEpIndex < DEVICE_EP_COUNT)
				{
					//�ظ���
					CBSlaveNetSend(pAckBuff, 2);

					//У����
					TmpFloat = ntohf(*((float*)(buffCBApsRecv+8)));
					TmpFloat = TmpFloat * 1.65f ;
					
					//���		   
					TmpFloatZero= ntohf(*((float*)(buffCBApsRecv+12)));
					if(g_bLocalSetZero == TRUE)
					{
						//��ȡ���ص���ֵ
						TmpFloatZero = g_fMesureZeroVol[nEpIndex];	
					}
					
					//��ȡ��ǰ������ѹֵ
					fMesureCur = DataAcqGetStatic(nEpIndex);
					
	
					//����ǰ��ѹֵ�����Ĳ�ֵ
					if((fMesureCur - TmpFloatZero) == 0) fMesureCur = 0.0001f;
					else fMesureCur = (float)(fMesureCur -TmpFloatZero);
					
					//(����*10000)/0��10000�ĵ�ѹ��ֵ
					g_fADStep[nEpIndex] = (g_fADStep[nEpIndex] * TmpFloat) / fMesureCur;

					//�洢У��ֵ
					DataAcqParamStore();
					ParamStore();

					//���¼���ADϵ��
					DataAcqParamCacul();
				}
				else
				{
					pAckBuff[0] |= 0x80;
					pAckBuff[2] = CB_ERR_EP_ADDR;
					CBSlaveNetSend(pAckBuff, 3);
				}
			}
			break;

		case CB_EP_CALI_RSTC:

			//У��ؼ���
			if(ntohl(*((uint32*)(buffCBApsRecv+2))) != BRG_CALI_KEY)
			{
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_KEY;
				CBSlaveNetSend(pAckBuff, 3);			
			}
			else
			{
				nEpIndex = 	buffCBApsRecv[6];

				if(nEpIndex < DEVICE_EP_COUNT)
				{
					//�ظ���
					CBSlaveNetSend(pAckBuff, 2);

					//��λ��Ĭ��ֵ
					g_fADStep[nEpIndex] = F_AD_STEP;

					//�洢У��ֵ
					DataAcqParamStore();
					ParamStore();

					//���¼���ADϵ��
					DataAcqParamCacul();
				}
				else
				{
					pAckBuff[0] |= 0x80;
					pAckBuff[2] = CB_ERR_EP_ADDR;
					CBSlaveNetSend(pAckBuff, 3);
				}
			}
			break;

		default:
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//�˵����ݳ�����ȡ
	case CB_EP_DATA_PREPS:
		
		//�Ƿ�æ
		if(g_bTaskBusy == TRUE)
		{
			//�ظ�æ״̬
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_BUSY;
			CBSlaveNetSend(pAckBuff, 2);
			break;
		}
		
		//����
		if(buffCBApsRecv[1] <= DEVICE_EP_COUNT)
		{
			nByteIndex = 0;
			pAckBuff[1] = buffCBApsRecv[1];	//���ö˵����
			for(i = 0; i < buffCBApsRecv[1]; i++)
			{
				nEpIndex = buffCBApsRecv[2 + i];		//�˵�����
				
				//��ֹ�˵����
				if(nEpIndex >= DEVICE_EP_COUNT)
				{
					pAckBuff[1]--;	//���ض˵������һ	
					continue;
				}
				
				//װ������
				pAckBuff[2 + nByteIndex] = nEpIndex;
				nByteIndex++;  
			}

			//����״̬
			g_bTaskBusy =  TRUE;
			g_nState |= COLLECTOR_STATIC;					//�þ�̬�ɼ���־λ
			g_nState &= ~COLLECTOR_DYNAMIC;					//ȡ����̬�ɼ���־λ

			//�������ݲɼ�
			DataAcqStart();

			//�ظ�����
			CBSlaveNetSend(pAckBuff, 2 + nByteIndex);
		}
		else
		{
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_EP_ADDR;
			CBSlaveNetSend(pAckBuff, 2);
		}
		break;

	//�˵����ݳ�����ȡ
	case CB_EP_DATA_GETS:
		
		//�Ƿ�æ
		if(g_bTaskBusy == TRUE)
		{
			//�ظ�æ״̬
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_BUSY;
			CBSlaveNetSend(pAckBuff, 2);
			break;
		}

		//��������
		if(buffCBApsRecv[1] <= DEVICE_EP_COUNT)
		{
			nByteIndex = 0;
			pAckBuff[1] = buffCBApsRecv[1];	//���ö˵����
			for(i = 0; i < buffCBApsRecv[1]; i++)
			{
				nEpIndex = buffCBApsRecv[2 + i];		//�˵�����
				
				//��ֹ�˵����
				if(nEpIndex >= DEVICE_EP_COUNT)
				{
					pAckBuff[1]--;	//���ض˵������һ	
					continue;
				}
				
				//װ������
				pAckBuff[2 + nByteIndex] = nEpIndex;
				nByteIndex++;
				
				//�������ݴ�ʱ���������߳��޸�
				CB_ENTER_CRITICAL();
				g_fEpValue[nEpIndex] = (DT_STORE)((DataAcqGetStatic(nEpIndex) - g_fMesureZeroVol[nEpIndex]) * g_fADStoreParam[nEpIndex]);
				g_fEpValue[nEpIndex] = htons(g_fEpValue[nEpIndex]);
				memcpy(pAckBuff + 2 + nByteIndex, arrEpDataPt[nEpIndex], arrEpDataLen[nEpIndex]);
				CB_EXIT_CRITICAL();
				
				nByteIndex += arrEpDataLen[nEpIndex];
			}

			CBSlaveNetSend(pAckBuff, 2 + nByteIndex);
		}
		else
		{
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[1] = CB_ERR_EP_ADDR;
			CBSlaveNetSend(pAckBuff, 2);
		}
		break;

	//�������
	case CB_ZERO:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		//���Ӳ���
		case CB_ZERO_SETS:

			//�Ƿ�æ
			if(g_bTaskBusy == TRUE)
			{
				//�ظ�æ״̬
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_BUSY;
				CBSlaveNetSend(pAckBuff, 3);
				break;
			}
			
			//����
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//���ö˵����
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//�˵�����
					
					//��ֹ�˵����
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//���ض˵������һ	
						continue;
					}
					
					//װ������
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;
					
					g_fMesureZeroVol[nEpIndex] = DataAcqGetStatic(nEpIndex);
					g_sMesureZero[nEpIndex] = (DT_STORE)(g_fMesureZeroVol[nEpIndex] * g_fADStoreParam[nEpIndex]);
				}
	
				//�ظ�����
				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);

				//�洢���
				DataAcqParamStore();
				ParamStore();
			}
			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		//���豸ͨ�Ż���������
		case CB_ZERO_CANCELS:
			
			//�Ƿ�æ
			if(g_bTaskBusy == TRUE)
			{
				//�ظ�æ״̬
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_BUSY;
				CBSlaveNetSend(pAckBuff, 3);
				break;
			}
			
			//����
			if(buffCBApsRecv[2] <= DEVICE_EP_COUNT)
			{
				nByteIndex = 0;
				pAckBuff[2] = buffCBApsRecv[2];	//���ö˵����
				for(i = 0; i < buffCBApsRecv[2]; i++)
				{
					nEpIndex = buffCBApsRecv[3 + i];		//�˵�����
					
					//��ֹ�˵����
					if(nEpIndex >= DEVICE_EP_COUNT)
					{
						pAckBuff[2]--;	//���ض˵������һ	
						continue;
					}
					
					//װ������
					pAckBuff[3 + nByteIndex] = nEpIndex;
					nByteIndex++;
					
					g_fMesureZeroVol[nEpIndex] = 0;
					g_sMesureZero[nEpIndex] = 0;
				}
	
				//�ظ�����
				CBSlaveNetSend(pAckBuff, 3 + nByteIndex);

				//�洢���
				DataAcqParamStore();
				ParamStore();
			}
			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}
			break;

		default:
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//�洢����
	case CB_EP_MEM:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		//���Ӳ���
		case CB_EP_MEM_SEG_ERASE:
			
			//����״̬
			SyncTimerStop();
			
			//ֹͣ�������Է�ֹ�쳣
			DataAcqStop();

			//Ӧ����
			CBSlaveNetSend(pAckBuff, 2);

			//�ȵ������ݲɼ��߳�ֹͣ
			OSTimeDly(3);

			//��ȡ��������
			FlashEraseBlockCount = ntohl(*((uint32*)(buffCBApsRecv+11)));

			//�������ݲ����¼�
			PostDataEraseEvent(FlashEraseBlockCount);  
			
			//����״̬
			g_bDisableDynamic = FALSE;			
					
			break;

		case CB_EP_MEM_SEG_DATA:
			nEpIndex = buffCBApsRecv[2];

			if(nEpIndex < DEVICE_EP_COUNT)
			{
				nRequireAddr = ntohl(*((uint32*)(buffCBApsRecv+3)));				//��ȡ������������ 
				nHostReadBytes = ntohs(*((uint32*)(buffCBApsRecv+7)));	
			
	
				//���ûظ���Ϣ
				memcpy(pAckBuff + 2, buffCBApsRecv + 2, 7);	//ep, nRequireAddr, nHostReadBytes
				//MemDataLoad(pCommDiskSem, pAckBuff+9, nRequireAddr, nHostReadBytes, nEpIndex);   //MemLoad�ڽ����ɼ������̶����ܳ����쳣
				DataLoad(pCommDiskSem, pAckBuff+9, nRequireAddr, nHostReadBytes, nEpIndex);
	
				//�ظ�
				CBSlaveNetSend(pAckBuff, 9 + nHostReadBytes);
			}
			else
			{
				//����Ӧ��
				pAckBuff[0] |= 0x80;
				pAckBuff[2] = CB_ERR_EP_ADDR;
				CBSlaveNetSend(pAckBuff, 3);
			}

			break;

		default:
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}

		break;

	//��̬ͬ��
	case CB_SAMPLE_SYNC:
		//�Ƿ��Ѿ���ʼ�ɼ������û�����˳�=====================================			
		if((g_nState & COLLECTOR_DYNAMIC) != COLLECTOR_DYNAMIC) return FALSE;

		//�ж��Ƿ񱻽�ֹ=======================================================
		if(g_bDisableDynamic == TRUE) return FALSE;

		//�������
		g_nAllowedSample = TRUE;

		//��ȡ������===========================================================
		nCbSyncCode = buffCBApsRecv[1];
		
		//ͬ���Ͳ�������=======================================================
		if((nCbSyncCode & CB_SYNC_CALI_TIME) != 0 )
		{
			//����������
			nHostTime = ntohl(*((uint32*)(buffCBApsRecv+2)));				//��ȡ������������

			//������������
			fHostCount = (float)nHostTime;						  	//��������(����������)
			fHostCount *= fBrgSampleDelt;						   	//�����������
			nHostCount = (uint32)fHostCount;						//��������(����)

			//��������ʱ��
			fHostCount = (float)nHostCount;							//��ǰ�ɼ�����
			fHostCount /= fBrgSampleDelt;							//��ǰ�ɼ�������Ӧ��ʱ��(����������)
			nCycleTimeLeft = (uint32)fHostCount;					//��ǰ�ɼ�������Ӧ��ʱ��(ȡ��)
			nCycleTimeLeft = nHostTime - nCycleTimeLeft;			//�����ʱ��(100us��λ)
			
			//У׼��ʱ��========================
			//�ض�ʱ��3�ж��Է�ֹ�쳣
			CloseSyncTimerISR();	
			
			//��ʱ��3ֹͣ
			SyncTimerStop();

			//У׼��ʱ��3
			//����ɼ�����ܶ�
			if(nLocalCount > nHostCount + 1)
			{
				SyncTimerResetRun();
			}
			else
			{
				//��һ�����һЩ
				SyncTimerCaliAndRun();
			}

			//�洢����==========================
			if(nLocalCount < nHostCount)
			{
				//�洢����
				PostSampleSem();			
			}

			//����ʱ��3�ж�
			OpenSyncTimerISR();					
		}

		//�Ƿ�Ҫ�ش���̬ʵʱ����=====================================================
		if(CheckAddrNet(buffCBApsRecv+7, buffCBApsRecv[6]) == TRUE)
		{
			//ѡ���˵�ǰ�ɼ���
			//�ش�������
			if((nCbSyncCode & SYNC_RETURN_EMPTY) != 0x0000 )
			{
				pAckBuff[0] = BRG_REASON_REPORT;	
				pAckBuff[1] = BRG_REASON_EMPTY;
				CBSlaveNetSend(pAckBuff, 2);
				break;
			} 

			//����ͨ����
			nEpIndex = buffCBApsRecv[7 + buffCBApsRecv[6]];

			//��Ҫ�ش����ݵ�ַ
			nRequireAddr = ntohl(*(uint32*)(buffCBApsRecv + 8 + buffCBApsRecv[6]));

			//Ҫ��ȡ�����ݳ���
			nHostReadBytes = ntohs(*(uint16*)(buffCBApsRecv + 12 + buffCBApsRecv[6]));
		
			//�ض�ʱ��3�ж��Է�ֹ�쳣
			CloseSyncTimerISR();		
			
			//����Ҫ�ش������ݳ���
			OS_ENTER_CRITICAL();
			if(nDynLocalAddr[nEpIndex] > nRequireAddr)
			{
				nHostDeviceByteDiff = nDynLocalAddr[nEpIndex] - nRequireAddr;
				if(nHostDeviceByteDiff > nHostReadBytes)
				{
					nHostDeviceByteDiff = nHostReadBytes;
				}
			}
			else nHostDeviceByteDiff = 0;
			OS_EXIT_CRITICAL();

			//�ظ���Ϣ
			pAckBuff[0] = CB_SAMPLE_SYNC;
			pAckBuff[1] = nEpIndex;			
			pAckBuff[2] = nRequireAddr;
			
			//װ������
			nHostReadBytes = nHostDeviceByteDiff;
			*(uint16*)(pAckBuff + 3) = ntohs(nHostReadBytes);

			//������
			DataLoad(pCommDiskSem, pAckBuff + 5, nRequireAddr, nHostReadBytes, nEpIndex);
			
			//����ʱ��3�ж�
			OpenSyncTimerISR();

			//��������
			CBSlaveNetSend(pAckBuff, 5 + nHostReadBytes);	
		}
			
		//�˳�
		break;
							 
	//ͨ��������
	case CB_COMM_CFG:
		nSubCmd = buffCBApsRecv[1];
		pAckBuff[1] = nSubCmd;
		switch(nSubCmd)
		{
		//���Ӳ���
		case CB_COMM_CHECK:
			CBSlaveNetSend(pAckBuff, 2);
			break;

		//��ȡͨ�Ż���������
		case CB_COMM_BUFFLEN_GET:
			*(uint16*)(pAckBuff+2) = ntohs(CB_BUFF_APS_RECV_MAX);
			*(uint16*)(pAckBuff+4) = ntohs(CB_BUFF_APS_SEND_MAX);
			CBSlaveNetSend(pAckBuff, 6);
			break;

		//ͨ������У��
		case CB_COMM_QUALITY_TEST:
			memcpy(pAckBuff, buffCBApsRecv, nApsRecv);
			CBSlaveNetSend(pAckBuff, nApsRecv);
			break;

		//����ʱ������
		case CB_COMM_BEAT_SET:
			//У��ؼ���
			if(ntohl(*(uint32*)(buffCBApsRecv+2)) == BEAT_SET_KEY)
			{
				//��������������λ��Ϊ5�룬��λ��Ҫ��ʱ�任��һ����
				g_nSleepPoint = ntohs(*(uint16*)(buffCBApsRecv+6));
				g_nSleepTicketMax = ntohs(*(uint16*)(buffCBApsRecv+8));

				//�ظ�
				CBSlaveNetSend(pAckBuff, 2);
			}
			break;

		default:
			//����Ӧ��
			pAckBuff[0] |= 0x80;
			pAckBuff[2] = CB_ERR_SUBCMD_US;
			CBSlaveNetSend(pAckBuff, 3);
			break;
		}
		break;

	//ͨ�Ż���
	case CB_COMM_WAKE:
		//У��ؼ���
		if(ntohs(*(uint16*)(buffCBApsRecv+1)) == WAKEUP_RESET_KEY)
		{
			//У��ɹ�
			return 1;
		}	
		else
		{	//У��ʧ��
			return 0;
		}
		//break;	//�Ѿ�return��

	default:
		//����Ӧ��
		pAckBuff[0] |= 0x80;
		pAckBuff[1] = CB_ERR_CMD_US;
		CBSlaveNetSend(pAckBuff, 2);
		break;
	}	 

	return TRUE;
}






