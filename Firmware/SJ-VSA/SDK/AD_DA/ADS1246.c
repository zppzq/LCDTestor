/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: ADS1246.c
**��   ��   ��: ����ǿ
**�� �� ��  ��: 2009��4��14��
**����޸�����: 2009��4��24��
**��        ��: 
********************************************************************************************************/
#define _ADS1246_C_

#include "includes.h"
#include "CpuPortAccess.h"
#include "DataAcq.h"
#include "ADS1246.h"
#include "spi.h"


//�źŶ���
#define ADC1_EXTI_LINE 		EXTI_Line8
#define ADC2_EXTI_LINE 		EXTI_Line6
#define ADC3_EXTI_LINE 		EXTI_Line13
#define ADC4_EXTI_LINE 		EXTI_Line11

			  
#define BREAD 		1								//��		
#define BWRITE		0								//д  

//�źŶ���**********************************************************************************************
//ADS1246��Դ����
#define ADPWEN_PORT			GPIOE
#define ADPWEN				2

//ADS1246��Դ����
#define ADSENSOREN_PORT		GPIOG
#define ADSENSOREN			9

//ADS1246��λ�ź�
#define ADRESET_PORT		GPIOE
#define ADRESET				1

//ADS1246ADƬѡ�ź�
#define ADCS4_PORT			GPIOG
#define ADCS4				12

#define ADCS3_PORT			GPIOG
#define ADCS3				14

#define ADCS2_PORT			GPIOB
#define ADCS2				7

#define ADCS1_PORT			GPIOB
#define ADCS1				9

//ADS1246AD����ź�
/**/
#define ADRDY4_PORT			GPIOG
#define ADRDY4				11

#define ADRDY3_PORT			GPIOG
#define ADRDY3				13

#define ADRDY2_PORT			GPIOB
#define ADRDY2				6

#define ADRDY1_PORT			GPIOB
#define ADRDY1				8


//ADS1246AD��ʼ�ź�
#define ADSTART_PORT		GPIOE
#define ADSTART				0

//���ݶ���***********************************************************************
//AD�����ź�
OS_EVENT 	*pADS1246Event;					//�¼����ƿ�
static uint8 		nADS1246Err;			//�����־

//���ݲ����ź�
static OS_EVENT 	*pSampleEvent;			//�¼����ƿ�
static uint8 		nSampleEventErr;		//�����־

uint8 g_ADC_IT_Channel = 0x00;

//for ADS1246--------------------------------
//���ݲ���
uint8 g_nNeedSysInfo[DATA_ACQ_COUNT];		//��¼ϵͳ��Ϣ
uint8 g_nADCurGain[DATA_ACQ_COUNT];			//��¼��ǰ����

//����״̬��־��¼
static BOOL bADS1246Open = FALSE;						//�Ƿ���оƬ
static BOOL bADSample = FALSE;							//�Ƿ����ڲɼ�

BOOL g_bDetectionSensor;								//�Ƿ��⴫����
uint8 g_nDetectionSensorCount[DATA_ACQ_COUNT];		//����������

//�궨��**********************************************************************************************
//����ADS1246��Դ��POWER_DOWN��ʹAD��λ
#define SENSOR_POWER_ON() 		SetHi(ADPWEN);SetHi(ADSENSOREN)
#define SENSOR_POWER_OFF() 		SetLo(ADPWEN);SetLo(ADSENSOREN)

#define	ADSampleStart()			SetHi(ADSTART)
#define	ADSampleStop()			SetLo(ADSTART)

void ADC_IT_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	//ӳ��
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource11);	  	//ADRDY4
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource13);	  	//ADRDY3
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);	  		//ADRDY2
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);	  		//ADRDY1

	//�ж�����Ϊ�½��ش���	
    EXTI->FTSR |= (EXTI_Line6 | EXTI_Line8 | EXTI_Line11 | EXTI_Line13);
}

