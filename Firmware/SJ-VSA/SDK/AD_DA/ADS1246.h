/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: ADS1246.h
**��   ��   ��: ����ǿ
**�� �� ��  ��: 2009��04��14��
**����޸�����: 2009��04��15��
**��        ��: 
********************************************************************************************************/
#ifndef		_ADS1246_H_		
#define		_ADS1246_H_	

#ifdef		_ADS1246_C_
#define		_ADS1246_EXT
#else
#define		_ADS1246_EXT		extern
#endif


//���뿪��=======================================================================================
//#define	_ADS1246_STANDBY_MODE_
//#define	_ADS1246_SET_PARAM_
//#define	_ADS1246_IS_DATA_READY_
#define	_ADS1246_SET_ZERO_
//#define	_ADS1246_AGC_
//#define	_ADS1246_SET_RANGE_
//#define	_ADS1246_GAIN_CAL_
//#define	_ADS1246_OFFSET_CAL_
#define	_ADS1246_SET_SAMPLE_RATE_
#define	_ADS1246_SENSOR_DETECTION_
#define	_ADS1246_READ_DATA_CON_


#define AD_RATE_MIN			1
#define	AD_RATE_MAX			2000 

#define ADIntEable()		ADC_IT_Eable()			//��AD�ж�
#define ADIntDisable()		ADC_IT_Disable()		//��ֹAD�ж�


//ADS1246�Ĵ�������==========================================================================
#define	ADS1246_BCS			0x00
#define	ADS1246_VBIAS		0x01
#define	ADS1246_MUX1		0x02		//ƫ��У��������У��
#define	ADS1246_SYS0		0x03		//���棬�������趨
#define	ADS1246_OFC0		0x04		//ƫ��У��ϵ��[7:0]
#define	ADS1246_OFC1		0x05		//[15:8]
#define	ADS1246_OFC2		0x06		//[23:16]
#define	ADS1246_FSC0		0x07		//���̶�У��ϵ��[7:0]
#define	ADS1246_FSC1		0x08		//[15:8]
#define	ADS1246_FSC2		0x09		//[23:16]
#define	ADS1246_ID			0x0A		//ID ��DOUT����

//ADS1246 SPI�����ֶ���==========================================================================
#define	ADS1246_WAKEUP		0x00		//Exit sleep mode
#define	ADS1246_SLEEP		0x02		//Enter sleep mode
#define	ADS1246_SYNC		0x04		//Synchronize the AD conversion
#define	ADS1246_RESET		0x06		//Reset to power-up values
#define	ADS1246_PO_INIT		0x0E		//Power-on initialization
#define	ADS1246_NOP			0xFF		//No operation
#define	ADS1246_RDATA		0x12		//Read data once
#define	ADS1246_RDATAC		0x14		//Read data continuously
#define	ADS1246_SDATAC		0x16		//Stop read data continuously
#define	ADS1246_RREG		0x20		//Read from register rrrr
#define	ADS1246_WREG		0x40		//Write to register rrrr
#define	ADS1246_SYSOCAL		0x60		//System offset calibration
#define	ADS1246_SYSGCAL		0x61		//System gain calibration
#define	ADS1246_SELFOCAL	0x62		//Self offset calibration

//ADS1246��ز����궨��==========================================================================
#define ADS1246_REF 	((float)2.5)	//�ο���ѹ

extern uint8 g_ADC_IT_Channel;
//����AD���ÿ����������ĺ�����
//����PGA
//���㹫ʽ: 	
//			  0.5 * ADS1246_REF * 10^6
//fVolBit = -----------------------------
//	                  (2^23 - 1) 
//
#define	F_ADS1246_STEP		0.298023f  

//�궨��=========================================================================================
//#define INVALIDFP32 			0.0f


//ADS1246��ز����ṹ�嶨��=====================================================================
typedef struct tagADS1246_DCB_S
{
	uint8	nState;
	uint8	nADGain;				//AD����
	uint8	nADSpeed;				//AD����
}ADS1246_DCB_S;


//ADS1246��ز���ö�ٶ���========================================================================
//ADS1246ͨ��ѡ��
typedef enum tagAD_STATE_E
{
	AD_STATE_IDLE = 0,			//������
	AD_STATE_ACTIVE,			//������
	AD_STATE_BUTT,				//��ʼֵ
}AD_STATE_E;

//ADS1246ͨ��ѡ��
typedef enum tagAD_CHANNEL_SELECT_E
{
	AD_CHANNEL_SELECT_1 = 0,	//ѡ��ͨ��1
	AD_CHANNEL_SELECT_2,		//ѡ��ͨ��2
	AD_CHANNEL_SELECT_3,		//ѡ��ͨ��3
	AD_CHANNEL_SELECT_4,		//ѡ��ͨ��4
	AD_CHANNEL_SELECT_BUTT,		//��ʼֵ
}AD_CHANNEL_SELECT_E;

