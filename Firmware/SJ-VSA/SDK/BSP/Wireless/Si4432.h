/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Si4432.h
**��   ��   ��: ��п�
**����޸�����: 2008��12��26��
**��        ��: Si4432ͷ�ļ�
********************************************************************************************************/
#ifndef		_SI4432_H_
#define		_SI4432_H_

#ifdef		_SI4432_C_
#define		SI4432_EXT
#else
#define		SI4432_EXT	extern
#define		SI4432_LOC	extern
#endif

//Si4432��������
typedef struct
{
	BOOL	bPHEn;					//�Ƿ����������ֻ֧�����������
	BOOL  	bCRCEn;					//�Ƿ�����CRCУ�飬
	uint8 	nBroad;					//�㲥��ַ����
	uint8	nRxHeaderCheck;			//��������ͷУ�����
	uint8 	nSysWordLen;			//ͬ���ָ���
	uint8	nTxHeaderLen;			//��������ͷ����
	uint16	nPreLen;				//ǰ���볤�ȣ�4bits*nPreLen
	uint8	nPreDectLen;			//ǰ���������ޣ�4bits*nPreDectLen
	uint32	SysWord;				//ͬ����
	uint32	TxHeader;				//��������ͷ
	uint32 	RxHeader;				//����У��������ͷ
	uint32 	RxHeaderCheckEn;		//����У��������ͷʹ��λ����
	BOOL	bFixPkLen;				//�Ƿ�ʹ�ù̶�������
	uint8	TrxPkLen;				//���ͽ��հ�����
} SI4432PH;

//Si4432RxModem����
typedef struct
{
	BOOL	bAFCEn;					//�Ƿ�ʹ��AFC
	
	//���¸�����ֵ���ɸ���excel��������ã������ֹ����㣬����һ����δ֪
	uint8 	dwn3_bypass;			//dwn3_bypass Bypass Decimator by 3 (if set). 
	uint8	ndec_exp;				//ndec_exp[2:0]��IF Filter Decimation Rates.
	uint8	filset;					//filset[3:0]��  IF Filter Coefficient Sets.
	uint16	rxosr;		 			//rxosr[10:0]��  Oversampling Rate.
	uint32	ncoff;					//ncoff[19:0]��  NCO Offset.
	uint16	crgain;					//crgain[10:0]�� Clock Recovery Timing Loop Gain.
} SI4432RM;

//Si4432�豸����ģ�鶨��
typedef struct 
{
	uint16	nFreq;					//Si4432Ƶ�ʣ�			��λ��0.1MHz
	uint8	nFreqStep;				//Si4432Ƶ�ʲ�����		10kHz*nFreqStep
	uint8	nChannel;				//Si4432Ƶ��ͨ����		0-255
	uint16	nFreqDeviation;			//Si4432Ƶ��ƫ��ֵ��	625Hz*nFreqDeviation
	uint8 	nDataRate;				//Si4432ͨ�����ʣ�		1-128kbps���Ƽ����ֵѡ��125������128�����ʺܴ����ʹ������˹�ر��룬���ֵΪ64
	uint8	nOutPower;				//������ʣ�			��λ��dbm��ֻ����11��14��17��20��0<=(11)<14��14<=(14)<17��17<=(17)<20��20==(20)
	uint8	nModulateType;			//���Ʒ�ʽ��			���֣�MT_NONE��MT_OOK��MT_FSK��MT_GFSK
	uint8	nModDataSource;			//��������Դ			���֣�MDS_DIRECT_GPIO��MDS_DIRECT_SDI��MDS_FIFO��MDS_PN9
	BOOL	bManCheEn;				//�Ƿ�ʹ������˹�ر��룬ע�⣺ʹ������˹�ر��룬��Ч�����ʽ�����
	BOOL 	bVCOEn;					//�Ƿ�ÿ��У׼VCO

	SI4432PH sPacketHandler;		//������ṹ
	SI4432RM sRxModem;				//����Modem�ṹ		
} SI4432DCB;						  


