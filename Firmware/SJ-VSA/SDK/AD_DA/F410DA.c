/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------�� �� �� Ϣ--------------------------------------------------------**
**��   ��   ����															 **
**��   ��   �ˣ�															 **
**�� ��  ʱ �䣺2007.6.1													 **
**����޸�ʱ�䣺													 		 **
**��        ����														 	 **
******************************************************************************/

//--------Includes-------------------------------------------------------------
#include "std\global.h"
#include "410DA.h"


/******************************************************************************
**��	�ƣ�																 
**��	�ܣ�																 
**��ڲ�����																 
**���ڲ�����															 
**˵	����															 
******************************************************************************/
#ifdef DA_INIT
void DAInit(void)
{
	IDA0CN = 0xF7;						// IDA0ʹ�ܡ�дIDA0H���¡��Ҷ��롢2mA
	P0SKIP |= 0x01;

	IDA1CN = 0xF7;						// IDA1ʹ�ܡ�дIDA1H���¡��Ҷ��롢2mA

	//REF0CN &= 0x7F;						// IDAMRG = 0; IDA1����P0.1
	//P0SKIP |= 0x02;
	REF0CN |= 0x80;						// IDAMRG = 0; IDA1����P0.1

}
#endif

/******************************************************************************
**��	�ƣ�																 
**��	�ܣ�																 
**��ڲ�����																 
**���ڲ�����															 
**˵	����															 
******************************************************************************/
#ifdef DA_OUT_0
void DAOut0(unsigned int DAValue)
{

	IDA0L = DAValue & 0x00FF;			// дIDA0��8λ
	IDA0H = (DAValue & 0xFF00) >> 8;	// дIDA0��8λ���������

}
#endif

/******************************************************************************
**��	�ƣ�																 
**��	�ܣ�																 
**��ڲ�����																 
**���ڲ�����															 
**˵	����															 
******************************************************************************/
#ifdef DA_OUT_1
void DAOut1(unsigned int DAValue)
{
	IDA1L = DAValue & 0x00FF;			// дIDA1��8λ
	IDA1H = (DAValue & 0xFF00) >> 8;	// дIDA1��8λ���������
}
#endif

/******************************************************************************
**��	�ƣ�CalDAValueAtom()																 
**��	�ܣ�����ԭ�ӻ�������Ӧ��DAֵ																 
**��ڲ�����ԭ�ӻ�����										 
**���ڲ�������Ӧ��DAֵ													 
**˵	������
******************************************************************************/
#ifdef CAL_DA_VALUE_ATOM
unsigned int CalDAValueAtom(float fValue)
{
	unsigned int nDAValue;
	unsigned char temp;
	//nDAValue = fValue/MAX_CURRENT * (0x0FA0 - 0x06AA) + 0x06AA;

	//���Ƶ���
	if(fValue>20.0)
	{
		fValue = 20.0;
	}	
	temp = (unsigned char)(fValue/1);
	nDAValue = (fValue-DivCurrentValue[temp])/1.0 * (DivImitateValue[temp+1] - DivImitateValue[temp]) 
														+ DivImitateValue[temp];
	if(nDAValue <= DivImitateValue[0])
	{
		nDAValue = 0x0000;
	}
	if(nDAValue > 0x0FFF)
	{
		nDAValue = 0x0FFF;
	}
	return nDAValue;
}
#endif

/******************************************************************************
**��	�ƣ�																 
**��	�ܣ�																 
**��ڲ�����																 
**���ڲ�����															 
**˵	����															 
******************************************************************************/
#ifdef OUT_CURRENT_ATOM
void OutCurrentAtom(float fValue)
{
	unsigned int nDAValue;	
	
	nDAValue = CalDAValueAtom(fValue);
	DA_Out0(nDAValue);
	
	//--------����ָʾ��-------------------------------------------------------
	if(nDAValue <= DivImitateValue[0])
	{
		LightAtomOff();
	}
	else
	{
		LightAtomOn();
	}
}
#endif
