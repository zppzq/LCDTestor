/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: DataAcq.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��09��17��
**����޸�����: 2008��09��17��
**��        ��: ������ͷ�ļ�
*****************************************************************************************************************/
#ifndef 	_DATA_ACQ_H_
#define 	_DATA_ACQ_H_

#ifdef		_DATA_ACQ_C_
#define		DATA_ACQ_EXT
#else
#define		DATA_ACQ_EXT		extern
#endif

//****************************************************************************************************************
//����ʹ��
#define		_DATA_ACQ_INIT_EN_
#define 	_DATA_ACQ_START_EN_
#define		_DATA_ACQ_PAUSE_RESUME_
#define		_DATA_ACQ_STOP_EN_
#define		_DATA_ACQ_GET_STATIC_EN_
#define		_DATA_ACQ_GET_DYNAMIC_EN_
#define		_MAIN_PARAM_STORE_
#define		_ZERO_PARAM_STORE_
#define		_POST_DATA_ACQ_PRO_

//*******************************************************************************************************
//�ɼ�ͨ�����ݸ���
#define DATA_ACQ_COUNT		4	

//���ݲɼ��������Ͷ���
#define	DT_DATA_ACQ			float		//���ݲɼ������ڼ��������
#define	DT_STORE			int16		//���ݲɼ������ڴ洢�ʹ��������


//�ṹ����============================================================
typedef	struct tagSTORE_DATA_TYPE_S
{
	uint8 nStoreType;
	uint8 nDataType;
}STORE_DATA_TYPE_S;

//�洢���ݱ�ʾֵ
typedef	enum tagSTORE_TYPE_E
{
	STORE_TYPE_UE_1,			//��΢Ӧ��ֵ��ȫ��
	STORE_TYPE_UE_2,			//����
	STORE_TYPE_UE_4,			//1/4��
	STORE_TYPE_UV,				//���ѹֵ
}STORE_TYPE_E;


//��̬���ݴ洢�ṹ�壬���ڱ��浱ǰ����
typedef enum tagDATA_TYPE_E
{
	DATA_TYPE_2F_128G = 0,		//��2С��������128
	DATA_TYPE_1F_128G,			//��1С��������128
	DATA_TYPE_0F_128G,			//��0С��������128
	DATA_TYPE_0F_64G,			//��0С��������64
	DATA_TYPE_0F_32G,			//��0С��������32
	DATA_TYPE_0F_16G,
	DATA_TYPE_0F_8G,
	DATA_TYPE_0F_4G,
	DATA_TYPE_0F_2G,
	DATA_TYPE_0F_1G,
	DATA_TYPE_BUTT,
}DATA_TYPE_E;


//ȫ�ֱ�������
extern uint8 g_bTaskBusy;								//����æ��־


//����****************************************************************************************************
//��ʼ��
void DataStoreInit(void);
void DataAcqInit(void) reentrant;

//���ݲɼ�����
void DataAcqParamInit(void) reentrant;
void DataAcqParamStore(void) reentrant;
void DataAcqParamLoad(void) reentrant;
void OnHostParamSet(uint8 nEp, uint8 nType, fp32 fSense, fp32 fRange) reentrant;
void DataAcqParamCacul(void) reentrant;



void DataAcqStart(void) reentrant;
void DataAcqPauseResume(void) reentrant;
void PostDataAcqPro(void) reentrant;
void WaitDataAcqEvent(void) reentrant;
void DataAcqStop(void) reentrant;
float DataAcqGetStatic(uint8 nChannelID) reentrant;
void DataAcqSetStatic(int32 nValue, uint8 nIndex) reentrant;
DT_STORE DataAcqGetDynamic(uint8 nIndex) reentrant;
void DataAcqSetDynamic(int32 nValue, uint8 nIndex) reentrant;
void DataAcqPushDynamic(uint8 nLen) reentrant;
void DataStoreEnd(void) reentrant;
DT_STORE* GetDynDataAddr(uint8 nBlock, uint8 nChannelID) reentrant;
void DataLoad(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint16 nlen,uint8 nChannelID) reentrant;
void DataAcqDataStore(void) reentrant;
void SetDataAcqTask(uint8 nTask) reentrant;
uint8 GetDataAcqTask(void) reentrant;
void DataAcqResetStaticIndex(void) reentrant;


//������
void ADProcess(void) reentrant;
void SampleProcess(void) reentrant;



//****************************************************************************************************************
#endif