//********�Ĵ�����״̬�Ⱥ궨��***********************************************
//--------�Ĵ�����ַ(����)---------------------------------------------------
#define	SI4432_DEVCIE_TYPE			0x00	//�豸����
#define	SI4432_VERSION				0x01	//�汾��
#define	SI4432_DEVICE_STATUS		0x02	//�豸״̬
#define	SI4432_INT_STATUS_1			0x03	//�ж�״̬1
#define	SI4432_INT_STATUS_2			0x04	//�ж�״̬2
#define	SI4432_INT_EN_1				0x05	//�ж�ʹ��1
#define	SI4432_INT_EN_2				0x06	//�ж�ʹ��2
#define	SI4432_MODE_CTRL_1			0x07	//ģʽ����1
#define	SI4432_MODE_CTRL_2			0x08	//ģʽ����2
#define	SI4432_LOAD_CAP				0x09	//���ӵ���
#define	SI4432_GPIO_0				0x0B	//GPIO0����
#define	SI4432_GPIO_1				0x0C	//GPIO1����
#define	SI4432_GPIO_2				0x0D	//GPIO2����
#define	SI4432_IO_PORT_CFG			0x0E	//IO����
#define	SI4432_IF_FILTER_BW			0x1C	//IF Filter Bandwidth
#define	SI4432_AFC_LOOP_GEAR		0x1D	//AFC Loop Gearshift Override
#define	SI4432_AFC_TIMING_CTRL		0x1E	//AFC Timing Control
#define	SI4432_CLK_REC_GEAR			0x1F	//Clock Recovery Gearshif Override
#define	SI4432_OVERSP_RATE			0x20	//Clock Recovery Oversampling Rate
#define	SI4432_CLK_REC_OFFSET_2		0x21	//Clock Recovery Offset 2
#define	SI4432_CLK_REC_OFFSET_1		0x22	//Clock Recovery Offset 1
#define	SI4432_CLK_REC_OFFSET_0		0x23	//Clock Recovery Offset 0
#define	SI4432_CLK_REC_GAIN_1		0x24	//Clock Recovery Timing Loop Gain 1
#define	SI4432_CLK_REC_GAIN_0		0x25	//Clock Recovery Timing Loop Gain 0
#define	SI4432_RX_STRENGTH			0x26	// �����ź�ǿ��
#define	SI4432_RSSI_THRES			0x27	//RSSI Threshold for Clear Channel Indicator 

#define	SI4432_DATA_ACCESS_CTRL		0x30	//Data Access Control
#define	SI4432_EZMAC_STATUS			0x31	//EZMac Status, Read Only
#define	SI4432_HEADER_CTRL_1		0x32	//Header Control 1
#define	SI4432_HEADER_CTRL_2		0x33	//Header Control 2
#define	SI4432_PREAMBLE_LEN			0x34	//Preamble Length ǰ���볤��
#define	SI4432_PREAMBLE_DET_CTRL	0x35	//Preamble Length ǰ���������
#define	SI4432_SYNC_WORD			0x36	//Sync Word ---Bit32
#define	SI4432_TX_HEADER			0x3A	//Transmit Header ---Bit32
#define	SI4432_TX_LEN				0x3E	//Transmit Packet Length
#define	SI4432_CHECK_HEADER			0x3F	//Check Header ---Bit32
#define	SI4432_HEADER_EN			0x43	//Header Enable ---Bit32
#define	SI4432_RX_HEADER			0x47	//Receive Header ---Bit32
#define	SI4432_RX_LEN				0x4B	//Receive Packet Length


