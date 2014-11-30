#include "includes.h"

//signal defines--------------------------------------------------------------------------------------------------

//****************************************************************************************************************
static OS_EVENT 	*pLCDEvent;						    	//�¼����ƿ�
//static uint8 		nLCDErr;									//�����־
//static uint8 		bLCDignleLED = 0;						//�����־
//static uint16		nLCDTicksSpan;							//ָʾ������

//�˿��趨
#define	LCDHSYNC_PORT  			GPIOF
#define	LCDVSYNC_PORT  			GPIOF
#define	LCDENABLE_PORT  		GPIOF
#define	LCDRSVD_PORT  			GPIOF
#define	LCDCLK_PORT  			GPIOF
#define	LCDHSYNC  				0
#define	LCDVSYNC  				1
#define	LCDENABLE  				2
#define	LCDRSVD  				3
#define	LCDCLK  				4


//�궨��
enum
{
	NORMAL,
	VERTICAL_PORCH
};
/****************************************************************************
* ��	�ƣ�LCDInit()
* ��	�ܣ�ָʾ�Ƴ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void LCDInit() reentrant
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF |
						   RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure); //B

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOF, &GPIO_InitStructure); //CTRL

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOG, &GPIO_InitStructure); //RG

	SetLo(LCDRSVD);							//RSVD pin set low always

	//In idle mode the four pin's state as follow
	SetLo(LCDCLK);
	SetHi(LCDHSYNC);
	SetHi(LCDVSYNC);
	SetLo(LCDENABLE);

	//�����ź���
	pLCDEvent = OSSemCreate(0);
}

/****************************************************************************
* ��	�ƣ�LCDSendHorizontalFrontPorch()
* ��	�ܣ�v
* ��ڲ�������
* ���ڲ������ɼ���ID
* ˵	������
****************************************************************************/
void LCDSendHorizontalFrontPorch() reentrant
{
	u8 i = 0, j = 0;

	SetLo(LCDENABLE);
	SetLo(LCDHSYNC);
	SetHi(LCDCLK);
	GPIOE->ODR = 0x000000FFUL;
	GPIOG->ODR = 0x0000FFFFUL;
	SetLo(LCDCLK);
	i++;
	i++;
	SetHi(LCDHSYNC);

	for (i = 0; i < 160; i++)
	{
		SetHi(LCDCLK);
		GPIOE->ODR = 0x000000FFUL;
		GPIOG->ODR = 0x0000FFFFUL;
		SetLo(LCDCLK);
		j++;
	}
}

void LCDSendHorizontalBackPorch() reentrant
{
	u8 i = 0, j = 0;

	SetLo(LCDENABLE);

	for (i = 0; i < 160; i++)
	{
		SetHi(LCDCLK);
		GPIOE->ODR = 0x000000FFUL;
		GPIOG->ODR = 0x0000FFFFUL;
		SetLo(LCDCLK);
		j++;
	}
}

//��ɫ
void LCDSendHorizontalData(u8 R, u8 G, u8 B, u8 mode) reentrant
{
	u16 i = 0, j = 0;
	u32 RGData;
	u32 BData;

	if (mode == VERTICAL_PORCH)
	{
		SetLo(LCDENABLE);
	}
	else
	{
		SetHi(LCDENABLE);
	}

	RGData = ((G << 8) | R);
	BData  = (B);

	for (i = 0; i < 1024; i++)
	{
		SetHi(LCDCLK);
		GPIOE->ODR = BData;
		GPIOG->ODR = RGData;
		SetLo(LCDCLK);
		j++;
	}
}

void LCDSendHorizontal(u8 R, u8 G, u8 B, u8 mode) reentrant
{
	LCDSendHorizontalFrontPorch();
	LCDSendHorizontalData(R, G, B, mode);
	LCDSendHorizontalBackPorch();
}

void LCDSendVerticalFrontPorch() reentrant
{
	u8 i;

	SetLo(LCDENABLE);
	SetLo(LCDVSYNC);

	for (i = 0; i < 2; i++)
	{
		LCDSendHorizontal(0, 0, 0, VERTICAL_PORCH);
	}
	SetHi(LCDVSYNC);

	for (i = 0; i < 8; i++)
	{
		LCDSendHorizontal(0, 0, 0, VERTICAL_PORCH);
	}
}

void LCDSendVerticalBackPorch() reentrant
{
	u8 i;

	SetLo(LCDENABLE);

	for (i = 0; i < 10; i++)
	{
		LCDSendHorizontal(0, 0, 0, VERTICAL_PORCH);
	}
}

void LCDSendVerticalData(u8 R, u8 G, u8 B) reentrant
{
	u16 i;
	for (i = 0; i < 600; i++)
	{
		LCDSendHorizontal(R, G, B, NORMAL);
	}
}

void LCDSendVertical(u8 R, u8 G, u8 B) reentrant
{
	LCDSendVerticalFrontPorch();
	LCDSendVerticalData(R, G, B);
	LCDSendVerticalBackPorch();
}


void LCDDisplayRed() reentrant
{
	LCDSendVertical(0xFF, 0, 0);
/*	int i, j;
	int Delay;

	SetLo(LCDCLK);

	SetHi(LCDHSYNC);
	SetHi(LCDVSYNC);
	SetLo(LCDENABLE);
	for (Delay = 0; Delay < 100; Delay++);
	SetLo(LCDVSYNC);
	for (Delay = 0; Delay < 100; Delay++);
	SetHi(LCDVSYNC);

	for (i = 0; i < 600; i++)
	{
		SetLo(LCDHSYNC);
		for (Delay = 0; Delay < 30; Delay++);
		SetHi(LCDHSYNC);
		for (j = 0; j < 1024; j++)
		{
			SetLo(LCDCLK);
			GPIOE->ODR = 0x000000FFUL;
			GPIOG->ODR = 0x0000FFFFUL;
			SetHi(LCDCLK);
		}
	}*/
}

/****************************************************************************
* ��	�ƣ�PostLightOn()
* ��	�ܣ�ָʾ�Ƶ����ź�
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void PostLCD(uint16 nTicksSpan) reentrant
{
	OSSemPost(pLCDEvent);
}


/****************************************************************************
* ��	�ƣ�LCDProcess()
* ��	�ܣ�ָʾ�ƴ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void LCDProcess() reentrant
{
//	OSSemPend(pLCDEvent, 500, &nLCDErr);

   	LCDDisplayRed();
}
