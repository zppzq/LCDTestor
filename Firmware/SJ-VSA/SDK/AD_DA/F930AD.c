/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------�� �� �� Ϣ--------------------------------------------------------**
**��   ��   ����AD.c														 **
**��   ��   �ˣ�															 **
**�� ��  ʱ �䣺2007.6.1													 **
**����޸�ʱ�䣺													 		 **
**��        ����ADԴ�ļ�												 	 **
******************************************************************************/

//--------Includes-------------------------------------------------------------
#include "..\Global.h"
#include "F930AD.h"

#define F_F930AD_STEP	0.00322265625;			// 3.3f / 1024.0f

#if		(PCB_VERSION == SJ_RFQC_SC_4D_3_1)		//3.1���߰�

//��Դ�������ؿ���
#define	PW_AD_EN_PORT	PORT(2)
#define	PW_AD_EN		BIT(5)

//��������˿�
#define	PW_AD_PORT		PORT(2)
#define	PW_AD			BIT(4)

#elif	(PCB_VERSION == SJ_RFQC_SC_4D_3_3)		//3.3��

#define	PW_AD_EN_PORT	PORT(2)
#define	PW_AD_EN		BIT(6)

//��������˿�
#define	PW_AD_PORT		PORT(2)
#define	PW_AD			BIT(5)

#endif

/******************************************************************************
**��	�ƣ�F930ADInit()																 
**��	�ܣ�930AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef AD_INIT
void F930ADInit(void) reentrant
{
	uint8 tmp = 0;
	MakePushPull(PW_AD_EN);
	SetLo(PW_AD_EN);

	SetADIn(PW_AD);			//ģ������
	MakeOpenDrain(PW_AD);
	
	ADC0CN &= 0xF8;			// ѡ��дAD0BUSY��ΪAD��ʼ�ź�
	ADC0CN &= 0xBF;			// ѡ������ģʽ

	// tracking mode ��Ĭ�Ϸ�ʽ
//	ADC0TK &= 0xF0;			// ʹ��Pre-Tracking Mode
//	ADC0TK |= 0x08;
//	ADC0CF &= 0x07;			// ϵͳʱ��Ϊ49152000��SARʱ��Ϊ49152000/5
	ADC0CF |= 0x01;			// PGA = 1
//	ADC0CF &= 0xF9;			// �ۼƲ���16��
//	ADC0CF |= 0x06;

	//ѡ��ADC0���ӵ�������
	tmp = (GETPORT(PW_AD) << 3);
	tmp += GETPORTIO(PW_AD);
	ADC0MX  = tmp;			// ѡ��AD����

	//��Դ��ѹ���
	SetHi(PW_AD);

	ADC0CN = 0x80;			// ʹ��ADC0��Burst Mode��ֹ��,AD0BUSY

	ADC0AC = 0x00;			// Right justified
	REF0CN	&= 0xEF;			// ʹ��VDD���ο���ѹ
}
#endif


/******************************************************************************
**��	�ƣ�F930GetADValue()																 
**��	�ܣ�AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_AD_VALUE
unsigned int F930GetADValue(void) reentrant
{
	unsigned int nADValue = 0;

	// --------����ADC0---------------------------------------------------------
	while(AD0BUSY);
	AD0BUSY = 1;

	while(!AD0INT);		//�ȴ�ת�����
	AD0INT = 0;

	OS_ENTER_CRITICAL();
	nADValue = ADC0H;
	nADValue = nADValue << 8;
	nADValue += ADC0L;
	OS_EXIT_CRITICAL();

	return nADValue;
}
#endif

/******************************************************************************
**��	�ƣ�F930GetADVol()																 
**��	�ܣ�AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_AD_VOL_VALUE
float F930GetADVol(void) reentrant
{
	float f;
	uint16 n;

	// ��Դ����------------------------------------
	// ���ڲ�AD
	ADC0CN |= 0x80;
	
	SetADIn(PW_AD);			//ģ������
	MakeOpenDrain(PW_AD);

	// ��Դ��ѹ���
	SetHi(PW_AD);
	MakePushPull(PW_AD_EN);
	// --------------------------------------------

	
	//����ѹ����Դ
	SetHi(PW_AD_EN);
	OSTimeDly(1);
	
	//��ȡADֵ
	n = F930GetADValue();

	// ��Դ����------------------------------------
	MakeOpenDrain(PW_AD);
	MakeOpenDrain(PW_AD_EN);
	SetLo(PW_AD);
	SetLo(PW_AD_EN);

	// �ر��ڲ�AD
	ADC0CN &= ~0x80;
	// --------------------------------------------

	//�����ѹֵ
	f = (float)n * F_F930AD_STEP;

	//�ص�ѹ����Դ
	SetLo(PW_AD_EN);


	return f;
}
#endif

/******************************************************************************
**��	�ƣ�GetPowerVolValue()																 
**��	�ܣ�																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_POWER_VOL_VALUE
float F930GetPowerVolValue(void) reentrant
{
	float f;
	unsigned int n = GetADValue();

	f = (float)n * 2.49f / 4096.0f;

	f *= 11;

	return f;
}
#endif


/******************************************************************************
**��	�ƣ�GetPowerVolValue()																 
**��	�ܣ�																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_TEMPER_VALUE
float F930GetTemperValue(void) reentrant
{
	float f;
	unsigned int n = GetADValue();

	f = (float)n/4095*2.49;

	f *= 11;

	return f;
}
#endif


/******************************************************************************
**��	�ƣ�CalCurrentValue()																 
**��	�ܣ��������ֵ																 
**��ڲ�����AD�Ĳ���ֵ																 
**���ڲ���������ֵ															 
**˵	������														 
******************************************************************************/
#ifdef CAL_CURRENT_VALUE
float F930CalCurrentValue(unsigned int nADValue) reentrant
{
	float temp;
	float fCurrentValue = 0.0;
	temp = ((float)nADValue/4096)*VOL_REF;

	//�������ֵ
	fCurrentValue = ((float)nADValue/4096)*VOL_REF/3.7445/CalResValue(CurrentValue);
	return fCurrentValue*1.06;
}
#endif