#define	SI4432_TX_RAMP_CTRL			0x52	//TX Ramp Control
#define	SI4432_PLL_TUNE_TIME		0x53	//PLL Tune Time
#define	SI4432_CALIB_CTRL			0x55	//Calibration Control
#define	SI4432_PW_STATUS			0x62	//Crystal Oscillator/Power-On-Reset Control 
#define	SI4432_AGC_OVERRIDE_1		0x69	//AGC Override 1 
#define	SI4432_AGC_OVERRIDE_2		0x6A	//AGC Override 2 
#define	SI4432_TX_POWER				0x6D	//TX Power
#define	SI4432_TX_DATA_RATE			0x6E	//TX Data Rate	---16Bit
#define	SI4432_MODUL_CTRL_1 		0x70	// ���ƿ���1
#define	SI4432_MODUL_CTRL_2			0x71	//Modulation Mode Control 2
#define	SI4432_FREQ_DEVIATION		0x72	//Frequency Deviation
#define	SI4432_FREQ_OFFSET_1		0x73	//Frequency Offset 1 ---fo[7:0]
#define	SI4432_FREQ_OFFSET_2		0x74	//Frequency Offset 2 ---fo[9:8]
#define	SI4432_FREQ_BAND_SEL		0x75	//Ƶ�ʴ���ѡ��
#define	SI4432_NOM_CAR_FREQ			0x76	//Nominal Carrier Frequency ---Bit16
#define	SI4432_FREQ_CH_SEL			0x79	//Frequency Hopping Channel Select
#define	SI4432_FREQ_STEP_SIZE		0x7A	//Frequency Hopping Step Size
#define	SI4432_TX_FIFO_CTRL_1		0x7C	//TX FIFO Control 1
#define	SI4432_TX_FIFO_CTRL_2		0x7D	//TX FIFO Control 2
#define	SI4432_RX_FIFO_CTRL			0x7E	//RX FIFO Control
#define	SI4432_FIFO_ACCESS			0x7F	//FIFO Access


//--------ģʽ״̬���ƣ���SHUTDOWN������ģʽ��-------------------------------
#define MODE_STANDBY 		0x00		//�ȴ�״̬
#define MODE_SLEEP			0x20		//˯��״̬
#define MODE_SENSOR			0x40		//������״̬	//����
#define MODE_TX				0x08		//����״̬
#define MODE_RX				0x04		//����״̬
#define MODE_TUNE			0x02		//��г״̬
#define MODE_READY			0x01		//׼������״̬

//--------��������-----------------------------------------------------------
#define MT_NONE				0x00 		//BIT[1:0]�������ƣ��������ز�ǿ�Ȳ���
#define MT_OOK				0x01		//BIT[1:0]��OOK
#define MT_FSK				0x02		//BIT[1:0]��FSK
#define MT_GFSK				0x03		//BIT[1:0]��GFSK

//--------��������Դ---------------------------------------------------------
#define MDS_DIRECT_GPIO		(0x00 << 4)	//BIT[5:4]��ֱ�����ŷ�ʽ�����ţ�GPIO
#define MDS_DIRECT_SDI		(0x01 << 4)	//BIT[5:4]��ֱ�����ŷ�ʽ�����ţ�SDI
#define MDS_FIFO 			(0x02 << 4)	//BIT[5:4]��FIFO��ʽ
#define MDS_PN9 			(0x03 << 4)	//BIT[5:4]��PN9��ʽ

