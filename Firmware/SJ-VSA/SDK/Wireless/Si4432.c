/****************************************Copyright (c)************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------
**��   ��   ��: Si4432.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��12��26��
**����޸�����: 2008��12��26��
**��        ��: Si4432Դ�ļ�
**˵        ��: 				
******************************************************************************/
#define _SI4432_C_

#include "includes.h"
#include "WirelessInterface.h"
#include "Si4432.h"
#include "Nrzi.h"

//���������ȶ���
#define	SI4432_RXLEN_MAX	64
#define SI4432_TXLEN_MAX	64

#define _SI4432POWER_

//********�źŶ���***********************************************************
#ifdef _SI4432POWER_
//�رյ�Դ���ţ�0�����磬1���ص�
#define	SI4432SDN_PORT		GPIOC
#define	SI4432SDN			5
#endif //_SI4432POWER_

//SPIͨ��Ƭѡ����
#define	SI4432NSEL_PORT		GPIOC
#define	SI4432NSEL			4

//�ж��������
#define	NIRQ_PORT			GPIOB
#define	NIRQ				0

//����GPIO�ӵ���Ƭ��IO�ڣ���򿪴˺�
//#define	SI_GPIO_INPUT
#ifdef 	SI_GPIO_INPUT
// GPIO����
#define GPIO0_PORT		PORT(2)
#define GPIO0			BIT(7)
#define GPIO1_PORT		PORT(3)
#define GPIO1			BIT(0)
#define GPIO2_PORT		PORT(3)
#define GPIO2			BIT(1) 
#endif

//--------��Ƶ�����շ�����---------------------------------------------------
#define	AS179_92	1
#define	AS178_73	2
//ѡ�����ù��ʿ��أ����ر�ע��
#define	PA_SWITCH_CHIP	AS179_92

#define	SELF_TRSW
// û�ж����Կ�����Ƶ�����շ�����ͨ����Ƭ�����ſ���
#ifndef	SELF_TRSW				
#define	TRSW1_PORT			PORT(3)
#define	TRSW1				BIT(7)
#define	TRSW2_PORT			PORT(2)
#define	TRSW2				BIT(6)

// �������Կ�����Ƶ�����շ���ͨ��оƬGPIO����
#else
#define	SI4432GPIO0		0
#define	SI4432GPIO1		1
#define	SI4432GPIO2		2

#define	TRSW1				SI4432_GPIO_0
#define	TRSW2				SI4432_GPIO_1
#endif


//---------------------------------------------------------------------------

//--------������ض���-------------------------------------------------------
//#define USE_PA			//���ʹ�ù��ţ���򿪴˺�
#ifdef  USE_PA
//���ſ���
#define PA_PORT			PORT(2)
#define PA				BIT(5)

//���ſ���
#define PAOn()			SetLo(PA)	// ���ſ�����Ƶ���ص�����ͨ·	
#define PAOff()			SetHi(PA)	// ���Źأ���Ƶ���ص�ֱͨ·

//����ר�ú���
void NrfDelay(uint16 nTime);
#endif

//#define	USE_NRF_LED
#ifdef USE_NRF_LED
//ָʾ��
#define NRFLED_PORT			PORT(2)
#define NRFLED				BIT(3)

#define NrfLedOn()			SetLo(NRFLED)
#define NrfLedOff()			SetHi(NRFLED)
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//����ź�=================================================================
//ʱ���ź�                                                                                                                                                                                                                                                                                           
#define SI4432_SCK_PORT	SPI1_GPIO
#define SI4432_SCK			5

//�����ź�==================================================================
//��������
#define SI4432_MISO_PORT	SPI1_GPIO
#define SI4432_MISO		6

//�������
#define SI4432_MOSI_PORT	SPI1_GPIO
#define SI4432_MOSI		7

#define BREAD 		1								//��		
#define BWRITE		0								//д  

#ifndef	NOP
#define NOP			_nop_();_nop_();_nop_()	 		//С��ʱ
#endif
#ifdef _F930_H_
#define Si4432IntEnalbe()		IE0 = 0; EX0 = 1	//��4432���ӵ���Ƭ�����ⲿ�ж�
#define Si4432IntDisable()		EX0 = 0				//���ж�
#else
void Si4432IntEnalbe(void);							//��4432���ӵ���Ƭ�����ⲿ�ж�
void Si4432IntDisable(void);						//���ж�
#endif	//_F930_H_

//�������
#define USE_NRZI_4432
#define USE_NRZI_4432_ON_COLLECTOR
 
//********���ݶ���***********************************************************
//ϵͳ��Ҫ�ź�������
static OS_EVENT		*pSi4432Event;		   				//�¼����ƿ�
static uint8 		nSi4432Err;							//�����־
static uint16 		nSi4432TimeOut;						//���ճ�ʱʱ��
static SI4432DCB 	g_sSI4432DCB;						//��¼SI4432DCB���� 	
static uint8 		nFixPkLen; 							//�̶������ȷ�>0�������ȣ�0���ǹ̶�������
static uint16 		nIntStatus16;						//�жϱ�־
static uint8 		pSi4432SendBuff[80];				//���ͻ�����
static uint8 		pSi4432RecvBuff[80];				//���ջ�����
//****************************************************************************************************************
//********��������************************************************************************************************
//****************************************************************************************************************


void SI4432_IT_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);	  	
}

void Si4432IntDisable(void)
{
	EXTI_InitTypeDef EXTI_InitStructure; 
	EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
	EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
	EXTI_Init(&EXTI_InitStructure);
}

void Si4432IntEnalbe(void)
{
	EXTI_InitTypeDef EXTI_InitStructure; 
	EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure);
}

