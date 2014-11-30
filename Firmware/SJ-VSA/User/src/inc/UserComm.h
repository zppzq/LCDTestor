/****************************************Copyright (c)************************************************************
**                              
**                          �����������Ƽ����޹�˾       
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: UserComm.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��09��17��
**����޸�����: 2008��09��17��
**��        ��: ͨ��Ӧ�ò�ͷ�ļ�
*****************************************************************************************************************/
#ifndef 	_USERCOMM_H_
#define 	_USERCOMM_H_

#ifdef		_USERCOMM_C_
#define		USERCOMM_EXT
#else
#define		USERCOMM_EXT		extern
#endif
//****************************************************************************************************************

//����ʹ��
#define		_USERCOMM_INIT_EN_
//#define		_DATA_STORE_PRO_


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

typedef	enum tagMEM_TASK_E
{
	MEM_TASK_PUSH_DYN = 0,
	MEM_TASK_STORE_DATA,
	MEM_TASK_BUTT,
}MEM_TASK_E;

//***************************************************************************


/****************************************************************************
* ��	�ƣ�UserCommInit(void)
* ��	�ܣ���ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void UserCommInit(void);

/****************************************************************************
* ��	�ƣ�HostCommProcess(void)
* ��	�ܣ��ϲ�ͨ��������
* ��ڲ�����nLen�����ݰ�����
* ���ڲ������Ƿ�ɹ�
* ˵	������
****************************************************************************/ 
BOOL HostCommProcess(uint8 nLen);

/****************************************************************************
* ��	�ƣ�SensorProcess(void)
* ��	�ܣ�������������
* ��ڲ�������
* ���ڲ������Ƿ�ɹ�
* ˵	������
****************************************************************************/ 
BOOL SensorProcess(void);

/****************************************************************************
* ��	�ƣ�MemProcess(void)
* ��	�ܣ��洢������
* ��ڲ�������
* ���ڲ������Ƿ�ɹ�
* ˵	������
****************************************************************************/ 
BOOL MemProcess(void);

/****************************************************************************
* ��	�ƣ�SetNormalRecvTimeOut(void)
* ��	�ܣ���������״̬ʱͨ�Ž��ճ�ʱʱ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void SetNormalRecvTimeOut(void);

/****************************************************************************
* ��	�ƣ�SetSleepRecvTimeOut(void)
* ��	�ܣ���������ʱͨ�Ž��ճ�ʱʱ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void SetSleepRecvTimeOut(void);

/****************************************************************************
* ��	�ƣ�SetFreqToDefault(void)
* ��	�ܣ�����Ƶ��ΪĬ��Ƶ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void SetFreqToDefault(void);

/****************************************************************************
* ��	�ƣ�SetFreqToCur(void)
* ��	�ܣ�����Ƶ��Ϊ��ƵƵ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void SetFreqToCur(void);

/****************************************************************************
* ��	�ƣ�SetStoreType(void)
* ��	�ܣ����ü�����
* ��ڲ�����nType, ��ʽ��fSense, ������ϵ���� fRange, ����
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void SetStoreType(uint8 nType, fp32 fSense, fp32 fRange);

/****************************************************************************
* ��	�ƣ�CommDataClear(void)
* ��	�ܣ����ͨ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void CommDataClear(void);

/****************************************************************************
* ��	�ƣ�PostDataStore(void)
* ��	�ܣ��������ݴ洢�ź�
* ��ڲ�����nChannelID��ͨ����
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void PostDataStore(uint8 nChannelID);

/****************************************************************************
* ��	�ƣ�ADProcess(void)
* ��	�ܣ�AD������
* ��ڲ�������
* ���ڲ������Ƿ�ɹ�
* ˵	������
****************************************************************************/ 
BOOL ADProcess(void);

/****************************************************************************
* ��	�ƣ�SampleProcess(void)
* ��	�ܣ�����������
* ��ڲ�������
* ���ڲ������Ƿ�ɹ�
* ˵	������
****************************************************************************/ 
BOOL SampleProcess(void);

/****************************************************************************
* ��	�ƣ�SensorDetectionAndShow(void)
* ��	�ܣ����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 
void SensorDetectionAndShow(void);

/****************************************************************************
* ��	�ƣ�UserCommInit(void)
* ��	�ܣ���ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/ 



#endif