//--------�ж�����״̬-------------------------------------------------------
#define IS_FIFO_ERROR		0x8000		//BIT[15]�� FIFO������������ϣ�����
#define IS_TX_FIFO_FULL		0x4000		//BIT[14]�� ����FIFO�����趨����
#define IS_TX_FIFO_EMPTY	0x2000		//BIT[13]�� ����FIFO�գ��趨����
#define IS_RX_FIFO_FULL		0x1000		//BIT[12]�� ����FIFO�����趨����
#define IS_EXT_INT			0x0800		//BIT[11]�� �ⲿ�ж�
#define IS_PACKET_SENT		0x0400		//BIT[10]�� ���Ͱ����
#define IS_VALID_PACKET_RX	0x0200		//BIT[9]��  ���յ���ȷ�İ�
#define IS_CRC_ERROR		0x0100		//BIT[8]��  CRC����
#define IS_SYNC_WORD_DECT	0x0080		//BIT[7]��  ���յ�ͬ����
#define IS_VALID_PRE_DECT	0x0040		//BIT[6]��  ���յ���ȷ��ǰ����
#define IS_INVALID_PRE_DECT	0x0020		//BIT[5]��  ���յ������ǰ����
#define IS_RSSI_EXCEED		0x0010		//BIT[4]��  RSSIˮƽ������������ֵ
#define IS_WUT_EXPIRATION	0x0008		//BIT[3]��  Wake-Up-Timer���
#define IS_LB_DECT			0x0004		//BIT[2]��  �͵�ѹ���
#define IS_CHIP_READY		0x0002		//BIT[1]��  оƬ����READY״̬
#define IS_POR				0x0001		//BIT[0]��  �ϵ�
#define IsInt(x,y,z)		((x) & (y) & (z))	// �ж��ж����ͣ�x�ж�ʹ�ܣ�y�ж�״̬��z��Ӧλ��


//***************************************************************************
//********��������***********************************************************
//***************************************************************************
//********�������***********************************************************
#define	_SI4432_INIT_						//��ʼ��
//#define _SI4432_GET_INT_STATUS_			//��ȡ�ж�״̬
//#define _SI4432_GET_DEV_STATUS_			//��ȡ�豸״̬
//#define _SI4432_GET_EZMAC_STATUS_			//��ȡEzMac״̬
//#define _SI4432_GET_POWER_STATUS_			//��ȡ��Դ״̬

#define _SI4432_PORT_SHUT_

#define _SI4432_STANDBY_
#define _SI4432_SET_NEED_FREQ_
#define _SI4432_OPEN_			   			//��оƬ��Դ
#define _SI4432_CLOSE_			   			//�ر�оƬ��Դ
#define _SI4432_SET_MODE_					//���ù���ģʽ
#define _SI4432_SET_FREQUENCY_				//����Ƶ��
#define _SI4432_SET_FHSS_					//����Ƶ�ʣ�FHSS��ʽ
#define _SI4432_SET_FREQUENCY_DEVIATION_	//����Ƶ��ƫ��
#define _SI4432_SET_DATA_RATE_				//��������ͨ����
#define _SI4432_SET_OUT_POWER_				//����ͨ������
#define _SI4432_MODULATION_ 				//���õ���
#define _SI4432_SET_PACKET_HANDLER_			//���ð�����
#define _SI4432_SET_RX_MODEM_				//����RX MODEM
#define _SI4432_SET_SYS_TIMING_				//���÷��ͽ���ʱ��
#define _SI4432_SET_RECV_TIME_OUT_ 			//���ó�ʱʱ��
//#define _SI4432_SET_GPIO_					//����GPIO 
//#define _SI4432_SET_LDO_					//����LDO
//#define _SI4432_SET_LDCM_					//���õ�ռ�ձ�ģʽ

//#define _SI4432_ENABLE_TX_INT_			//ʹ�ܷ����ж�
//#define _SI4432_ENABLE_RX_INT_			//ʹ�ܽ����ж�
//#define _SI4432_ENABLE_SYNCWORD_INT_ 		//ʹ��ͬ�����ж�
//#define _SI4432_ENABLE_INVALIDPRE_INT_	//ʹ��ǰ������Ч�ж�
//#define _SI4432_ENABLE_VALIDPRE_INT_		//ʹ��ǰ������Ч�ж�


//#define _SI4432_IS_CHANNEL_CLEARH_		//�ж��ŵ��Ƿ����
//#define _SI4432_TX_FIFO_CLEAR_				//������ͻ�����
#define _SI4432_SEND_						//����
#define _SI4432_SEND_CARRY_				//�����ز�
#define _SI4432_RX_FIFO_CLEAR_				//������ջ�����
#define _SI4432_RECEIVE_					//����
#define _SI4432_READ_			   			//��ȡ��������

//#define _SI4432_RESET_					//��λ
//#define _SI4432_SET_CALIBRATION_			//����У׼
//#define _SI4432_CALIBRATION_				//У׼