/****************************************************************************
* ��	�ƣ�ADS1246PortInit()
* ��	�ܣ�ADS1246��ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void ADS1246PortInit()
{
	MakePushPull(ADPWEN);
	MakePushPull(ADSENSOREN);
	MakePushPull(ADRESET);
	MakePushPull(ADSTART);
	MakePushPull(ADCS1);
	MakeOpenDrain(ADRDY1);
	SetHi(ADRESET);
	SetHi(ADRDY1);

	//�ж�����
	ADC_IT_Configuration();

#ifdef	ADCS2
	MakePushPull(ADCS2);
	MakePushPull(ADCS3);
	MakePushPull(ADCS4);
	MakeOpenDrain(ADRDY2);
	MakeOpenDrain(ADRDY3);
	MakeOpenDrain(ADRDY4);
	SetHi(ADRDY2);
	SetHi(ADRDY3);
	SetHi(ADRDY4);
#endif	
	
	ADIntDisable();//��ֹAD�ж�
	SENSOR_POWER_OFF();		//�ر�AD��Դ
	ADSampleStop();			//�رղɼ�
	ADS1246ChannelClose();	//ADƬѡ�ر�

	ADS1246PortOpen();
	SpiPortInit();	//
	SpiPortOpen();
}

//�˿����ſ���
///////////�������Ҫ����//////// 
void ADS1246PortShut()
{
	//�ش�������Դ
	MakePushPull(ADSENSOREN);
	SetLo(ADSENSOREN);

	//��AD��Դ
	MakePushPull(ADPWEN);
	SetLo(ADPWEN);

	//����
	MakeOpenDrain(ADRESET);
	MakeOpenDrain(ADSTART);
	MakeOpenDrain(ADCS1);
	MakeOpenDrain(ADRDY1);


	SetLo(ADRESET);
	SetLo(ADSTART);
	SetLo(ADCS1);
	SetLo(ADRDY1);
	
		
#ifdef	ADCS2
	MakeOpenDrain(ADCS2);
	MakeOpenDrain(ADCS3);
	MakeOpenDrain(ADCS4);
	MakeOpenDrain(ADRDY2);
	MakeOpenDrain(ADRDY3);
	MakeOpenDrain(ADRDY4);
	
	
	SetLo(ADCS2);
	SetLo(ADCS3);
	SetLo(ADCS4);
	SetLo(ADRDY2);
	SetLo(ADRDY3);
	SetLo(ADRDY4);
#endif	  	   
}

void ADS1246PortOpen()
{
	//���뿪��Դ����Ȼ�ɼ����޷�����??
	//SetHi(ADPWEN);
	
	SetHi(ADRESET);
	SetLo(ADSTART);
	SetHi(ADCS1);
	SetHi(ADRDY1);

	MakePushPull(ADRESET);
	MakePushPull(ADSTART);
	MakePushPull(ADCS1);
	MakeOpenDrain(ADRDY1);

#ifdef	ADCS2
	SetHi(ADCS2);
	SetHi(ADCS3);
	SetHi(ADCS4);
	SetHi(ADRDY2);
	SetHi(ADRDY3);
	SetHi(ADRDY4);


	MakePushPull(ADCS2);
	MakePushPull(ADCS3);
	MakePushPull(ADCS4);
	MakeOpenDrain(ADRDY2);
	MakeOpenDrain(ADRDY3);
	MakeOpenDrain(ADRDY4);
#endif
}

/****************************************************************************
* ��	�ƣ�ADS1246VariInit()
* ��	�ܣ�ADS1246ȫ�ֱ�����ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void ADS1246VariInit()
{
	pADS1246Event = NULL;
	pSampleEvent = NULL;
	
	//����ADת������ź���
	if(pADS1246Event == NULL) 
	{
		pADS1246Event = OSSemCreate(0);
	}

	//���������ź���
	if(pSampleEvent == NULL) 
	{
		pSampleEvent = OSSemCreate(0);
	}	

	//������ʼ��
	DataInitZero(g_nNeedSysInfo);
	DataInit(g_nADCurGain,AD_GAIN_SELECT_BUTT);
	g_bDetectionSensor = FALSE;
}


/****************************************************************************
* ��	�ƣ�ADS1246SensorDetection()
* ��	�ܣ�ADS1246 ���������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	_ADS1246_SENSOR_DETECTION_

//�궨��=====================================================================
#define	VALUE_SENSOR_ON		0x00000200		//��������λʱ��ת����ֵ����
#define SENSOR_DETECTION_TIME 		50				//���ʱ��2S
#define MIN_COUNT			20				//С�����޵����ٴ���

void ADS1246IsSensorConnected(int32 nValue, uint8 nChannelID)
{
	//ȡ����ֵ
	nValue = labs(nValue);
	
	//С�ڼ�����ޣ���ʾ����Ӵ�����
	if(nValue < VALUE_SENSOR_ON)
	{
		g_nDetectionSensorCount[nChannelID]++;								
	}
}


/****************************
*BCS		����
*0x01		0
*0x41		0.5uA
*0x81		2uA
*0xC1		10uA		
*****************************/
void ADS1246SensorDetection(uint8 *pSensorConnected)
{
	uint8 i;
	uint8 tmp = 0x41; 		//�趨����

	DataInitZero(g_nDetectionSensorCount);
	
	//��AD��Դ��������������Դ
	SetHi(ADPWEN);
	SetLo(ADSENSOREN);

	bADS1246Open = TRUE;
	
	//ADS1246оƬ��ʼ��
	OSTimeDly(20);		

	ADS1246PowerOnInit();	//оƬ�ϵ縴λ

	//���üĴ�������ʼ����
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		ADS1246RWReg(ADS1246_BCS, &tmp, 1, BWRITE, i);
		*(pSensorConnected+i) = AD_SENSOR_DETECTION_BUTT;
		ADS1246SetGain(AD_GAIN_SELECT_1, i);
		ADS1246SetSampleRate(80, i);
	}

	//��ʼ����
	ADS1246StartSample();

	//ʹ�ܲ���ͳ��
	g_bDetectionSensor = TRUE;

	//��ʱ�趨ʱ�䣬AD������ͳ�����޴�������
	OSTimeDly(SENSOR_DETECTION_TIME);

	//��ֹ����ͳ��
	g_bDetectionSensor = FALSE;

	//�ر����д�������Դ
	ADS1246Close();

	//�ж��Ƿ��д�����
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		if(g_nDetectionSensorCount[i] >= MIN_COUNT)  
			*(pSensorConnected+i) = AD_SENSOR_DETECTION_ON;
		else *(pSensorConnected+i) = AD_SENSOR_DETECTION_OFF;
	}
	
}
#endif