//ADS1246����ѡ��
typedef enum tagAD_GAIN_SELECT_E
{
	AD_GAIN_SELECT_1 = 0,		//����ѡ��1
	AD_GAIN_SELECT_2 = 1,		//����ѡ��2
	AD_GAIN_SELECT_4 ,			//����ѡ��4
	AD_GAIN_SELECT_8  ,			//����ѡ��8
	AD_GAIN_SELECT_16 ,			//����ѡ��16
	AD_GAIN_SELECT_32 ,			//����ѡ��32
	AD_GAIN_SELECT_64 ,			//����ѡ��64
	AD_GAIN_SELECT_128  ,		//����ѡ��128
	AD_GAIN_SELECT_BUTT,		//��ʼֵ������Ϊ����ΪAGC�����ھ�̬
}AD_GAIN_SELECT_E;

//ADS1246��������ѡ��
typedef enum tagAD_SPEED_SELECT_E
{
	AD_SPEED_SELECT_5SPS = 0,	
	AD_SPEED_SELECT_10SPS = 1,
	AD_SPEED_SELECT_20SPS,
	AD_SPEED_SELECT_40SPS,
	AD_SPEED_SELECT_80SPS,
	AD_SPEED_SELECT_160SPS,
	AD_SPEED_SELECT_320SPS,
	AD_SPEED_SELECT_640SPS,
	AD_SPEED_SELECT_1000SPS,
	AD_SPEED_SELECT_2000SPS,
	AD_SPEED_SELECT_BUTT,
}AD_SPEED_SELECT_E;

//ADS1246��������״̬
typedef enum tagAD_DATA_STATUS_E
{
	AD_DATA_STATUS_START_CONVERSION = 0,		//��ʼת��
	AD_DATA_STATUS_UNSETTLED_VIN,				//VINδ�ȶ�
	AD_DATA_STATUS_DIGITAL_FILTER_USETTLED1,	//�����˲�δ�ȶ�
	AD_DATA_STATUS_DIGITAL_FILTER_USETTLED2,	//�����˲�δ�ȶ�
	AD_DATA_STATUS_DIGITAL_FILTER_USETTLED3, 	//�����˲�δ�ȶ�
	AD_DATA_STATUS_SETTLED,						//�����ȶ�
	AD_DATA_STATUS_BUTT,						//��ʼֵ
}AD_DATA_STATUS_E;

//ADS1246ƫ��У׼״̬
typedef enum tagAD_OFFSET_CALIBRATION_E
{
	AD_OFFSET_CALIBRATION_HANDLING = 0,	//У�������
	AD_OFFSET_CALIBRATION_OK,			//У�����
	AD_OFFSET_CALIBRATION_BUTT,			//��ʼֵ
}AD_OFFSET_CALIBRATION_E;

//ADS1246�͹���ģʽ״̬
typedef enum tagAD_STANDBY_MODE_E
{
	AD_STANDBY_MODE_TRUE = 0,		//�͹���ģʽ��
	AD_STANDBY_MODE_FALSE,			//��������ģʽ
	AD_STANDBY_MODE_BUTT,			//��ʼֵ
}AD_STANDBY_MODE_E;

//ADS1246ƫ��У׼״̬
typedef enum tagAD_SENSOR_DETECTION_E
{
	AD_SENSOR_DETECTION_OFF = 0,	//�޴�����
	AD_SENSOR_DETECTION_ON,			//�д�����
	AD_SENSOR_DETECTION_SHORT,		//�ⲿ��·
	AD_SENSOR_DETECTION_BUTT = 0x80,
}AD_SENSOR_DETECTION_E;

static void ADS1246PowerOnInit(void);
static void ADS1246RWReg(uint8 nReg, uint8 *pData, uint8 nLen, BOOL bRW, uint8 nChannelID);
static void ADS1246ChannelOpen(uint8 nChannelID);
void ADS1246ChannelClose(void);
void ADS1246SetGain(uint8 nGainSelect,uint8 nChannelID);
uint8 ADS1246GetGain(uint8 nChannelID);
void ADS1246SetSampleRate(uint16 nRate,uint8 nChannelID);
void ADS1246Close(void);
void ADS1246StartSample(void);
void ADS1246StopSample(void);



//��������================================================
void ADS1246RWReg(uint8 nReg,uint8 *pData,uint8 nLen,BOOL bRW, uint8 nChannelID) ;

//AD��������
int32 ADS1246GetValue(uint8 nChannelID) ;	
void ADS1246PortInit(void) ;
void ADS1246VariInit(void) ;
BOOL IsADS1246Open(void) ;
void ADS1246Open(void) ;
void ADS1246Close(void) ;
void ADS1246StartSample(void) ;
void ADS1246StopSample(void) ;
BOOL ADS1246GetVol(float *fVol) ;
void ADS1246SetSampleRate(uint16 nRate,uint8 nChannelID) ;
void ADS1246SetRange(uint16 nRange,uint8 nChannelID) ;
void ADS1246ChannelClose(void) ;
void ADS1246SetGain(uint8 nGainSelect,uint8 nChannelID) ;
void ADS1246CalTmpValue(void) ;
void PostSampleSem(void) ;
void ADS1246WaitDataReady(void) ;
void WaitSampleEvent(void) ;
void ADS1246IsSensorConnected(int32 nValue, uint8 nChannelID) ;
void ADS1246SensorDetection(uint8 *pSensorConnected) ;
void ADS1246StopStatic(void) ;
void ADC_IT_Disable(void);
void ADC_IT_Eable(void);



//�˿����ſ���
void ADS1246PortShut(void) ;
void ADS1246PortOpen(void) ;
   
#endif