//----------------------------------------
//#define _SI4432_SET_ADC_  				//����ADC
//#define _SI4432_GET_ADC_  				//��ȡADCֵ
#define _SI4432_SET_RSSI_GATE_				//����RSSI
//#define _SI4432_GET_RSSI_					//��ȡRSSI
//#define _SI4432_SET_TS_					//�����¶ȴ�����
//#define _SI4432_GET_TEMP					//��ȡ�¶�ֵ
//#define _SI4432_SET_WUT_					//����WUT
//#define _SI4432_SET_LBD_					//���õ͵�ѹ���
//#define _SI4432_SET_MCU_CLK_				//�������ʱ��

//********����������*******************************************************


//--------��Ҫ���ܺ���-------------------------------------------------------
// ��ʼ������
SI4432_EXT void 	Si4432PortInit() 					reentrant;		//Si4432�˿ڳ�ʼ�� 
SI4432_EXT void 	Si4432VariInit() 					reentrant;		//Si4432ȫ�ֱ�����ʼ��
SI4432_EXT SI4432DCB* Si4432GetDCB()					reentrant;		//�豸�豸���Ʋ���
SI4432_EXT void Si4432SetDCBToChip(SI4432DCB *pBrfDcb)  reentrant;

// ״̬��ȡ����
SI4432_EXT uint16 	Si4432GetIntStatus(void)			reentrant;		//���ж�״̬�Ĵ�����INTERRUPT_STATUS_1 INTERRUPT_STATUS_2
SI4432_EXT uint8 	Si4432GetDevStatus(void)			reentrant;		//���豸״̬
SI4432_EXT uint8 	Si4432GetEZMacStatus(void) 			reentrant;		//��ȡEzMac״̬
SI4432_EXT uint8 	Si4432GetPowerStatus(void) 			reentrant;		//����Դ״̬

// ��ʼ��������غ���
SI4432_EXT void 	Si4432SetNeedFreq(uint16 nFreq) 	reentrant;
SI4432_EXT void 	Si4432Open() 						reentrant;		//��Si4432��Դ
SI4432_EXT void 	Si4432Close() 						reentrant;		//�ر�Si4432��Դ
SI4432_EXT void		Si4432Standby()  					reentrant;		//�������״̬
SI4432_EXT void 	Si4432Ready()  						reentrant;		//����Ready״̬
SI4432_EXT void 	Si4432SetMode(uint8 nMode) 			reentrant;		//���ù���ģʽ
SI4432_EXT void 	Si4432SetFreq(uint16 nFreq) 		reentrant;		//����Ƶ��ֵ��ʽ����Ƶ��
SI4432_EXT void 	Si4432SetFHSS(uint16 nBaseFreq, uint8 nStep, uint8 nCh) 	 reentrant;	//FHSS��ʽ����Ƶ��
SI4432_EXT void 	Si4432SetFreqDeviation(uint16 nFD) 					   		 reentrant;	//���õ���Ƶ��ƫ��ֵ
SI4432_EXT void 	Si4432SetDataRate(uint8 ndr)								 reentrant;	//��������ͨ����
SI4432_EXT void 	Si4432SetOutPower(uint8 ndbm) 								 reentrant;	//�����������
SI4432_EXT void 	Si4432SetModulation(uint8 mMT, uint8 nMDS, BOOL bManChe)  	 reentrant;	//���õ���ģʽ
SI4432_EXT void 	Si4432SetPacketHandler(SI4432PH *pSi4432PH) 				 reentrant;	//���ð�����
SI4432_EXT void 	Si4432SetRXModem(SI4432RM *pSi4432RM) 						 reentrant;	//���ý���Modem
SI4432_EXT void 	Si4432SetSysTiming(uint8 bEnVCO, uint8 nPLLTS, uint8 nPLLTO) reentrant;	//���÷��ͽ���ʱ��
SI4432_EXT void 	Si4432SetRecvTimeOut(uint16 nTimeOut) 	reentrant; 	//���ý��ճ�ʱʱ��

