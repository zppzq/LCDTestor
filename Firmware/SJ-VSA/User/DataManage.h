/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: DataManage.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��7��21��
**����޸�����: 2008��7��21��
**��        ��: �ɼ������ݹ���ͷ�ļ�
********************************************************************************************************/
#ifndef		_DATA_MANAGE_H_
#define		_DATA_MANAGE_H_

#ifdef		_DATA_MANAGE_C_
#define		DATA_MANAGE_EXT
#else
#define		DATA_MANAGE_EXT	extern
#endif

//�ն˲����ṹ��
//�ն˲����ṹ
//����Ҫ�洢�Ĳ��������ڴ˽ṹ����
typedef struct _STerminalParam
{
	uint32 nDataCfgKey;
	float arrADStep[DATA_ACQ_COUNT];
	float arrZeroVol[DATA_ACQ_COUNT];  
}STerminalParam;

//ȫ���ն˲���
extern STerminalParam g_TerminalParam;

//�������=========================================================================
#define _DATA_MNG_PARAM_CONFIGED_
#define _DATA_MNG_DATA_LOAD_
#define _DATA_MNG_DATA_STORE_
#define _DATA_MNG_MEMORIZER_ERASE_
#define _DATA_MNG_MEMORIZER_ERASE_TO_SECT_
#define _DATA_MNG_ZERO_CONFIGED_

//��������
void DataManageInit(void);

void ParamLoad(void);
void ParamStore(void);
BOOL IsParamEmpty(void);
void ParamSetDefault(void);

void PostParamStoreEvent(void);
void PostDataEraseEvent(uint32 nBlockToBeErase);
void ResetDataEraseBlock(void);


void MemorizerErase(void) reentrant;


BOOL MemDataLoad(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint32 nlen,uint8 nChannelID);
BOOL MemDataStore(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint32 nlen,uint8 nChannelID);


//������
void DataManagerProcess(void);


//�ļ�����
#endif