/****************************************************************************
* ��	�ƣ�ADS1246_rw()
* ��	�ܣ�ADS1246 spi ��д����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
static void ADS1246PowerOnInit(void)
{
	//ʹ��RESET�ź���
	SetLo(ADRESET);		
	OSTimeDly(1);		//ADRESET������ʱ10ms
	SetHi(ADRESET);									
	OSTimeDly(1);	
}

/****************************************************************************
* ��	�ƣ�ADS1246ReadReg()
* ��	�ܣ�ADS1246��д�Ĵ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
static void ADS1246RWReg(uint8 nReg, uint8 *pData, uint8 nLen, BOOL bRW, uint8 nChannelID)
{
	uint8 i;

#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	if(BWRITE == bRW) nReg |= ADS1246_WREG;
	else if(BREAD == bRW) nReg |= ADS1246_RREG;

	OS_ENTER_CRITICAL();
	
	ADS1246ChannelOpen(nChannelID);
	spi_rw(nReg);  
	spi_rw(nLen-1);			//��д����nLen-1
	for(i=0;i<nLen;i++)										
	{
		if(BWRITE == bRW)
		{
			spi_rw(*(pData+i));
		}
		else if(BREAD == bRW)
		{
			*(pData+i) = spi_rw(ADS1246_NOP);
		}
	}
	ADS1246ChannelClose();
	
	
	OS_EXIT_CRITICAL();
}

/****************************************************************************
* ��	�ƣ�ADS1234ChannelOpen()
* ��	�ܣ�ADS1234ͨ����
* ��ڲ�����nChannelID ��ѡ���ͨ������Χ��AD_CHANNEL_SELECT_E
* ���ڲ�������
* ˵	������
****************************************************************************/
static void ADS1246ChannelOpen(uint8 nChannelID)
{
	switch(nChannelID)
	{
		case AD_CHANNEL_SELECT_1: 
			SetLo(ADCS1);
			break;
#ifdef	ADCS2			
		case AD_CHANNEL_SELECT_2:
			SetLo(ADCS2);
			break;
		case AD_CHANNEL_SELECT_3:
			SetLo(ADCS3);
			break;
		case AD_CHANNEL_SELECT_4:
			SetLo(ADCS4);
			break;
#endif			
		default :
			SetLo(ADCS1);
			break;
	}  
}

