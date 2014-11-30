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
#include "bsp.h"
#include "F410AD.h"

/******************************************************************************
**��	�ƣ�AD_Init()																 
**��	�ܣ�AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef AD_INIT
void ADInit(void) reentrant
{
	ADC0CN &= 0xFC;			// ѡ��дAD0BUSY��ΪAD��ʼ�ź�
	ADC0CN &= 0xBF;			// ѡ������ģʽ

	// tracking mode ��Ĭ�Ϸ�ʽ
	ADC0TK &= 0xF0;			// ʹ��Pre-Tracking Mode
	ADC0TK |= 0x08;
	ADC0CF &= 0x07;			// ϵͳʱ��Ϊ49152000��SARʱ��Ϊ49152000/5
	ADC0CF |= 0x20;
//	ADC0CF &= 0xF9;			// �ۼƲ���16��
//	ADC0CF |= 0x06;

	//ѡ��ADC0���ӵ�������
	ADC0MX  = 0x0C;			// ѡ��P1.4ΪAD����

	//��Դ��ѹ���
	P1MDIN &= 0xEF;			// P1.4Ϊģ������
	P1SKIP |= 0x10;			// ���濪������P1.4
	P1 	   |= 0x10;

	ADC0CN = 0x80;			// ʹ��ADC0��Burst Mode��ֹ��Right justified,AD0BUSY
}
#endif


/******************************************************************************
**��	�ƣ�GetADValue()																 
**��	�ܣ�AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_AD_VALUE
unsigned int GetADValue(void) reentrant
{
	unsigned int nADValue = 0;

	//--------����ADC0---------------------------------------------------------
	while(AD0BUSY);
	AD0BUSY = 1;

	while(!AD0INT);		//�ȴ�ת�����
	AD0INT = 0;

	nADValue = ADC0H;
	nADValue = nADValue << 8;
	nADValue += ADC0L;

	return nADValue;
}
#endif

/******************************************************************************
**��	�ƣ�GetADVolValue()																 
**��	�ܣ�AD��ʼ��																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_AD_VOL_VALUE
float GetADVolValue(void) reentrant
{
	float f;
	unsigned int n;
	n = GetADValue();

	f = (float)n * 2.49f / 4096.0f;

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
float GetPowerVolValue(void) reentrant
{
	float f;
	unsigned int n = GetADValue();

	f = (float)n * 2.49f / 4096.0f;

	f *= 11;

	return f;
}
#endif


/******************************************************************************
**��	�ƣ�GetTemperValue()																 
**��	�ܣ�																 
**��ڲ�������														 
**���ڲ�������															 
**˵	������															 
******************************************************************************/
#ifdef GET_TEMPER_VALUE
float GetTemperValue(void) reentrant
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
float CalCurrentValue(unsigned int nADValue) reentrant
{
	float temp;
	float fCurrentValue = 0.0;
	temp = ((float)nADValue/4096)*VOL_REF;

	//�������ֵ
	fCurrentValue = ((float)nADValue/4096)*VOL_REF/3.7445/CalResValue(CurrentValue);
	return fCurrentValue*1.06;
}
#endif