SI4432_EXT void 	Si4432EnableInvalidPreInt(BOOL bEnable) reentrant;	//���ô���ǰ�����ж�
SI4432_EXT void 	Si4432EnableValidPreInt(BOOL bEnable) 	reentrant;	//����ǰ�����ж�  

SI4432_EXT void 	Si4432SetCOLC(float fLC) 				reentrant;	//���þ����ص���
SI4432_EXT void 	Si4432SetGPIO(void) 					reentrant;	//����GPIO
SI4432_EXT void 	Si4432SetLDCM(void) 					reentrant;	//���õ�ռ�ձ�ģʽ

//�жϿ���
SI4432_EXT void 	Si4432EnableTxInt(BOOL nEnable) 		reentrant;	//���÷���
SI4432_EXT void 	Si4432EnableRxInt(BOOL nEnable) 		reentrant;	//���ý���
SI4432_EXT void 	Si4432EnableSyncWordInt(BOOL bEnable) 	reentrant;	//����ͬ�����ж�

//���ͽ�����غ���
SI4432_EXT BOOL 	Si4432IsChannelClear()					reentrant;	//Si4432�ж��ŵ��Ƿ����
SI4432_EXT void 	Si4432TxFifoClear(void) 				reentrant;	//���TXFIFO������
SI4432_EXT void 	Si4432RxFifoClear(void)		 			reentrant;	//���RXFIFO������
SI4432_EXT void 	Si4432Send(uint8 *pd,uint16 nd) 		reentrant;	//Si4432��������
SI4432_EXT void 	Si4432SendCarry(uint16 nCycles) 		reentrant;	//�����ز�����������
SI4432_EXT void 	Si4432SendCarryEnd() 					reentrant;	//ֹͣ�����ز�����������
SI4432_EXT uint8 	Si4432Receive() 						reentrant;	//Si4432���ճ���
SI4432_EXT void 	Si4432ReadData(uint8 *pd,uint16 nd)		reentrant;	//Si4432����������
static void 		Si4432_rw(uint8 addr, uint8 *pd, uint16 nd, uint8 bNeedRead) reentrant;//Si4432��д����

// �������ܺ���
SI4432_EXT void 	Si4432Reset(void) 				reentrant;		//�����λ�����мĴ����ָ�Ĭ��ֵ
SI4432_EXT void 	Si4432SetCalibration(void) 		reentrant;		//����У׼ 
SI4432_EXT void 	Si4432Calibrate(void) 			reentrant;		//У׼
SI4432_EXT void 	Si4432SetLDO(void) 				reentrant;		//����LDO

//--------���ӹ��ܺ���--------------------------------------------------------
SI4432_EXT void 	Si4432SetADC(void) 				reentrant;		//����ADC
SI4432_EXT uint8 	Si4432GetADC(void) 				reentrant;		//��ȡADCֵ
SI4432_EXT void 	Si4432SetRSSIGate(uint8 nStep) 	reentrant;		//���������ź�ǿ��ָʾ����ֵ����λ��0.5dB
SI4432_EXT uint8 	Si4432GetRSSI(void) 			reentrant;		//��ȡ�����ź�ǿ��
SI4432_EXT void 	Si4432SetTS(void) 				reentrant;		//�����¶ȴ�����
SI4432_EXT float 	Si4432GetTemp(void) 			reentrant;		//���¶�ֵ
SI4432_EXT void 	Si4432SetWUT(uint32 nTimeOut)  	reentrant;		//���û��Ѷ�ʱ��
SI4432_EXT void 	Si4432SetLBD(float nVoltage) 	reentrant;		//���õ͵�ѹ���
SI4432_EXT void 	Si4432SetMCUClk(void) 			reentrant;		//�������ʱ��

//�˿����ſ���
void Si4432PortShut() reentrant;
void Si4432PortOpen() reentrant;

//********������������********************************************************

#endif