/****************************************************************************
* ��	�ƣ�ADS1234ChannelClose()
* ��	�ܣ�ADS1234ͨ����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void ADS1246ChannelClose()
{
	SetHi(ADCS1);
#ifdef	ADCS2
	SetHi(ADCS2);
	SetHi(ADCS3);
	SetHi(ADCS4);
#endif	
}


/****************************************************************************
* ��	�ƣ�ADS1246GainSelect()
* ��	�ܣ�ADS1246����ѡ����
* ��ڲ�����nGainSelect ��ѡ������棬��Χ��AD_GAIN_SELECT_E
* ���ڲ�������
* ˵	������
****************************************************************************/
void ADS1246SetGain(uint8 nGainSelect,uint8 nChannelID)
{
	g_nNeedSysInfo[nChannelID] &= 0x0F;
	g_nNeedSysInfo[nChannelID] |= (nGainSelect << 4);	
	g_nADCurGain[nChannelID] = nGainSelect;
}

/****************************************************************************
* ��	�ƣ�ADS1246GetGain()
* ��	�ܣ���ȡADC����
* ��ڲ�����nGainSelect ��ѡ������棬��Χ��AD_GAIN_SELECT_E
* ���ڲ�������
* ˵	������
****************************************************************************/
uint8 ADS1246GetGain(uint8 nChannelID)
{
	return g_nADCurGain[nChannelID];
}

/****************************************************************************
* ��	�ƣ�ADS1246SpeedSelect()
* ��	�ܣ�ADS1246��������ѡ����
* ��ڲ�����nSpeedSelect ��ѡ��������ʣ���Χ��AD_SPEED_SELECT_E
* ���ڲ�������
* ˵	������
****************************************************************************/
static void ADS1246SetSpeed(const uint8 nSpeedSelect,uint8 nChannelID)
{
	g_nNeedSysInfo[nChannelID] &= 0xF0;
	g_nNeedSysInfo[nChannelID] |= nSpeedSelect ;	
}

/****************************************************************************
* ��	�ƣ�ADS1246SetParam()
* ��	�ܣ�ADS1246���ò���
* ��ڲ����������õĲ�����ַ
* ���ڲ�������
* ˵	������
****************************************************************************/
static uint8 ADS1246GetSpeed(uint16 nSpeed)
{
#if	(AD_RATE_MAX > 300)
	//�ϸߵĲ�����
	if(1000 < nSpeed)		return AD_SPEED_SELECT_2000SPS; 
	else if(640 < nSpeed)	return AD_SPEED_SELECT_1000SPS; 
	else if(320 < nSpeed)	return AD_SPEED_SELECT_640SPS; 
	else if(160 < nSpeed)	return AD_SPEED_SELECT_320SPS; 
#else
	if(160 < nSpeed)	return AD_SPEED_SELECT_320SPS; 
#endif


	else if(80 < nSpeed)	return AD_SPEED_SELECT_160SPS; 
	else if(40 < nSpeed)	return AD_SPEED_SELECT_80SPS; 
	else if(20 < nSpeed) 	return AD_SPEED_SELECT_40SPS; 
	else if(10 < nSpeed) 	return AD_SPEED_SELECT_20SPS; 
	else if(5 < nSpeed) 	return AD_SPEED_SELECT_10SPS;
	else return AD_SPEED_SELECT_5SPS;
}

/****************************************************************************
* ��	�ƣ�ADS1246SetSampleRate()
* ��	�ܣ�ADS1246���ò����ʲ���
* ��ڲ����������ʣ���λHz
* ���ڲ�������
* ˵	������֤�Ѿ�����AD
****************************************************************************/
#ifdef	_ADS1246_SET_SAMPLE_RATE_
void ADS1246SetSampleRate(uint16 nRate,uint8 nChannelID)
{
	ADS1246SetSpeed(ADS1246GetSpeed(nRate), nChannelID);
}
#endif