/****************************************************************************
* ��	�ƣ�Si4432PortInit()
* ��	�ܣ�Si4432�˿ڳ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_INIT_
void Si4432PortInit() reentrant
{
	//���ö˿��������
#ifdef _SI4432POWER_
	MakePushPull(SI4432SDN);			//�ص�������
#endif //_SI4432POWER_

//	MakePushPull(SI4432NSEL);			//SPIͨ��Ƭѡ���
	SPI1_Configuration();

#ifdef SI_GPIO_INPUT
	MakeOpenDrain(GPIO0);
	MakeOpenDrain(GPIO1);
	MakeOpenDrain(GPIO2);
#endif
	
	//���ó�ʼ�˿�״̬
#ifdef _SI4432POWER_
	SetHi(SI4432SDN);					//��ʼ�ص�
#endif //_SI4432POWER_
	SetHi(SI4432NSEL);					//Ƭѡ�ź��ø�

	//�жϳ�ʼ��
#ifdef _F930_H_
	IT0 = 1;				 			//���ش���
	IT01CF &= 0xF0;						//����ж�0���б�־
	IT01CF |= 0x00;						//�͵�ƽ��Ч
	IT01CF |= GetBitValue(NIRQ);		//��������
	MakeOpenDrain(NIRQ);
#else
	SI4432_IT_Configuration();
#endif
	
	//�������
#ifdef USE_PA
	MakePushPull(PA);
	PAOff();							//�ع���
#endif

#ifdef USE_NRF_LED
	MakePushPull(NRFLED);
	NrfLedOff();
#endif

}
#endif

//�˿����ſ���
void Si4432PortShut() reentrant
{
	//MakeOpenDrain(SI4432NSEL);			//SPIͨ��Ƭѡ���
	//SetLo(SI4432NSEL);					//SPIͨ��Ƭѡ���
}

void Si4432PortOpen() reentrant
{
	//SetHi(SI4432NSEL);					//SPIͨ��Ƭѡ���
	//MakePushPull(SI4432NSEL);			//SPIͨ��Ƭѡ���
}

/****************************************************************************
* ��	�ƣ�Si4432VariInit()
* ��	�ܣ�Si4432ȫ�ֱ�����ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_INIT_
void Si4432VariInit() reentrant
{
	pSi4432Event = NULL;
	nSi4432TimeOut = 0;

	//��������
	g_sSI4432DCB.nFreq 			= 4420;								//Si4432Ƶ��
	g_sSI4432DCB.nFreqStep 		= 50;								//Si4432Ƶ�ʲ���
	g_sSI4432DCB.nChannel 		= 0;								//Si4432Ƶ��ͨ��
	g_sSI4432DCB.nFreqDeviation = 50;								//Si4432Ƶ��ƫ��ֵ��625Hz*200=125kHz
	g_sSI4432DCB.nDataRate 		= 100;								//Si4432ͨ������
	g_sSI4432DCB.nOutPower		= 20;								//�������
	g_sSI4432DCB.nModulateType 	= MT_GFSK;							//���Ʒ�ʽ
	g_sSI4432DCB.nModDataSource = MDS_FIFO;							//��������Դ
	g_sSI4432DCB.bManCheEn		= FALSE;							//�Ƿ�ʹ������˹�ر��룬ע�⣺ʹ������˹�ر��룬��Ч�����ʽ�����
	g_sSI4432DCB.bVCOEn			= TRUE;								//�Ƿ�ÿ��У׼VCO

	//������ṹ
	g_sSI4432DCB.sPacketHandler.bPHEn 			= TRUE;				//�Ƿ����������
	g_sSI4432DCB.sPacketHandler.bCRCEn 		 	= FALSE;			//�Ƿ�����CRCУ��
	g_sSI4432DCB.sPacketHandler.nBroad			= 0;				//�㲥��ַ����
	g_sSI4432DCB.sPacketHandler.nPreLen 		= 10;				//ǰ���볤��
	g_sSI4432DCB.sPacketHandler.nPreDectLen	 	= 5;				//ǰ����������
	g_sSI4432DCB.sPacketHandler.nSysWordLen 	= 4;				//ͬ���ָ���
	g_sSI4432DCB.sPacketHandler.SysWord 		= 0x7E2DD4E7;		//ͬ����
	g_sSI4432DCB.sPacketHandler.nTxHeaderLen 	= 0;				//��������ͷ����
	g_sSI4432DCB.sPacketHandler.TxHeader 		= 0x66666666;		//��������ͷ
	g_sSI4432DCB.sPacketHandler.nRxHeaderCheck  = 0;				//��������ͷУ�����
	g_sSI4432DCB.sPacketHandler.RxHeader 		= 0x66666666;		//����У��������ͷ
	g_sSI4432DCB.sPacketHandler.RxHeaderCheckEn = 0x00000000;		//����У��������ͷʹ��λ����
	g_sSI4432DCB.sPacketHandler.bFixPkLen		= FALSE;			//�Ƿ�ʹ�ù̶�������
	g_sSI4432DCB.sPacketHandler.TrxPkLen		= 0;				//���ͽ��հ�����
	
	//����Modem�ṹ
	g_sSI4432DCB.sRxModem.bAFCEn	  	= TRUE;						//�Ƿ�ʹ��AFC
	g_sSI4432DCB.sRxModem.dwn3_bypass 	= 1;						//dwn3_bypass Bypass Decimator by 3 (if set). 
	g_sSI4432DCB.sRxModem.ndec_exp    	= 0;						//ndec_exp[2:0]��IF Filter Decimation Rates.
	g_sSI4432DCB.sRxModem.filset 	  	= 0x0F;						//filset[3:0]��  IF Filter Coefficient Sets.
	g_sSI4432DCB.sRxModem.rxosr	   	  	= 0x0078;					//rxosr[10:0]��  Oversampling Rate.
	g_sSI4432DCB.sRxModem.ncoff	   		= 0x00011111;				//ncoff[19:0]��  NCO Offset.
	g_sSI4432DCB.sRxModem.crgain	   	= 0x0446;					//crgain[10:0]�� Clock Recovery Timing Loop Gain.
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432GetDCB()
* ��	�ܣ���ȡ�豸���Ʋ���
* ��ڲ�����pBrfDcb���豸���Ʋ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_INIT_
SI4432DCB* Si4432GetDCB() reentrant
{
	return &g_sSI4432DCB;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetDCBToChip()
* ��	�ܣ����ò�����оƬ
* ��ڲ�����pBrfDcb���豸���Ʋ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_INIT_
void Si4432SetDCBToChip(SI4432DCB *pBrfDcb)  reentrant
{
	uint8 tmp;
	uint16 nIntStatus;

	//��������ж�ʹ��
	nIntStatus = 0x0000;
	nIntStatus = htons(nIntStatus);
	Si4432_rw(SI4432_INT_EN_1, (uint8 *)(&nIntStatus), 2, BWRITE);

	//����ͨ��Ƶ�ʼ��ز�Ƶ��
	Si4432SetFHSS(pBrfDcb->nFreq, pBrfDcb->nFreqStep, pBrfDcb->nChannel);

	//����Ƶ��ƫ��
	Si4432SetFreqDeviation(pBrfDcb->nFreqDeviation);
	
	//��������ͨ����
	Si4432SetDataRate(pBrfDcb->nDataRate);

	//�����������
	Si4432SetOutPower(pBrfDcb->nOutPower);

	//���õ��Ʒ�ʽ
	Si4432SetModulation(pBrfDcb->nModulateType, pBrfDcb->nModDataSource, pBrfDcb->bManCheEn);	
	
	//���ý�����Modem������FSK,GFSK
	Si4432SetRXModem(&(pBrfDcb->sRxModem));

	//���ð�����
	Si4432SetPacketHandler(&(pBrfDcb->sPacketHandler));
	
	//���÷��ͽ���ʱ��
	Si4432SetSysTiming(pBrfDcb->bVCOEn, 10, 2);	

	//������������
	Si4432SetRSSIGate(0xFF);

	//Excel����ֵ
	tmp = 0x0B;	  											//Set this value for optimal performance!
	Si4432_rw(SI4432_AGC_OVERRIDE_2, &tmp, 1, BWRITE);		//Register 6Ah. AGC Override 2
	
	//��¼������		
	if(pBrfDcb->sPacketHandler.bFixPkLen == TRUE)					// ����ǹ̶�������
	{
		nFixPkLen = pBrfDcb->sPacketHandler.TrxPkLen;	
	}
	else
	{
		nFixPkLen = 0;
	}

	//ʹ�ܻ������ж�(�����꣬�����꣬CRCУ�����)
	nIntStatus = IS_PACKET_SENT | IS_VALID_PACKET_RX | IS_CRC_ERROR;
	nIntStatus = htons(nIntStatus);
	Si4432_rw(SI4432_INT_EN_1, (uint8 *)(&(nIntStatus)), 2, BWRITE);
}
#endif


/****************************************************************************
* ��	�ƣ�Si4432Reset()
* ��	�ܣ������λ�����мĴ����ָ�Ĭ��ֵ
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_RESET_
void Si4432Reset(void) reentrant
{
	uint8 tmp;
	
	Si4432_rw(SI4432_MODE_CTRL_1, &tmp, 1, BREAD);			// Register 07h. Operating Mode and Function Control 1
	tmp |= 0x80;
	Si4432_rw(SI4432_MODE_CTRL_1, &tmp, 1, BWRITE);
}
#endif


/****************************************************************************
* ��	�ƣ�Si4432TRSWSend()
* ��	�ܣ�Si4432���Ϳ��ؿ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
static void  Si4432TRSWSend() 
{
#ifdef SELF_TRSW
	uint8 tmp;
#endif

//179-92
#if	(PA_SWITCH_CHIP == AS179_92)
	
	//�õ�Ƭ�����ƹ��ʿ���
#ifndef	SELF_TRSW				
	SetLo(TRSW1);
	SetHi(TRSW2);

	//��SI4432�Լ���IO�ڿ���
#else
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BREAD);
	tmp &= (0xFE << (TRSW1 - 0x0B));	//�õ�TRSW1
	tmp |= (0x01 << (TRSW2 - 0x0B));	//�ø�TRSW2
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BWRITE);
#endif

//178-73
#elif (PA_SWITCH_CHIP == AS178_73)

	//�õ�Ƭ�����ƹ��ʿ���
#ifndef	SELF_TRSW				
	SetHi(TRSW1);
	SetLo(TRSW2);

	//��SI4432�Լ���IO�ڿ���
#else
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BREAD);
	tmp |= (0x01 << (TRSW1 - 0x0B));	//�ø�TRSW1
	tmp &= (0xFF << (TRSW2 - 0x0B));	//�õ�TRSW2
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BWRITE);
#endif
#endif
}

/****************************************************************************
* ��	�ƣ�Si4432TRSWReceive()
* ��	�ܣ�Si4432���Ϳ��ؿ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
static void  Si4432TRSWReceive()
{
#ifdef SELF_TRSW
	uint8 tmp;
#endif

// 179-92
#if	(PA_SWITCH_CHIP == AS179_92)
	//�õ�Ƭ�����ƹ��ʿ���
	#ifndef	SELF_TRSW				
	SetHi(TRSW1);
	SetLo(TRSW2);
	//��SI4432�Լ���IO�ڿ���
	#else
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BREAD);
	tmp |= (0x01 << (TRSW1 - 0x0B));	//�ø�TRSW1
	tmp &= (0xFF << (TRSW2 - 0x0B));	//�õ�TRSW2
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BWRITE);
	#endif
// 178-73
#elif (PA_SWITCH_CHIP == AS178_73)
	//�õ�Ƭ�����ƹ��ʿ���
	#ifndef	SELF_TRSW				
	SetLo(TRSW1);
	SetHi(TRSW2);
	//��SI4432�Լ���IO�ڿ���
	#else
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BREAD);
	tmp &= (0xFE << (TRSW1 - 0x0B));	//�õ�TRSW1
	tmp |= (0x01 << (TRSW2 - 0x0B));	//�ø�TRSW2
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BWRITE);
	#endif
#endif
}

/****************************************************************************
* ��	�ƣ�Si4432GetIntStatus()
* ��	�ܣ����ж�״̬�Ĵ�����INTERRUPT_STATUS_1 INTERRUPT_STATUS_2
* ��ڲ�������
* ���ڲ����������Ĵ���ֵ����һ�Ĵ���ռ���ֽڣ��ڶ��Ĵ���ռ���ֽ�
* ˵	������
****************************************************************************/
#ifdef _SI4432_GET_INT_STATUS_
uint16 Si4432GetIntStatus(void)	reentrant
{
	uint16 status;
	
	Si4432_rw(SI4432_INT_STATUS_1, (uint8 *)(&status), 2, BREAD);  	// ��2��

	status = htons(status);
	return status;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432GetDevStatus()
* ��	�ܣ����豸״̬
* ��ڲ�������
* ���ڲ������Ĵ���ֵ
* ˵	������
****************************************************************************/
#ifdef _SI4432_GET_DEV_STATUS_
uint8 Si4432GetDevStatus(void)	reentrant
{
	uint8 status;

	Si4432_rw(SI4432_DEVICE_STATUS, &status, 1, BREAD);		// Register 02h. Device Status

	return status;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432GetEZMacStatus()
* ��	�ܣ���ȡEzMac״̬
* ��ڲ�������
* ���ڲ�������
* ˵	����BIT[5]�����������������ڽ��հ����յ���Ч����
	        CRC�������ڷ��Ͱ������������
****************************************************************************/
#ifdef _SI4432_GET_EZMAC_STATUS_
uint8 Si4432GetEZMacStatus(void) reentrant
{
	uint8 tmp;
	
	Si4432_rw(SI4432_EZMAC_STATUS, &tmp, 1, BREAD);		// Address: 31h��EZMac Status, Read Only
	
	return tmp;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432GetPowerStatus()
* ��	�ܣ�����Դ״̬
* ��ڲ�������
* ���ڲ������Ĵ���ֵ
* ˵	������
****************************************************************************/
#ifdef _SI4432_GET_POWER_STATUS_
uint8 Si4432GetPowerStatus(void) reentrant
{
	uint8 status;
	
	Si4432_rw(SI4432_PW_STATUS, &status, 1, BREAD);		// Register 62h. Crystal Oscillator/Power-on-Reset Control
		
	return status;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetMode()
* ��	�ܣ����ù���ģʽ
* ��ڲ�������Ҫ�����ģʽ
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_MODE_
void Si4432SetMode(uint8 nMode) reentrant
{
	Si4432_rw(SI4432_MODE_CTRL_1, &nMode, 1, BWRITE);		// Register 07h. Operating Mode and Function Control 1
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetFreq()
* ��	�ܣ����û���Ƶ��
* ��ڲ�����nFreq����Ҫ���õ�Ƶ��ֵ����λ��0.1MHz
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_FREQUENCY_
void Si4432SetFreq(uint16 nFreq) reentrant
{
	uint8 tmp;
	uint8 hbsel, fb;
	uint16 fc;
	float fFreqTemp;
	
	fFreqTemp = (float)(nFreq / 10.0f);
	
	//�ж�Ƶ���Ƿ񳬳���Χ
	fFreqTemp = (fFreqTemp >= 240)? fFreqTemp : 240;
	fFreqTemp = (fFreqTemp <= 930)? fFreqTemp : 930;

	//�ж����ڸ�Ƶ�ʶλ����ڵ�Ƶ�ʶ�
	if(fFreqTemp < 480)
	{
		hbsel = 0;
		fb = (fFreqTemp - 240)/10.0;
	}
	else
	{
		hbsel = 1;
		fb = (fFreqTemp - 480)/20.0;
	}
	tmp = hbsel << 5;
	tmp	+= fb;
	
	tmp |= 0x40;		//sbsel����֪��ʲô��

	//дƵ�ʴ���ѡ��Ĵ���	
	Si4432_rw(SI4432_FREQ_BAND_SEL, &tmp, 1, BWRITE);			//Register 75h. Frequency Band Select


	//����С����
	fFreqTemp /= (hbsel +1);
	fFreqTemp /= 10;
	fFreqTemp = fFreqTemp - fb - 24;	
	fFreqTemp *= 64000;
	fc = fFreqTemp;			// ����Ƶ�ʼĴ���ֵ

	//дƵ��ֵ
	fc = htons(fc);
	Si4432_rw(SI4432_NOM_CAR_FREQ, (uint8 *)(&fc), 2, BWRITE);	//Register 76h. Nominal Carrier Frequency
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetFHSS()
* ��	�ܣ�FHSS��ʽ����Ƶ��
* ��ڲ�����nBaseFreq����Ƶ����λ��0.1MHz��nStep��ͨ��Ƶ�ʲ�������λ��10KHz��nCh��ͨ����
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_FHSS_
void Si4432SetFHSS(uint16 nBaseFreq, uint8 nStep, uint8 nCh) reentrant
{
	//���û���Ƶ��
	Si4432SetFreq(nBaseFreq);
		
	//����Ƶ�ʲ���
	Si4432_rw(SI4432_FREQ_STEP_SIZE, &nStep, 1, BWRITE);		//Register 7Ah. Frequency Hopping Step Size

	//����Ƶ��ͨ��
	Si4432_rw(SI4432_FREQ_CH_SEL, &nCh, 1, BWRITE);				//Register 79h. Frequency Hopping Channel Select
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetFreqDeviation()
* ��	�ܣ����õ���Ƶ��ƫ��ֵ
* ��ڲ�����nFD��ƫ��ֵ����λ��625Hz
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_FREQUENCY_DEVIATION_
void Si4432SetFreqDeviation(uint16 nFD) reentrant
{
	uint32 nFdForCount;
	uint8 tmp;
	uint8 fd;

	//�ж�����ֵ�Ƿ񳬳����Χ
	nFD = (nFD >= 1)? nFD : 1;
	nFD = (nFD <= 320)? nFD : 320;

	//����ɼĴ�����ֵ
	nFdForCount = (uint32)nFD;								//����ʱnFD�ֳ�����
	nFdForCount = (nFdForCount * 1000) / 625;				//����
	nFD = (uint16)nFdForCount;								//ת����uint16

	//���λ
	fd = nFD >> 8;

	//fd[8:0]��9λ��дBIT[8]
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BREAD);			//Register 71h. Modulation Mode Control 2
	if(fd > 0)	tmp |= 0x04;								//BIT[2]
	else 		tmp &= ~0x04;
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BWRITE);			

	//fd[8:0]��9λ��дBIT[7:0]
	tmp = nFD & 0x00FF;
	Si4432_rw(SI4432_FREQ_DEVIATION, &tmp, 1, BWRITE);		//Register 72h. Frequency Deviation
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetDataRate()
* ��	�ܣ���������ͨ����
* ��ڲ�����ndr����ͨ���ʣ�1-128����λ��kbps
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_DATA_RATE_
void Si4432SetDataRate(uint8 ndr) reentrant
{
	uint8 tmp,r;
	uint16 txdr;
	uint32 n;

	ndr = (ndr >= 1)? ndr : 1;
	ndr = (ndr <= 128)? ndr : 128;

	//�ж��������Ƿ�С��30kbps���С��ָ��λ����1��������0
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BREAD);	// Register 70h. Modulation Mode Control 1
	
	//�޸�ֵ
	if(ndr < 30)	
	{
		tmp |= 0x20;				// BIT[5]								
		r = 1;	
	}
	else			
	{
		tmp &= ~0x20;
		r = 0;		
	}
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BWRITE);	// Register 70h. Modulation Mode Control 1

	//����Ĵ�����ֵ
	n = 1;
	n = n<<(16+5*r);
	n *= ndr;
	n /= 1000;
	txdr = n;

	//����ͨ������	
	txdr = htons(txdr);
	Si4432_rw(SI4432_TX_DATA_RATE, (uint8 *)(&txdr), 2, BWRITE); 	// Register 6Eh-6Fh. TX Data Rate 1 - 0
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetOutPower()
* ��	�ܣ������������
* ��ڲ��������õ��������ֵ��11��14��17��20
* ���ڲ�������
* ˵	������ڲ�����0<=(11)<14��14<=(14)<17��17<=(17)<20��20==(20)
****************************************************************************/
#ifdef _SI4432_SET_OUT_POWER_
void Si4432SetOutPower(uint8 ndbm)	reentrant
{
	if((20 < ndbm) || (11 > ndbm)) ndbm = 20;		//����ֵ��Ĭ���趨Ϊ20dbm
	// �淶�������ֵ��ֻ����11��14��17��20��4��ֵ
	ndbm = (ndbm - 11) / 3;

	// ���ù���	
	Si4432_rw(SI4432_TX_POWER, &ndbm, 1, BWRITE);  	// Register 6Dh. TX Power
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetModulation()
* ��	�ܣ����õ���ģʽ
* ��ڲ�����nMT���������ͣ�nMDS����������Դ��
* ���ڲ�������
* ˵	����
****************************************************************************/
#ifdef _SI4432_MODULATION_
void Si4432SetModulation(uint8 nMT, uint8 nMDS, BOOL bManChe) reentrant
{
	uint8 tmp;

	//����ģʽ�͵�������Դ����
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BREAD);			// Register 71h. Modulation Mode Control 2
	tmp = (tmp & 0xFC) + (nMT & 0x03);
	tmp = (tmp & 0xCF) + (nMDS & 0x30);
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BWRITE);

	//��������˹�ر��룬��֤ǰ����Ϊ��10101010......
	tmp = 0x0A;
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BWRITE); 		// Register 70h. Modulation Mode Control 1

	//����˹�ر������
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BREAD);			// Register 70h. Modulation Mode Control 1
	if(bManChe == TRUE)	tmp |= 0x02;						// BIT[1]
	else				tmp &= ~0x02;
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetPacketHandler()
* ��	�ܣ����ð�����
* ��ڲ�������
* ���ڲ�������
* ˵	��������30h-4Bh�Ĵ�����
			uint8	bPHEn;					// �Ƿ����������
			uint8  	bCRCEn;					// �Ƿ�����CRCУ��
			uint8 	nBroad;					// �㲥��ַ����
			uint8	nRxHeaderCheck;			// ��������ͷУ�����
			uint8 	nSysWordLen;			// ͬ���ָ���
			uint8	nTxHeaderLen;			// ��������ͷ����
			uint16  nPreLen;				// ǰ���볤��
			uint8	nPreDectLen;			// ǰ����������
			uint32	SysWord;				// ͬ����
			uint32	TxHeader;				// ��������ͷ
			uint32 	RxHeader;				// ����У��������ͷ
			uint32 	RxHeaderCheckEn;		// ����У��������ͷʹ��λ����
			uint8	bFixPkLen;				// �Ƿ�ʹ�ù̶�������
			uint8	TrxPkLen;				// ���ͽ��հ�����
���ṹ��Preamble -> Sync Word -> TX Header -> Packet Length -> Data -> CRC
		(1-512B)	( 1-4B  )	 ( 0-4B  )	  (   0or1B   )	   (***B)  (0or2B)
������
Recommended preamble length: (�ο�Table 49. Minimum Receiver settling time)
Mode 					Arst		Rpl when pdt = 8b 		Rpl when pdt = 16b
(G)FSK AFC Disabled 	1 byte 		2 byte 					3 byte
(G)FSK AFC Enabled 		2 byte 		3 byte 					4 byte
������
Recommended preamble length: (�ο�Table 15. Minimum Receiver Settling Time)
Mode 					Arst		Rpl when pdt = 8b 		Rpl when pdt = 20b
(G)FSK AFC Disabled 	1 byte 		20 bits 					32 bits
(G)FSK AFC Enabled 		2 byte 		28 bits 					40 bits
****************************************************************************/
#ifdef _SI4432_SET_PACKET_HANDLER_
void Si4432SetPacketHandler(SI4432PH *pSi4432PH) reentrant
{
	uint8 tmp;
	uint32 tmp32;

	pSi4432PH = pSi4432PH;
	
	//���ð�����CRC��
	tmp = 0x00;												//��ʼֵ����������հ�������ʹ��LSB��ǰ���������Ͱ�������ֹCRC
	if(pSi4432PH->bPHEn == TRUE)	tmp = 0x88;				//���������
	if(pSi4432PH->bCRCEn == TRUE)	tmp |= 0x05;			//����CRC
	Si4432_rw(SI4432_DATA_ACCESS_CTRL, &tmp, 1, BWRITE);	//Address 30h��Data Access Control
															//Address: 31h��EZMac Status, Read Only
	//���ù㲥��ַ������ͷУ��	
	tmp = (pSi4432PH->nRxHeaderCheck) & 0x0F;				//��������ͷУ��
	tmp += (pSi4432PH->nBroad<<4) & 0xF0;	   				//�㲥��ַ����
	Si4432_rw(SI4432_HEADER_CTRL_1, &tmp, 1, BWRITE);		//Address: 32h��Header Control 1

	//����ͬ���֡���������ͷ���ȡ��Ƿ�̶������ȡ�ǰ���볤�����λ
	tmp = ((pSi4432PH->nSysWordLen-1) << 1) & 0x06;			//ͬ���ֳ���
	tmp += (pSi4432PH->nTxHeaderLen << 4) & 0xE0;			//����ͷ����
	if(pSi4432PH->bFixPkLen)	tmp |= 0x08;				//�̶�������
	if(pSi4432PH->nPreLen >255)	tmp |= 0x01;				//ǰ���볤�ȵ����λ��prealen[8]
	Si4432_rw(SI4432_HEADER_CTRL_2, &tmp, 1, BWRITE);		//Address: 33h��Header Control 2

	//����ǰ���볤�ȣ�ǰ���볤�ȵĵ�8λ��prealen[7:0]��prealen[8]��Ҫ��33h������		
	tmp = (pSi4432PH->nPreLen) & 0x00FF;			
	Si4432_rw(SI4432_PREAMBLE_LEN, &tmp, 1, BWRITE);								//Address: 34h��Preamble Length 				

	//����ǰ����������ֵ		
	tmp = ((pSi4432PH->nPreDectLen) << 3) & 0x00F8;	
	Si4432_rw(SI4432_PREAMBLE_DET_CTRL, &tmp, 1, BWRITE);							//Address: 35h��Preamble Detection Control 1

	//����ͬ��������
	tmp32 = htonl(pSi4432PH->SysWord);
	Si4432_rw(SI4432_SYNC_WORD, (uint8 *)(&(tmp32)), 4, BWRITE);		//Address: 36h-39h �� Synchronization Word 3-0

	//���÷�������ͷ
	tmp32 = htonl(pSi4432PH->TxHeader);
	Si4432_rw(SI4432_TX_HEADER, (uint8 *)(&(tmp32)), 4, BWRITE);		//Address: 3Ah-3Dh �� Transmit Header 3-0
	
	//���÷������ݰ����ȣ����ʹ�ù̶������ȣ����ͺͽ��ն�ʹ�øó��ȣ�����ֻ�Ƿ��ͳ���
	Si4432_rw(SI4432_TX_LEN, &(pSi4432PH->TrxPkLen), 1, BWRITE);					//Address: 3Eh��Transmit Packet Length

	//���ñ�������ͷ
	tmp32 = htonl(pSi4432PH->RxHeader);
	Si4432_rw(SI4432_CHECK_HEADER, (uint8 *)(&(tmp32)), 4, BWRITE);	//Address: 3Fh-42h �� Check Header 3-0
	
	//��ƽ�������ͷУ������λ
	tmp32 = htonl(pSi4432PH->RxHeaderCheckEn);
	Si4432_rw(SI4432_HEADER_EN, (uint8 *)(&(tmp32)), 4, BWRITE);	//Address: 43h-46h �� Header Enable 3-0
																			// Address: 47h-4Ah �� Received Header 3-0, Read Only
																			// Address: 4Bh��Received Packet Length, Read Only. Note: �ܱ�142���ϵ�˵������ȷ
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetRXModem()
* ��	�ܣ����ý���Modem
* ��ڲ�������
* ���ڲ�������
* ˵	��������1Ch-25h�Ĵ�����
			uint8	bAFCEn;					// �Ƿ�ʹ��AFC
			// ���¸�����ֵ���ɸ���excel��������ã������ֹ����㣬����һ����δ֪
			uint8 	dwn3_bypass;			// dwn3_bypass Bypass Decimator by 3 (if set). 
			uint8	ndec_exp;				// ndec_exp[2:0]��IF Filter Decimation Rates.
			uint8	filset;					// filset[3:0]��  IF Filter Coefficient Sets.
			uint16	rxosr;		 			// rxosr[10:0]��  Oversampling Rate.
			uint32	ncoff;					// ncoff[19:0]��  NCO Offset.
			uint16	crgain;					// crgain[10:0]�� Clock Recovery Timing Loop Gain.
****************************************************************************/
#ifdef _SI4432_SET_RX_MODEM_
void Si4432SetRXModem(SI4432RM *pSi4432RM) reentrant
{
	uint8 tmp;
	uint8 tmp2;
	uint16 rxosr;
	uint32 ncoff;
	uint16 crgain;

	pSi4432RM = pSi4432RM;

	//������Ƶ�˲�
	tmp  = ((pSi4432RM->dwn3_bypass) << 7) & 0x80;
	tmp += ((pSi4432RM->ndec_exp) << 4) & 0x70;
	tmp += (pSi4432RM->filset) & 0x0F;
	Si4432_rw(SI4432_IF_FILTER_BW, &tmp, 1, BWRITE);		// Register 1Ch. IF Filter Bandwidth��//dwn3_bypass, ndec_exp[2:0], filset[3:0]

	//����AFC��·	
	Si4432_rw(SI4432_AFC_LOOP_GEAR, &tmp, 1, BREAD); 		// Register 1Dh. AFC Loop Gearshift Override	NOTE: ʹ��Ĭ��ֵ
	if(pSi4432RM->bAFCEn == TRUE)	tmp |= 0x40;			//ʹ��AFC
	else							tmp &= ~0x40;			//��ֹAFC
	Si4432_rw(SI4432_AFC_LOOP_GEAR, &tmp, 1, BWRITE);

	// Register 1Eh. AFC Timing Control  				NOTE: ʹ��Ĭ��ֵ
	//tmp = 0x08;
	//Si4432_rw(SI4432_AFC_TIMING_CTRL, &tmp, 1, BWRITE);

	// Register 1Fh. Clock Recovery Gearshift Override	NOTE: ʹ��Ĭ��ֵ
	//tmp = 0x05;
	//Si4432_rw(SI4432_CLK_REC_GEAR, &tmp, 1, BWRITE); 

	//���ù�����
	rxosr = pSi4432RM->rxosr;
	tmp = rxosr & 0x00FF;
	Si4432_rw(SI4432_OVERSP_RATE, &tmp, 1, BWRITE);				//Register 20h. Clock Recovery Oversampling Rate
	
	//����ʱ��ָ�ƫ��	
	tmp2 = rxosr >> 8;
	tmp2 = tmp2 << 5;							
	Si4432_rw(SI4432_CLK_REC_OFFSET_2, &tmp, 1, BREAD);			//Register 21h. Clock Recovery Offset 2
	tmp	= (tmp & 0x1F) + (tmp2 & 0xE0);

	ncoff = pSi4432RM->ncoff;
	tmp2 = ncoff >> 16;
	tmp = (tmp & 0xF0) + (tmp2 & 0x0F);
	Si4432_rw(SI4432_CLK_REC_OFFSET_2, &tmp, 1, BWRITE);

	tmp = ncoff >> 8;
	Si4432_rw(SI4432_CLK_REC_OFFSET_1, &tmp, 1, BWRITE);		//Register 22h. Clock Recovery Offset 1

	tmp = ncoff & 0x000000FF;
	Si4432_rw(SI4432_CLK_REC_OFFSET_0, &tmp, 1, BWRITE);			// Register 23h. Clock Recovery Offset 0
	
	//���á�������
	crgain = pSi4432RM->crgain;
	tmp = crgain >> 8;
	tmp &= 0x07;
	Si4432_rw(SI4432_CLK_REC_GAIN_1, &tmp, 1, BWRITE); 			// Register 24h. Clock Recovery Timing Loop Gain 1

	tmp = crgain & 0x00FF;
	Si4432_rw(SI4432_CLK_REC_GAIN_0, &tmp, 1, BWRITE);			// Register 25h. Clock Recovery Timing Loop Gain 0
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetSysTiming()
* ��	�ܣ����÷��ͽ���ʱ��
* ��ڲ�����bEnVCO: �Ƿ�����VCO��nPLLTS: PLL Soft Settling Time����λ��10us, nPLLTO: PLL Settling Time����λ��10us
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_SYS_TIMING_
void Si4432SetSysTiming(uint8 bEnVCO, uint8 nPLLTS, uint8 nPLLTO) reentrant
{
	uint8 tmp;
	
	//�����Ƿ�У׼VCO
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BREAD);   	//Register 55h. Calibration Control
	if(bEnVCO == TRUE)	tmp &= ~0x01; 					//BIT[0], ʹ��У׼
	else				tmp |=  0x01;					//BIT[0], ��ʹ��У׼
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BWRITE);	
	
	//��ʱ
	nPLLTS = nPLLTS;
	nPLLTO = nPLLTO;
	tmp = 0xA2;
	Si4432_rw(0x53, &tmp, 1, BWRITE);

/*
	//PLL ʱ������
	//Register 53h. PLL Tune Time
	tmp = nPLLTS << 3;
	tmp = (tmp & 0xF8) + (nPLLTO & 0x07);
	Si4432_rw(SI4432_PLL_TUNE_TIME, &tmp, 1, BWRITE);		//0x53

	//Register 52h. TX Ramp Control 	NOTE: ��Ҫ����
	tmp = 20;							// Ĭ�ϣ�8us-5us-5us
	Si4432_rw(SI4432_TX_RAMP_CTRL, &tmp, 1, BWRITE);		//0x52
*/
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetRecvTimeOut()
* ��	�ܣ����ý��ճ�ʱʱ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_RECV_TIME_OUT_
void Si4432SetRecvTimeOut(uint16 nTimeOut) reentrant
{
	if(nTimeOut == 0)
	{
		nSi4432TimeOut = 0;
		return;
	}
	nSi4432TimeOut = (nTimeOut - 1) / 10 + 1;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetTX()
* ��	�ܣ����÷���
* ��ڲ�������
* ���ڲ�������
* ˵	���������Ƿ��Զ����ͣ��Ƿ��Զ��ط������TX FIFO����
****************************************************************************/
#ifdef _SI4432_SET_TX_
void Si4432EnableTxInt(BOOL nEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_1, &tmp, 1, BREAD);				//Register 05h. Interrupt Enable 1
	
	if(nEnable == TRUE)	 tmp |= 0x04;
	else				 tmp &= ~0x04;

	Si4432_rw(SI4432_INT_EN_1, &tmp, 1, BWRITE);			  
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetRX()
* ��	�ܣ����ý���
* ��ڲ�������
* ���ڲ�������
* ˵	���������Ƿ���ն��PACKET�����RX FIFO����
****************************************************************************/
#ifdef _SI4432_ENABLE_RX_INT_
void Si4432EnableRxInt(BOOL nEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_1, &tmp, 1, BREAD);			//Register 05h. Interrupt Enable 1

	if(nEnable == TRUE)		tmp |= 0x02;
	else				 	tmp &= ~0x02;									

	Si4432_rw(SI4432_INT_EN_1, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetInvalidPreInt()
* ��	�ܣ����ô���ǰ�����ж�
* ��ڲ�����bEnable, 0����ʹ�ܣ�1��ʹ��
* ���ڲ�������
* ˵	������⵽����ǰ���룬�Ƿ���������ж�
****************************************************************************/
#ifdef _SI4432_ENABLE_INVALIDPRE_INT_
void Si4432EnableInvalidPreInt(BOOL bEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BREAD);	// Register 06h. Interrupt Enable 2
	
	if(nEnable == TRUE)		tmp |=  0x20;			// Enable Invalid Preamble Detected.		
	else					tmp &= ~0x20;			// Enable Invalid Preamble Detected.
	
	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetValidPreInt()
* ��	�ܣ�����ǰ�����ж�
* ��ڲ�����bEnable, 0����ʹ�ܣ�1��ʹ��
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_ENABLE_VALIDPRE_INT_
void Si4432EnableValidPreInt(BOOL bEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BREAD);	// Register 06h. Interrupt Enable 2
	
	if(nEnable == TRUE)		tmp |=  0x40;			// BIT[6], Enable Valid Preamble Detected.		
	else					tmp &= ~0x40;			// BIT[6], Enable Valid Preamble Detected.
	
	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetSyncWordInt()
* ��	�ܣ�����ͬ�����ж�
* ��ڲ�����bEnable, 0����ʹ�ܣ�1��ʹ��
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_ENABLE_SYNCWORD_INT_
void Si4432EnableSyncWordInt(BOOL bEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BREAD);	// Register 06h. Interrupt Enable 2

	if(nEnable == TRUE)		tmp |=  0x80;			// BIT[8], Enable Sync Word Detected.		
	else					tmp &= ~0x80;			// BIT[8], Enable Sync Word Detected.

	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetCOLC()
* ��	�ܣ����þ����ص���
* ��ڲ�����fLC: ���ص���ֵ����λ��pF��0 - 97fF*127=12.319pF
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_COLC_
void Si4432SetCOLC(float fLC) reentrant
{
	uint8 tmp;
	
	// ����Ĵ���ֵ	
	fLC = (fLC >= 0)? fLC : 0;
	fLC = (fLC < 12.5)? fLC : 12.5;  
	tmp = fLC * 1000 / 97.0;
	tmp = (tmp < 127)? tmp : 127;
	tmp &= 0x7F;

	// ����
	Si4432_rw(SI4432_LOAD_CAP, &tmp, 1, BWRITE);	// Register 09h. 30 MHz Crystal Oscillator Load Capacitance
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetCalibration()
* ��	�ܣ�����У׼
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_CALIBRATION_
void Si4432SetCalibration(void) reentrant
{
	uint8 tmp;

	//��ֹRC��У׼��ʹ��VCO˫����У׼�����ͽ���֮ǰȡ��VCOУ׼
	tmp = 0x04;		
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BWRITE);	// Address: 55h��Calibration Control
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432Calibration()
* ��	�ܣ�У׼
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_CALIBRATE_
void Si4432Calibrate(void) reentrant
{
	uint8 tmp; 

	//����ԭʼֵ��ֻ��Ӧ�õĲ�������Ӱ������ֵ
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BREAD);	//Address: 55h��Calibration Control
	tmp |= 0x0A;									//У׼32 kHz RC Oscillator��У׼VCO 
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432Open()
* ��	�ܣ���Si4432��Դ
* ��ڲ�������
* ���ڲ�������
* ˵	������Si4432��Դ���ȴ�оƬ׼���ú�����DCB
****************************************************************************/
#ifdef _SI4432_INIT_
void Si4432Open()  reentrant
{
	uint8 tmp;

	//�����ź���
	if(pSi4432Event == NULL) 
	{
		pSi4432Event = OSSemCreate(0);
	}

	//��SI4432
#ifdef _SI4432POWER_
	SetLo(SI4432SDN);				
#endif //_SI4432POWER_
	
	//��ʱ100mS
	//0x03
	OSTimeDly(10);
	do
	{
		Si4432_rw(SI4432_INT_STATUS_2, &tmp, 1, BREAD);
	}while(!(tmp & 0x02));



	//����READY״̬
	Si4432SetMode(MODE_READY);

#ifdef	SELF_TRSW
	//����GPIOΪֱ���������
	tmp = 0x0A;
	Si4432_rw(TRSW1,&tmp,1,BWRITE);
	Si4432_rw(TRSW2,&tmp,1,BWRITE);
#endif

	//����Ϊ����(��ȫ����)
	Si4432TRSWSend();						
	
	//����DCB
	Si4432SetDCBToChip(&g_sSI4432DCB);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432Close()
* ��	�ܣ��ر�Si4432��Դ
* ��ڲ�������
* ���ڲ�������
* ˵	�����ر�Si4432��Դ֮�����мĴ������þ���ʧ�������ʹ��
****************************************************************************/
#ifdef _SI4432_CLOSE_
void Si4432Close() reentrant
{
#ifdef _SI4432POWER_
	SetHi(SI4432SDN);
#endif //_SI4432POWER_
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432Standby()
* ��	�ܣ��������ģʽ
* ��ڲ�������
* ���ڲ�������
* ˵	�������ٹ���
****************************************************************************/
#ifdef _SI4432_STANDBY_
void Si4432Standby()  reentrant
{
	//����STANDBY״̬
	Si4432SetMode(MODE_STANDBY);						 		
	//Si4432SetMode(MODE_SLEEP);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432Ready()
* ��	�ܣ�����Readyģʽ
* ��ڲ�������
* ���ڲ�������
* ˵	�������ٹ���
****************************************************************************/
#ifdef _SI4432_STANDBY_
void Si4432Ready()  reentrant
{
	//����STANDBY״̬
	Si4432SetMode(MODE_READY);						 		
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432IsChannelClear()
* ��	�ܣ�Si4432�ж��ŵ��Ƿ����
* ��ڲ�������
* ���ڲ�����ָʾ�ŵ��Ƿ���У�1�����У�2��������
* ˵	������
****************************************************************************/
#ifdef _SI4432_IS_CHANNEL_CLEARH_
BOOL Si4432IsChannelClear() reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_RX_STRENGTH, &tmp, 1, BREAD);	// Register 26h. Received Signal Strength Indicator
	
	//��Ҫ����.....
	return tmp;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432TxFifoClear()
* ��	�ܣ����TXFIFO������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_TX_FIFO_CLEAR_
void Si4432TxFifoClear(void) reentrant
{
	uint8 tmp;
	
	//���FIFO����д1����д0
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BREAD);	// Register 08h. Operating Mode and Function Control 2
	tmp |= 0x01;
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BWRITE);			

	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BREAD);	// Register 08h. Operating Mode and Function Control 2
	tmp &= ~0x01;
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432RxFifoClear()
* ��	�ܣ����RXFIFO������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_RX_FIFO_CLEAR_
void Si4432RxFifoClear(void) reentrant
{
	uint8 tmp;
	
	//���FIFO����д1����д0
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BREAD);	// Register 08h. Operating Mode and Function Control 2
	tmp |= 0x02;
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BWRITE);
		
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BREAD);	// Register 08h. Operating Mode and Function Control 2
	tmp &= ~0x02;
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetNeedFreq()
* ��	�ܣ�������ҪƵ��
* ��ڲ�����nFreq������Ƶ��
* ���ڲ�������
* ˵	�����������
****************************************************************************/
#ifdef _SI4432_SET_NEED_FREQ_
void Si4432SetNeedFreq(uint16 nFreq) reentrant
{
	if(g_sSI4432DCB.nFreq != nFreq)
	{
		g_sSI4432DCB.nFreq = nFreq;
		//����Ƶ��
		Si4432SetFreq(nFreq);
	}
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432Send()
* ��	�ܣ�Si4432��������
* ��ڲ�����pd ��д���ݵ�ָ��; nd ��д���ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SEND_
void Si4432Send(uint8 *pd, uint16 nd) reentrant
{
	uint8 nSendLen = (uint8)nd;
	uint8 nCodeType = 0x40;
//	uint16 nIdMask;

#ifdef USE_NRF_LED
	NrfLedOn();
#endif

#ifdef USE_NRZI_4432
#ifndef USE_NRZI_4432_ON_COLLECTOR
	//ӳ��ID��===================================================================
	//��ת��ID
	nIdMask = htons(RELAY_NET_ID);	  
	if((*((uint16*)pd) & nIdMask) == nIdMask)
	{
		nCodeType |= 0x20;
		*((uint16*)pd) &= ~nIdMask;
	}													  
	else
	{
		nCodeType |= 0x10;
	}

	//�ɼ���ID
	nIdMask = htons(COLLECTOR_NET_ID);
	if((*((uint16*)pd) & nIdMask) == nIdMask)
	{
		nCodeType |= 0x08;
		*((uint16*)pd) &= ~nIdMask;
	}
	else
	{
		nCodeType |= 0x04;
	}

	//ͨ����
	if((pd[2] & COLLECTOR_CHANNEL_MASK) == COLLECTOR_CHANNEL_MASK)
	{
		nCodeType |= 0x02;
		pd[2] &= ~COLLECTOR_CHANNEL_MASK;
	}
	else
	{
		nCodeType |= 0x01;
	}
#else
	nCodeType = 0x55;
#endif

	//����=====================================================================
	nSendLen = NrziEncode0(pd, nd, pSi4432SendBuff+1); 
	
	//���ñ�������
	pSi4432SendBuff[0] = nCodeType;
	nSendLen++;

	//������д�����ͻ�����=====================================================
	Si4432_rw(SI4432_FIFO_ACCESS, pSi4432SendBuff, nSendLen, BWRITE);		//0x7F
	
#else

	//������д�����ͻ�����
	Si4432_rw(SI4432_FIFO_ACCESS, pd, nd, BWRITE);		//0x7F
#endif


 	//����ǿɱ䳤�ȣ����÷��ͳ���
	if(nFixPkLen == 0)
	{
		//���÷������ݰ�����
		Si4432_rw(SI4432_TX_LEN, &nSendLen, 1, BWRITE);
	}

	//ת������ͨ��������
	Si4432TRSWSend();					

	//������
#ifdef USE_PA
	PAOn();	
	Si4432PADelay(10000);
	Si4432PADelay(10000);	
#endif

	//ʹ���ж�
	Si4432IntEnalbe();
	
	//���뷢��
	Si4432SetMode(MODE_TX);				
		
	//�޳�ʱ�ȴ��������
	OSSemPend(pSi4432Event, 0, &nSi4432Err);

	//����READY״̬
	Si4432SetMode(MODE_READY);

	//��4432�жϼĴ���(оƬ�Զ������־λ)	
	Si4432_rw(SI4432_INT_STATUS_1, (uint8 *)(&nIntStatus16), 2, BREAD);

	//�ع���
#ifdef USE_PA
	PAOff();
#endif
			
#ifdef USE_NRF_LED
	NrfLedOff();
#endif
}
#endif


/****************************************************************************
* ��	�ƣ�Si4432SendCarry()
* ��	�ܣ������ز�����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef	_SI4432_SEND_CARRY_
void Si4432SendCarry(uint16 nCycles) reentrant
{
	uint8 tmp;
	
//������
#ifdef 	USE_PA
	PAOn();
	Si4432PADelay(200);	
#endif

	//ѡ���Ͳ������ز���ʽ��ѡ��������ԴΪֱ�ӷ�ʽ
	//���õ��Ʒ�ʽ�� MT_NONE��Unmodulated Carrier����MDS_DIRECT_GPIO��Direct Mode using TX_Data via GPIO pin��
	tmp = 0x00;
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BWRITE);		// Register 71h. Modulation Mode Control 2

	//--------��ʼ����--------
	//ת������ͨ��������
	Si4432TRSWSend();					

	//�ر��ж�
	Si4432IntDisable(); 					

	//���뷢��
	Si4432SetMode(MODE_TX);				

	//һֱ����
	if(0 == nCycles) while(1);
}

void Si4432SendCarryEnd() reentrant
{
	//����READY״̬
	Si4432SetMode(MODE_READY);

	//��4432�жϼĴ���(оƬ�Զ������־λ)	
	Si4432_rw(SI4432_INT_STATUS_1, (uint8 *)(&nIntStatus16), 2, BREAD);

	//ʹ���ж�
	Si4432IntEnalbe();				
}


#endif

/****************************************************************************
* ��	�ƣ�Si4432Receive()
* ��	�ܣ�Si4432���ճ���
* ��ڲ�������
* ���ڲ��������յ����ݵĸ��������δ���ճɹ�������0
* ˵	������
****************************************************************************/
#ifdef	_SI4432_RECEIVE_
uint8 Si4432Receive() reentrant
{
	uint8 tmp;

#ifdef USE_NRZI_4432
	uint8 nCodeType;
#endif

	//���־�ͽ���FIFO
	Si4432RxFifoClear();

	//ת������ͨ��������
	Si4432TRSWReceive();

	//ʹ���ж�
	Si4432IntEnalbe();

	//�������״̬
	Si4432SetMode(MODE_RX);
	
	//�ȴ��������
	OSSemPend(pSi4432Event, nSi4432TimeOut, &nSi4432Err);

	//�˳�����ģʽ
	Si4432SetMode(MODE_READY);

	//��4432�жϼĴ���(оƬ�Զ������־λ)	
	Si4432_rw(SI4432_INT_STATUS_1, (uint8 *)(&nIntStatus16), 2, BREAD);
	nIntStatus16 = htons(nIntStatus16);

	//��������
	if(nSi4432Err == OS_NO_ERR) 
	{
		//���գ����յ���ȷ�İ�
		if((nIntStatus16 & IS_VALID_PACKET_RX) != 0)				
		{
 			//����ǹ̶������ȣ��ͷ��ظó���
			if(nFixPkLen != 0)
			{
				return nFixPkLen;
			}

			//������ǹ̶������ȣ�������յ����ݳ��ȣ����س���ֵ
			Si4432_rw(SI4432_RX_LEN, &tmp, 1, BREAD);	// Register 4Bh. Received Packet Length


#ifdef USE_NRZI_4432
			
			//�����ճ��ȣ���ֹ���������
			if(tmp == 0) return 0;
			if(tmp > SI4432_RXLEN_MAX) return 0;
			
			//��������(����SendBuff��Ϊ����)===========================================
			Si4432_rw(SI4432_FIFO_ACCESS, pSi4432SendBuff, tmp, BREAD);
			nCodeType = pSi4432SendBuff[0];
			if((nCodeType & 0x40) != 0x40) return 0;

			//����=====================================================================
			tmp = NrziDecode0(pSi4432SendBuff+1, tmp-1, pSi4432RecvBuff); 

			//ӳ��ID��=================================================================
			//��ת��ID
			if((nCodeType & 0x20) == 0x20)
			{
				*((uint16*)pSi4432RecvBuff) |= htons(RELAY_NET_ID);
				//pSi4432RecvBuff[0] |= RELAY_NET_ID >> 8;
				//pSi4432RecvBuff[1] |= RELAY_NET_ID;
			}													  
		
			//�ɼ���ID
			if((nCodeType & 0x08) == 0x08)
			{
				*((uint16*)pSi4432RecvBuff) |= htons(COLLECTOR_NET_ID);
				//pSi4432RecvBuff[0] |= COLLECTOR_NET_ID >> 8;
				//pSi4432RecvBuff[1] |= COLLECTOR_NET_ID;
			}
		
			//ͨ����
			if((nCodeType & 0x02) == 0x02)
			{
				pSi4432RecvBuff[2] |= COLLECTOR_CHANNEL_MASK;
			}

#endif

			return tmp;
		}
	}

	//���ճ�ʱ
	if(nSi4432Err == OS_TIMEOUT) return 1;

	return 0;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432ReadData()
* ��	�ܣ�Si4432����������
* ��ڲ�����pd �������ݵ�ָ��; nd �������ݵĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_READ_
void Si4432ReadData(uint8 *pd,uint16 nd) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif
#ifdef USE_NRZI_4432
	OS_ENTER_CRITICAL();
	memcpy(pd, pSi4432RecvBuff, nd);
	OS_EXIT_CRITICAL();
#else
	Si4432_rw(SI4432_FIFO_ACCESS, pd, nd, BREAD);	//0x7F
#endif
}
#endif
	
/****************************************************************************
* ��	��	Int0ISR()
* ��	�ܣ�����/�����жϳ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void EXTI0_IRQHandler()
{		 
	//�����ж�
	OSIntEnter();

    //����жϱ�־
    EXTI_ClearITPendingBit(EXTI_Line0);

	//��4432�ж�
	Si4432IntDisable();

	//�����ź���
	OSSemPost(pSi4432Event);

	//�رտ���
#ifdef USE_PA
	PAOff();					   
#endif

	//�˳��ж�
	OSIntExit();				
}

/****************************************************************************
* ��	�ƣ�Si4432_rw()
* ��	�ܣ�Si4432��д����
* ��ڲ�����addr ��д�Ĵ�����ַ; pd ����д���ݵ�ָ��; nd ����д���ݵĳ���; bNeedRead ��д����,�Ƿ���Ҫ��������
* ���ڲ�������
* ˵	����
****************************************************************************/
#ifdef _SI4432_INIT_
static void Si4432_rw(uint8 addr, uint8 *pd, uint16 nd, uint8 bNeedRead) reentrant
{
//#if OS_CRITICAL_METHOD == 3  
//	OS_CPU_SR  cpu_sr = 0;
//#endif

	uint16 i;
	uint8  trst;

	//��д����
	if(bNeedRead)	addr &= 0x7F;	//�������λΪ0
	else			addr |= 0x80;	//д�����λΪ1

	//OS_ENTER_CRITICAL();
	
	SetLo(SI4432NSEL);
	
	SPI1_RwByte(addr);
	for(i=0; i<nd; i++)
	{
		trst = SPI1_RwByte(pd[i]);
		if(bNeedRead)
		{
			pd[i] = trst;
		}
	}

	SetHi(SI4432NSEL);
	
	//OS_EXIT_CRITICAL();
}
#endif

/****************************************************************************
* ��	��	Si4432PADelay()
* ��	�ܣ���ʱ����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef USE_PA
static void Si4432PADelay(uint16 nTime)
{
	while(nTime--);
}	
#endif

//****************************************************************************************************************
//****************************************************************************************************************
//********���ӹ��ܺ���********************************************************************************************
//****************************************************************************************************************
//****************************************************************************************************************

/****************************************************************************
* ��	�ƣ�Si4432SetADC()
* ��	�ܣ�����ADC
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_ADC_
void Si4432SetADC(void) reentrant
{

}
#endif

/****************************************************************************
* ��	�ƣ�Si4432GetADC()
* ��	�ܣ���ȡADCֵ
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_GET_ADC_
uint8 Si4432GetADC(void) reentrant
{
	uint8 tmp;
	
	tmp =0;

	return tmp;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetRSSI()
* ��	�ܣ����������ź�ǿ��ָʾ����ֵ����λ��0.5dB
* ��ڲ���������ֵ
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_RSSI_GATE_
void Si4432SetRSSIGate(uint8 nStep) reentrant
{
	Si4432_rw(SI4432_RSSI_THRES, &nStep, 1, BWRITE);	// Register 27h. RSSI Threshold for Clear Channel Indicator
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432GetRSSI()
* ��	�ܣ���ȡ�����ź�ǿ��
* ��ڲ�������
* ���ڲ����������ź�ǿ��
* ˵	������
****************************************************************************/
#ifdef _SI4432_GET_RSSI_
uint8 Si4432GetRSSI(void) reentrant
{
	uint8 tmp;
	
	Si4432_rw(SI4432_RX_STRENGTH, &tmp, 1, BREAD);	// Register 26h. Received Signal Strength Indicator

	//��Ҫ���ơ�������������������������
	return tmp;
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetTS()
* ��	�ܣ������¶ȴ�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_TS_
void Si4432SetTS(void) reentrant
{
	uint8 tmp;

	Si4432_rw(0x66, &tmp, 1, BREAD); 		// ��ʹ���¶ȴ������Ĵ���
	tmp |= 0x20;							// �޸�ֵ��ʹ���¶ȴ�����
	Si4432_rw(0x66, &tmp, 1, BWRITE); 		// дʹ���¶ȴ������Ĵ���

	tmp = 0x00;
	Si4432_rw(0x0F, &tmp, 1, BWRITE); 		// ����ADC�����¶ȴ�������ADC�ο���ѹ
	tmp = 0x20;
	Si4432_rw(0x12, &tmp, 1, BWRITE); 		// �����¶ȴ������¶ȷ�Χ��ƫ��	
	
	tmp = 0x80;
	Si4432_rw(0x0F, &tmp, 1, BWRITE); 		// ��ʼ�ɼ�		
	Si4432_rw(0x11, &tmp, 1, BREAD); 		// ���¶�ֵ
	Si4432_rw(0x13, &tmp, 1, BREAD); 		// ���¶�ƫ��
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432GetTemp()
* ��	�ܣ����¶�ֵ
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_TS_
float Si4432GetTemp(void) reentrant
{
	uint8 tmp;
	float f;

	tmp = 0x80;
	Si4432_rw(0x0F, &tmp, 1, BWRITE); 	// ��ʼ�ɼ�		
	Si4432_rw(0x11, &tmp, 1, BREAD); 	// ���¶�ֵ
	//Si4432_rw(0x13, &tmp, 1, BREAD); 	// ���¶�ƫ��

	f = 0.5 * tmp - 64;
	
	return f;
}
#endif


/****************************************************************************
* ��	�ƣ�Si4432SetGPIO()
* ��	�ܣ�����GPIO
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_GPIO_
void Si4432SetGPIO(void) reentrant
{
	uint8 tmp;

	// ��GPIO����
	//Si4432_rw(0x0B, &tmp, 1, BREAD);			// ��
	//Si4432_rw(0x0C, &tmp, 1, BREAD);			// ��
	//Si4432_rw(0x0D, &tmp, 1, BREAD);			// ��
	tmp = 0x1F;				// ������ΪGND
	Si4432_rw(0x0B, &tmp, 1, BWRITE);			// д
	Si4432_rw(0x0C, &tmp, 1, BWRITE);			// д
	Si4432_rw(0x0D, &tmp, 1, BWRITE);			// д

}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetLDO()
* ��	�ܣ�����LDO
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_LDO_
void Si4432SetLDO(void) reentrant
{

}
#endif


/****************************************************************************
* ��	�ƣ�Si4432SetWUT()
* ��	�ܣ����û��Ѷ�ʱ��
* ��ڲ�������Ҫ��ʱ��ʱ�䣬��λ���루S������Χ��0 - 65535*32=2097120(S)=34952(M)=582.53(H)=24.27(D)
* ���ڲ�������
* ˵	�������Ѷ�ʱ��ֻ��SLEEPģʽ��������
****************************************************************************/
#ifdef _SI4432_SET_WUT_
void Si4432SetWUT(uint32 nTimeOut) reentrant
{
	uint8 tmp;
	uint16 ntime;
	uint8 b,r;

	nTimeOut = (nTimeOut <= 2097120)? nTimeOut : 2097120;		// ���ܳ������ֵ
	
	tmp = (nTimeOut-1) / 65535 + 1;					// ��ȡ����
	ntime = nTimeOut / tmp;		

	//��2�Ĵη���
	b = 1;
	r = 0;
	while(b < tmp)
	{
		b <<= 1;
		r++;
	}
	
	tmp = r + 10;									// ��Ӧ��ʽ
	tmp = tmp << 2;									// �������ƶ���ָ��λ��
													
	Si4432_rw(0x14, &tmp, 1, BWRITE); 				// ����D��R
	
	ntime = htons(ntime);
	Si4432_rw(0x15, (uint8 *)(&ntime), 2, BWRITE); 	// ����M
	
	// ʹ��WUT�ж�
	Si4432_rw(0x06, &tmp, 1, BREAD); 				// ���ж�ʹ�ܼĴ��� 
	tmp |= 0x08;									// �޸�֮��ʹ��WUT�ж�
	Si4432_rw(0x06, &tmp, 1, BWRITE); 				// д��
	
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetLBD()
* ��	�ܣ����õ͵�ѹ���
* ��ڲ������͵�ѹ����ֵ
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_LBD_
void Si4432SetLBD(float nVoltage) reentrant
{
	uint8 tmp;
	uint8 i;

	nVoltage = 0;

	//ʹ�ܵ͵�ѹ���
	Si4432_rw(SI4432_MODE_CTRL_1, &tmp, 1, BREAD); 		//����ģʽ���ƼĴ���1��ֵ
	tmp |= 0x40;								   		//�޸�ֵ�����ӵ͵�ѹ��⹦��
	Si4432_rw(SI4432_MODE_CTRL_1, &tmp, 1, BWRITE); 	//д��ģʽ���ƼĴ���1��ֵ

	tmp = 0x00;
   	Si4432_rw(0x1A, &tmp, 1, BWRITE);					//д�͵�ѹ����ֵ

	NOP;NOP;NOP;

	for(i=0; i<4; i++)									//��4�����ݣ��������������ж�
	{
		Si4432_rw(0x1B, &tmp, 1, BREAD);				//����ѹֵˮƽ
	}
}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetMCUClk()
* ��	�ܣ��������ʱ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_MCU_CLK_
void Si4432SetMCUClk(void) reentrant
{

}
#endif

/****************************************************************************
* ��	�ƣ�Si4432SetLDCM()
* ��	�ܣ����õ�ռ�ձ�ģʽ
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef _SI4432_SET_LDCM_
void Si4432SetLDCM(void) reentrant
{

}
#endif

