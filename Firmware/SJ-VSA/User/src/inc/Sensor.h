/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: Sensor.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��09��17��
**����޸�����: 2008��09��17��
**��        ��: ������ͷ�ļ�
*****************************************************************************************************************/
#ifndef 	_SENSOR_H_
#define 	_SENSOR_H_

#ifdef		_SENSOR_C_
#define		SENSOR_EXT
#else
#define		SENSOR_EXT		extern
#endif

//****************************************************************************************************************
//����ʹ��
#define		_SENSOR_INIT_EN_
#define 	_SENSOR_START_EN_
#define		_SENSOR_PAUSE_RESUME_
#define		_SENSOR_STOP_EN_
//#define		_SET_SENSOR_STOP_EN_
#define		_SENSOR_GET_STATIC_EN_
#define		_SENSOR_GET_DYNAMIC_EN_
//#define	_SENSOR_UPDATE_PARAM_
#define		_MAIN_PARAM_STORE_
#define		_ZERO_PARAM_STORE_
#define		_POST_SENSOR_PRO_


//������������
#define SENSOR_TASK_NONE			0		
#define SENSOR_TASK_START			1
#define SENSOR_TASK_STOP			2
#define SENSOR_TASK_DETECTION		3
#define SENSOR_TASK_PREPARE			4


//��������ʽ
#define	MEASURE_VOL			0x01		//������ѹ��ʹ��AD���۲���
#define	MEASURE_STRAIN		0x02		//����Ӧ�䣬ʹ��У������


//��̬�ɼ��Ƿ�ʹ��Ԥ�ɼ�*********************************************************************************
#define USE_STATIC_PREPARE	


//*******************************************************************************************************
//�������ݸ�����(ͨ����)
#define SENSOR_MAX_COUNT			4				
//#define BUFF_LEN					4


//�������������Ͷ���
#define	DT_SENSOR			float		//������������������
#define	DT_STORE			int16		//�洢����������

//���������
#define	DYN_BUFF_LEN		256			//��̬���ݻ�������С
#define	DYN_STORE_THRES		16			//���ݴ洢��ֵ

//�������ݸ�����(ͨ����)
#define	F_AD_STEP			F_ADS1246_STEP


//��������
void SensorParamCacul(void) ;
void SensorInit(void) ;
void SensorDataInit(void) ;
void MainParamStore(void) ;
uint32 SensorParamStore(void) ;
uint32 SensorParamLoad(void) ;
void MainZeroParamStore(void) ;
uint32 SensorZeroParamStore(void) ;
uint32 SensorZeroParamLoad(void) ;
void SensorUpdateParam(fp32* pParam) ;
void SensorStart(void) ;
void SensorPauseResume(void) ;
void PostSensorPro(void) ;
void WaitSensorEvent(void) ;
void SetSensorStop(void) ;
BOOL IsSensorStop(void) ;
void SensorStop(void) ;
float SensorGetStatic(uint8 nChannelID) ;
void SensorSetStatic(int32 nValue, uint8 nIndex) ;
DT_STORE SensorGetDynamic(uint8 nIndex) ;
void SensorSetDynamic(int32 nValue, uint8 nIndex) ;
void SensorPushDynamic(uint8 nLen) ;
void DataStoreEnd(void) ;
DT_STORE* GetDynDataAddr(uint8 nBlock, uint8 nChannelID) ;
void DataLoad(uint8* pdat,uint32 nStartAddr,uint16 nlen,uint8 nChannelID) ;
void SensorDataStore(void) ;
void SetSensorTask(uint8 nTask) ;
uint8 GetSensorTask(void) ;
void SensorResetStaticIndex(void) ;



//****************************************************************************************************************
#endif