/****************************************************************************
* ��	�ƣ�ADS1246SetRange()
* ��	�ܣ�ADS1246��������
* ��ڲ����������ѹ���̣���λ:mv
* ���ڲ�������
* ˵	�������÷�Χ�����趨����
****************************************************************************/
#ifdef _ADS1246_SET_RANGE_
void ADS1246SetRange(uint16 nRange,uint8 nChannelID)
{
	uint8 nGainTemp;
	nGainTemp = ADS1246GetGain(nRange);

	//��������								  
	ADS1246SetGain(nGainTemp,nChannelID);	
}
#endif

/****************************************************************************
* ��	�ƣ�ADS1246Open()
* ��	�ܣ���ADS1246
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void ADS1246Open()
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//��AD�Ѿ����ڴ�״̬���򷵻�(�����)
	if(bADS1246Open == TRUE) return;	
	
	//��AD�ܵ�Դ
	SENSOR_POWER_ON();
	
	//ADS1246оƬ��ʼ�����ϵ����ʱ100ms
	OSTimeDly(20);

	//оƬ�ϵ縴λ
	ADS1246PowerOnInit();
	
	//����AD�򿪱�־
	OS_ENTER_CRITICAL();
	bADS1246Open = TRUE;
	OS_EXIT_CRITICAL();
}

/****************************************************************************
* ��	�ƣ�ADS1246StopStatic()
* ��	�ܣ��ر�ADS1246
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void ADS1246StopStatic()
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//�ر����д�������Դ
	ADS1246StopSample();	

	//�رմ�������Դ
	SetHi(ADPWEN);
	SetLo(ADSENSOREN);

	//����AD����״̬
	OS_ENTER_CRITICAL();
	bADS1246Open = FALSE;
	OS_EXIT_CRITICAL();
}

/****************************************************************************
* ��	�ƣ�ADS1246Close()
* ��	�ܣ��ر�ADS1246
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void ADS1246Close()
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif
	//�ر����д�������Դ
	ADS1246StopSample();	

	//�رմ�������Դ
	SENSOR_POWER_OFF();

	//����AD����״̬
	OS_ENTER_CRITICAL();
	bADS1246Open = FALSE;
	OS_EXIT_CRITICAL();
}

/****************************************************************************
* ��	�ƣ�ADS1246GainCal()
* ��	�ܣ�����У׼
* ��ڲ�������
* ���ڲ�������
* ˵	����Gain = 1, Gain calibration: VREFP �C VREFN (full-scale)
****************************************************************************/
#ifdef	_ADS1246_GAIN_CAL_
static void ADS1246GainCal()
{
	uint8 i;
	uint8 tmp;		  

	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		tmp = 0x02;
		ADS1246RWReg(ADS1246_MUX1,&tmp,1, BWRITE,i);
		ADS1246ChannelOpen(i);
		spi_rw(ADS1246_SYSGCAL);
		ADS1246ChannelClose();											
	}
		  
																						
	//�ȴ�����У�����
	ADS1246WaitDataReady();
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		//ADS1246RWReg(ADS1246_FSC0,((uint8*)(&g_nADGainCal[i])+1),3, BREAD,i);
	}
	//У������
	
}
#endif


/****************************************************************************
* ��	�ƣ�ADS1246OffsetCal()
* ��	�ܣ�ƫ����У��
* ��ڲ�������
* ���ڲ�������
* ˵	����Offset calibration: inputs shorted to midsupply (AVDD + AVSS)/2
****************************************************************************/
#ifdef	_ADS1246_OFFSET_CAL_
static void ADS1246OffsetCal()
{
	uint8 i;
	uint32 nOffsetTmp = 0;
	uint8 tmp;

	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		tmp = 0x01;		//Offset cal
		ADS1246RWReg(ADS1246_MUX1,&tmp, 1,BWRITE,i);		
		ADS1246ChannelOpen(i);
		spi_rw(ADS1246_SYSOCAL);		//��ƫ��У��
		ADS1246ChannelClose();
	}
	//�ȴ�ƫ��У�����
	ADS1246WaitDataReady();
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		//ADS1246RWReg(ADS1246_OFC0, ((uint8*)(&g_nADOffsetCal[i])+1), 3, BREAD, i);
	}
	//У������

}
#endif


