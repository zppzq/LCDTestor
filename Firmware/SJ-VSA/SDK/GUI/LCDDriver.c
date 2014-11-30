/****************************************�ļ���Ϣ**************************************************                      
**
**��   ��   ��: LCDDriver.c
**��   ��   ��: �ư�Դ
**�� �� ��  ��: 2007��7��16��
**����޸�����: 2007��8�� 4��
**��        ��: LCD����Դ�ļ�

**��		��: ��п�
**�� ��  �� ��: 2008��4��4��
***************************************************************************************************/
#define	_LCDDriver_C_

#include "..\BSP\bsp.h"
#include "LCDDriver.h"

//LCD�������ݽӿڣ�8BIT
#define  LCD_DATA       P4

//RS����ָ��ѡ���ź� 1:����,0:ָ��
#define LCD_RS_PORT	    PORT(2)
#define LCD_RS			BIT(5)

//��дѡ���ź� 1:��,0:д
#define LCD_RW_PORT	    PORT(2)
#define LCD_RW			BIT(0)

//��дʹ���ź� д����E�½�Ե��Ч,������E�ߵ�ƽ��Ч
#define LCD_EN_PORT	    PORT(2)
#define LCD_EN			BIT(4)

//LCD����ѡ��
#define	LCD_PSB_PORT	PORT(2)
#define	LCD_PSB			BIT(1)	


//���ݶ���
static INT8U xdata CharBuff[66];

/****************************************************************************
* ��	�ƣ�LCDDriverInit()
* ��	�ܣ�LCD�˿ڳ�ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef		LCDDRIVER_INIT
void LcdDriverInit() reentrant
{
	//�˿�����
	PortPushPull(LCD_DATA);
	MakePushPull(LCD_RS);
	MakePushPull(LCD_RW);
	MakePushPull(LCD_EN);
	MakePushPull(LCD_PSB);

	//��ʼ����ƽ
	SetHi(LCD_RS);
	SetHi(LCD_RW);
	SetLo(LCD_EN);
	SetHi(LCD_PSB);
}
#endif


/****************************************************************************
* ��	�ƣ�LcdReadState()
* ��	�ܣ�LCD��״̬����
* ��ڲ�������
* ���ڲ�����TmpData ,����LCDæ��״̬�͵�ַ
* ˵	������
****************************************************************************/
#ifdef	LCDDRIVER_READ_STATE
INT8U LcdReadState() reentrant
{
	INT8U TmpData;
	
	//�����˿�
	PortOpenDrain(LCD_DATA);
	LCD_DATA = 0xFF;

	//���˿�
	SetLo(LCD_RS);
	SetHi(LCD_RW);
	SetHi(LCD_EN);	
	TmpData = LCD_DATA;
	SetLo(LCD_EN);

	//�����˿�
	PortPushPull(LCD_DATA);

	return TmpData;
}
#endif

/****************************************************************************
* ��	�ƣ�LcdIsBusy()
* ��	�ܣ�LCDæ״̬�жϺ���
* ��ڲ�������
* ���ڲ�����LCDæ��־, 1:æ 0:��
* ˵	������
****************************************************************************/
#ifdef	LCDDRIVER_IS_BUSY
BOOL LcdIsBusy() reentrant
{
	INT8U Tmp;
	Tmp = LcdReadState();
	return ((Tmp & 0x80) > 0)?TRUE:FALSE;
}
#endif


/****************************************************************************
* ��	�ƣ�LcdReadAC()
* ��	�ܣ�LCD��λ��ַ������AC��ֵ����
* ��ڲ�������
* ���ڲ�����λ��ַ������AC��ֵ
* ˵	������
****************************************************************************/
#ifdef	LCDDRIVER_READ_AC
INT8U LcdReadAC() reentrant
{
	INT8U Tmp;
    while(LcdIsBusy() == TRUE);
    Tmp = LcdReadState();
	return (Tmp & 0x7F);
}
#endif


/****************************************************************************
* ��	�ƣ�LcdWriteCmd()
* ��	�ܣ�LCD��λ��ַ������AC��ֵ����
* ��ڲ�����cmd��ָ��
* ���ڲ�������
* ˵	 
****************************************************************************/
#ifdef	LCDDRIVER_WRITE_CMD
void LcdWriteCmd(INT8U cmd) reentrant
{
	while(LcdIsBusy() == TRUE);

	SetLo(LCD_RS);
	SetLo(LCD_RW);
	SetHi(LCD_EN);
	LCD_DATA = cmd;
	SetLo(LCD_EN);
}
#endif


/****************************************************************************
* ��	�ƣ�LcdWriteData()
* ��	�ܣ�LCDд���ݺ���
* ��ڲ�����dat������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	LCDDRIVER_WRITE_DATA
void LcdWriteData(INT8U dat) reentrant
{
	while(LcdIsBusy() == true);

	SetHi(LCD_RS);
	SetLo(LCD_RW);
	SetHi(LCD_EN);
	LCD_DATA = dat;
	SetLo(LCD_EN);
}
#endif


/****************************************************************************
* ��	�ƣ�LcdReadData()
* ��	�ܣ�LCD�����ݺ���
* ��ڲ�������
* ���ڲ�����pData ,�������� nLen ���ݳ���
* ˵	������
****************************************************************************/
#ifdef	LCDDRIVER_READ_DATA
void LcdReadData( INT8U* pData, INT8U nLen) reentrant
{	
    INT8U i;

	//�����˿�
	PortOpenDrain(LCD_DATA);
	LCD_DATA = 0xFF;
	
	//�ٶ�
	SetHi(LCD_RS);  
	SetHi(LCD_RW);
    LCD_DATA = 0xff;
	SetHi(LCD_EN);
	SetLo(LCD_EN);

	//���
	for(i = 0; i < nLen; i++)
	{
		SetHi(LCD_EN);
		pData[i] = LCD_DATA;
		SetLo(LCD_EN);
	}

	//�����˿�
	PortPushPull(LCD_DATA);
}
#endif




/****************************************************************************
* ��	�ƣ�LcdDelaynus()
* ��	�ܣ���ʱ n us
* ��ڲ�����ntime ��ʱ��ʱ��
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef LCD_DELAY_NUS
void LcdDelaynus(INT16U ntime) reentrant
{
   INT16U i;
   i = 6 * ntime;
   while(i--); 
}
#endif