/****************************************************************************
* ��	�ƣ�ADS1246StartSample()
* ��	�ܣ���ʼ�ɼ�
* ��ڲ�������ʼ��̬��̬
* ���ڲ�������
* ˵	������ʼ�������л�ͨ��ʱ����
****************************************************************************/
void ADS1246StartSample()
{
	uint8 i;
	uint8 tmp;
	
	//�ж�״̬
	if(TRUE == bADSample) return;

	//ȷ��ϵͳ��Ϣ���óɹ�
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		while(1)
		{
			//����
			OSTimeDly(1);
			ADS1246RWReg(ADS1246_SYS0, &g_nNeedSysInfo[i], 1, BWRITE,i);
			
			//��ȡ����
			tmp = 0;
			ADS1246RWReg(ADS1246_SYS0, &tmp, 1, BREAD,i);
			
			//У��
			if(g_nNeedSysInfo[i] == tmp) break;
			
			//��ֹ
			if(FALSE == bADS1246Open) break;
		}
	}

	//��ʼ����
	ADSampleStart();
	
	//ʹ���ж�
	ADIntEable();


	//У������
#ifdef _ADS1246_GAIN_CAL_
	ADS1246GainCal(); errset		//����У��					
#endif

#ifdef _ADS1246_OFFSET_CAL_
	ADS1246OffsetCal();	errset
#endif
	
	//���ÿ�ʼ������־
	g_ADC_IT_Channel = 0;
	bADSample = TRUE;
}

/****************************************************************************
* ��	�ƣ�ADS1246StopSample()
* ��	�ܣ���ʼ�ɼ�
* ��ڲ�������ʼ�ɼ���ͨ����
* ���ڲ�������
* ˵	������ʼ�������л�ͨ��ʱ����
****************************************************************************/
void ADS1246StopSample()
{
	if(bADSample == FALSE) return;

	//ֹͣ�ɼ�
	ADSampleStop();
	
	//��ֹ�ж�
	ADIntDisable();
	
	//�رղ�����־
	bADSample = FALSE;
}

/****************************************************************************
* ��	�ƣ�ADS1246AGC()
* ��	�ܣ���ȡ������ѹ
* ��ڲ�������
* ���ڲ�����������ѹ����λmv
* ˵	����
****************************************************************************/
#ifdef	_ADS1246_AGC_
static void ADS1246AGC(uint8 nChannelID)
{
	int8 nTmp1;
	int32 tmp;
	uint8 nGainTmp;		 

	ADS1246SetGain(AD_GAIN_SELECT_32,nChannelID);
	ADS1246WaitDataReady();				//�ȴ�ADת�����
	tmp = ADS1246GetValue(nChannelID);
	nTmp1 = LABYTE(tmp,1);
	if(0x01 > abs(nTmp1))
	{								   
		nGainTmp = AD_GAIN_SELECT_128;
	}
	else if(0x02 > abs(nTmp1))
	{
		nGainTmp = AD_GAIN_SELECT_64; 
	}
	else
	{
		nGainTmp = AD_GAIN_SELECT_32; 
	}

	ADS1246SetGain(nGainTmp,nChannelID);
	ADS1246SetReadC();
}
#endif

/****************************************************************************
* ��	�ƣ�ADS1246IsDataReady()
* ��	�ܣ���ȡ��ǰ����״̬
* ��ڲ�������
* ���ڲ�����TRUE,��׼���ã�FALSE,δ׼����
* ˵	����
****************************************************************************/
void  ADS1246WaitDataReady()
{
	//�ȴ��������
	OSSemPend(pADS1246Event, 0, &nADS1246Err);
}

/****************************************************************************
* ��	�ƣ�WaitSampleEvent()
* ��	�ܣ��ȴ������ź�
* ��ڲ�������
* ���ڲ�������
* ˵	����
****************************************************************************/
void WaitSampleEvent()
{
	//�ȴ��������
	OSSemPend(pSampleEvent, 0, &nSampleEventErr);
}

/****************************************************************************
* ��	�ƣ�PostSampleSem()
* ��	�ܣ����Ͳ����ź�
* ��ڲ�������
* ���ڲ�����
* ˵	����
****************************************************************************/
void PostSampleSem()
{
	OSSemPost(pSampleEvent);	 
}



//���ݷ���-----------------------------------------------------------------
//ע����˱���ģʽ��C51Ĭ��Ϊ��˱��룩
//
//�õ�һ���ֵĸ��ֽ�
#define HIBYTE(x)	(*((uint8*)(&x)))

//�õ�һ���ֵĵ��ֽ�
#define LOBYTE(x)	(*((uint8*)(&x)+1))

//�õ�ĳ�������ĵ�i���ֽڣ��͵�ַ��ʼ
#define LABYTE(val,i)		(*((uint8*)(&val)+i))

/****************************************************************************
* ��	�ƣ�ADS1246GetValue()
* ��	�ܣ���ȡ�������
* ��ڲ�������
* ���ڲ������������
* ˵	������ADC ���ݼĴ���������
****************************************************************************/
//#define _AD_CONTINOUS_READ_
static int32 nADBuff = 0;

int32 ADS1246GetValue(uint8 nChannelID)
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif
	
	//���жϣ���ֹ�쳣
	OS_ENTER_CRITICAL();		
	
	//ƬѡоƬ
	ADS1246ChannelOpen(nChannelID);

#ifdef _AD_CONTINOUS_READ_
	if(bADReadContinous[nChannelID] == FALSE)
	{
		//����������
		spi_rw(ADS1246_RDATAC);
		bADReadContinous[nChannelID] = TRUE;

		//ȡ��оƬѡ��
		ADS1246ChannelClose();

		//���ж�
		OS_EXIT_CRITICAL();

		return 0x007FFFFF;
	}
#else
		//����������
		spi_rw(ADS1246_RDATA);		  
#endif	
	

	//������
	LABYTE(nADBuff,2) = spi_rw(ADS1246_NOP);
	LABYTE(nADBuff,1) = spi_rw(ADS1246_NOP);		   
	LABYTE(nADBuff,0) = spi_rw(ADS1246_NOP);
	
	//ȡ��оƬѡ��
	ADS1246ChannelClose();
	
	//���ж�
	OS_EXIT_CRITICAL();
	
	//������չΪ4�ֽ�
	if(0 < (LABYTE(nADBuff,2) & 0x80)) LABYTE(nADBuff,3) = 0xFF;
	else LABYTE(nADBuff,3) = 0x00;

	return nADBuff;
}


/*****************************************************************************************************************
* ��	�ƣ�Int2ISR()
* ��	�ܣ�ADS1246ת������жϳ���
* ��ڲ�������
* ���ڲ�������
* ˵	����AD�ж���Ӧ
*****************************************************************************************************************/
void EXTI9_5_IRQHandler()
{
	//�����ж�		 
	OSIntEnter();
	
	
	//ͨ��1
	if(EXTI_GetITStatus(ADC1_EXTI_LINE) != RESET)
	{
		g_ADC_IT_Channel |= 0x01;
		EXTI_ClearITPendingBit(ADC1_EXTI_LINE);
		OSSemPost(pADS1246Event);			
	}

	//ͨ��2
	if(EXTI_GetITStatus(ADC2_EXTI_LINE) != RESET)
	{
		g_ADC_IT_Channel |= 0x02;
		EXTI_ClearITPendingBit(ADC2_EXTI_LINE);
		OSSemPost(pADS1246Event);
	}
	
	//�˳��ж�
	OSIntExit();					
}

void EXTI15_10_IRQHandler()
{
	//�����ж�		 
	OSIntEnter();
	
	//ͨ��3
	if(EXTI_GetITStatus(ADC3_EXTI_LINE) != RESET)
	{
 		g_ADC_IT_Channel |= 0x04;	
		EXTI_ClearITPendingBit(ADC3_EXTI_LINE);	
		OSSemPost(pADS1246Event);
	}

	//ͨ��4
	if(EXTI_GetITStatus(ADC4_EXTI_LINE) != RESET)
	{
		g_ADC_IT_Channel |= 0x08;
		EXTI_ClearITPendingBit(ADC4_EXTI_LINE);
		OSSemPost(pADS1246Event);
	}
	
	//�˳��ж�
	OSIntExit();							
}

void ADC_IT_Disable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//��ֹ�ж���Ӧ
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);	 
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	//��ֹ�ж�����	 
	EXTI->IMR &= ~(EXTI_Line6 | EXTI_Line8 | EXTI_Line11 | EXTI_Line13);
}

void ADC_IT_Eable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//ʹ���ж�����
	EXTI->IMR |= (EXTI_Line6 | EXTI_Line8 | EXTI_Line11 | EXTI_Line13);
	
	//ʹ���ж���Ӧ
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	 
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}
