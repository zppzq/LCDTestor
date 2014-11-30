/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Si4432.h
**创   建   人: 杨承凯
**最后修改日期: 2008年12月26日
**描        述: Si4432头文件
********************************************************************************************************/
#ifndef		_SI4432_H_
#define		_SI4432_H_

#ifdef		_SI4432_C_
#define		SI4432_EXT
#else
#define		SI4432_EXT	extern
#define		SI4432_LOC	extern
#endif

//Si4432包处理定义
typedef struct
{
	BOOL	bPHEn;					//是否允许包处理，只支持允许包处理
	BOOL  	bCRCEn;					//是否允许CRC校验，
	uint8 	nBroad;					//广播地址个数
	uint8	nRxHeaderCheck;			//接收数据头校验个数
	uint8 	nSysWordLen;			//同步字个数
	uint8	nTxHeaderLen;			//发送数据头长度
	uint16	nPreLen;				//前导码长度：4bits*nPreLen
	uint8	nPreDectLen;			//前导码检测门限：4bits*nPreDectLen
	uint32	SysWord;				//同步字
	uint32	TxHeader;				//发送数据头
	uint32 	RxHeader;				//接收校验用数据头
	uint32 	RxHeaderCheckEn;		//接收校验用数据头使能位控制
	BOOL	bFixPkLen;				//是否使用固定包长度
	uint8	TrxPkLen;				//发送接收包长度
} SI4432PH;

//Si4432RxModem定义
typedef struct
{
	BOOL	bAFCEn;					//是否使能AFC
	
	//以下各变量值可由附带excel表计算所得，可以手工计算，但有一参数未知
	uint8 	dwn3_bypass;			//dwn3_bypass Bypass Decimator by 3 (if set). 
	uint8	ndec_exp;				//ndec_exp[2:0]：IF Filter Decimation Rates.
	uint8	filset;					//filset[3:0]：  IF Filter Coefficient Sets.
	uint16	rxosr;		 			//rxosr[10:0]：  Oversampling Rate.
	uint32	ncoff;					//ncoff[19:0]：  NCO Offset.
	uint16	crgain;					//crgain[10:0]： Clock Recovery Timing Loop Gain.
} SI4432RM;

//Si4432设备控制模块定义
typedef struct 
{
	uint16	nFreq;					//Si4432频率：			单位：0.1MHz
	uint8	nFreqStep;				//Si4432频率步进：		10kHz*nFreqStep
	uint8	nChannel;				//Si4432频率通道：		0-255
	uint16	nFreqDeviation;			//Si4432频率偏离值：	625Hz*nFreqDeviation
	uint8 	nDataRate;				//Si4432通信速率：		1-128kbps，推荐最大值选择125，测试128误码率很大，如果使能曼彻斯特编码，最大值为64
	uint8	nOutPower;				//输出功率：			单位：dbm，只能是11、14、17、20，0<=(11)<14，14<=(14)<17，17<=(17)<20，20==(20)
	uint8	nModulateType;			//调制方式：			四种：MT_NONE、MT_OOK、MT_FSK、MT_GFSK
	uint8	nModDataSource;			//调制数据源			四种：MDS_DIRECT_GPIO、MDS_DIRECT_SDI、MDS_FIFO、MDS_PN9
	BOOL	bManCheEn;				//是否使能曼彻斯特编码，注意：使能曼彻斯特编码，有效数据率将减半
	BOOL 	bVCOEn;					//是否每次校准VCO

	SI4432PH sPacketHandler;		//包处理结构
	SI4432RM sRxModem;				//接收Modem结构		
} SI4432DCB;						  


//********寄存器、状态等宏定义***********************************************
//--------寄存器地址(部分)---------------------------------------------------
#define	SI4432_DEVCIE_TYPE			0x00	//设备类型
#define	SI4432_VERSION				0x01	//版本号
#define	SI4432_DEVICE_STATUS		0x02	//设备状态
#define	SI4432_INT_STATUS_1			0x03	//中断状态1
#define	SI4432_INT_STATUS_2			0x04	//中断状态2
#define	SI4432_INT_EN_1				0x05	//中断使能1
#define	SI4432_INT_EN_2				0x06	//中断使能2
#define	SI4432_MODE_CTRL_1			0x07	//模式控制1
#define	SI4432_MODE_CTRL_2			0x08	//模式控制2
#define	SI4432_LOAD_CAP				0x09	//连接电容
#define	SI4432_GPIO_0				0x0B	//GPIO0配置
#define	SI4432_GPIO_1				0x0C	//GPIO1配置
#define	SI4432_GPIO_2				0x0D	//GPIO2配置
#define	SI4432_IO_PORT_CFG			0x0E	//IO配置
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
#define	SI4432_RX_STRENGTH			0x26	// 接收信号强度
#define	SI4432_RSSI_THRES			0x27	//RSSI Threshold for Clear Channel Indicator 

#define	SI4432_DATA_ACCESS_CTRL		0x30	//Data Access Control
#define	SI4432_EZMAC_STATUS			0x31	//EZMac Status, Read Only
#define	SI4432_HEADER_CTRL_1		0x32	//Header Control 1
#define	SI4432_HEADER_CTRL_2		0x33	//Header Control 2
#define	SI4432_PREAMBLE_LEN			0x34	//Preamble Length 前导码长度
#define	SI4432_PREAMBLE_DET_CTRL	0x35	//Preamble Length 前导码检查控制
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
#define	SI4432_MODUL_CTRL_1 		0x70	// 调制控制1
#define	SI4432_MODUL_CTRL_2			0x71	//Modulation Mode Control 2
#define	SI4432_FREQ_DEVIATION		0x72	//Frequency Deviation
#define	SI4432_FREQ_OFFSET_1		0x73	//Frequency Offset 1 ---fo[7:0]
#define	SI4432_FREQ_OFFSET_2		0x74	//Frequency Offset 2 ---fo[9:8]
#define	SI4432_FREQ_BAND_SEL		0x75	//频率带宽选择
#define	SI4432_NOM_CAR_FREQ			0x76	//Nominal Carrier Frequency ---Bit16
#define	SI4432_FREQ_CH_SEL			0x79	//Frequency Hopping Channel Select
#define	SI4432_FREQ_STEP_SIZE		0x7A	//Frequency Hopping Step Size
#define	SI4432_TX_FIFO_CTRL_1		0x7C	//TX FIFO Control 1
#define	SI4432_TX_FIFO_CTRL_2		0x7D	//TX FIFO Control 2
#define	SI4432_RX_FIFO_CTRL			0x7E	//RX FIFO Control
#define	SI4432_FIFO_ACCESS			0x7F	//FIFO Access


//--------模式状态控制（非SHUTDOWN的其他模式）-------------------------------
#define MODE_STANDBY 		0x00		//等待状态
#define MODE_SLEEP			0x20		//睡眠状态
#define MODE_SENSOR			0x40		//传感器状态	//待定
#define MODE_TX				0x08		//发送状态
#define MODE_RX				0x04		//接受状态
#define MODE_TUNE			0x02		//调谐状态
#define MODE_READY			0x01		//准备就绪状态

//--------调制类型-----------------------------------------------------------
#define MT_NONE				0x00 		//BIT[1:0]，不调制，可用于载波强度测试
#define MT_OOK				0x01		//BIT[1:0]，OOK
#define MT_FSK				0x02		//BIT[1:0]，FSK
#define MT_GFSK				0x03		//BIT[1:0]，GFSK

//--------调制数据源---------------------------------------------------------
#define MDS_DIRECT_GPIO		(0x00 << 4)	//BIT[5:4]，直接引脚方式，引脚：GPIO
#define MDS_DIRECT_SDI		(0x01 << 4)	//BIT[5:4]，直接引脚方式，引脚：SDI
#define MDS_FIFO 			(0x02 << 4)	//BIT[5:4]，FIFO方式
#define MDS_PN9 			(0x03 << 4)	//BIT[5:4]，PN9方式

//--------中断类型状态-------------------------------------------------------
#define IS_FIFO_ERROR		0x8000		//BIT[15]， FIFO溢出，包括向上，向下
#define IS_TX_FIFO_FULL		0x4000		//BIT[14]， 发送FIFO满，设定门限
#define IS_TX_FIFO_EMPTY	0x2000		//BIT[13]， 发送FIFO空，设定门限
#define IS_RX_FIFO_FULL		0x1000		//BIT[12]， 接收FIFO满，设定门限
#define IS_EXT_INT			0x0800		//BIT[11]， 外部中断
#define IS_PACKET_SENT		0x0400		//BIT[10]， 发送包完毕
#define IS_VALID_PACKET_RX	0x0200		//BIT[9]，  接收到正确的包
#define IS_CRC_ERROR		0x0100		//BIT[8]，  CRC出错
#define IS_SYNC_WORD_DECT	0x0080		//BIT[7]，  接收到同步字
#define IS_VALID_PRE_DECT	0x0040		//BIT[6]，  接收到正确度前导码
#define IS_INVALID_PRE_DECT	0x0020		//BIT[5]，  接收到错误的前导码
#define IS_RSSI_EXCEED		0x0010		//BIT[4]，  RSSI水平高于设置门限值
#define IS_WUT_EXPIRATION	0x0008		//BIT[3]，  Wake-Up-Timer溢出
#define IS_LB_DECT			0x0004		//BIT[2]，  低电压检测
#define IS_CHIP_READY		0x0002		//BIT[1]，  芯片处于READY状态
#define IS_POR				0x0001		//BIT[0]，  上电
#define IsInt(x,y,z)		((x) & (y) & (z))	// 判断中断类型：x中断使能，y中断状态，z对应位置


//***************************************************************************
//********函数声明***********************************************************
//***************************************************************************
//********编译控制***********************************************************
#define	_SI4432_INIT_						//初始化
//#define _SI4432_GET_INT_STATUS_			//获取中断状态
//#define _SI4432_GET_DEV_STATUS_			//获取设备状态
//#define _SI4432_GET_EZMAC_STATUS_			//获取EzMac状态
//#define _SI4432_GET_POWER_STATUS_			//获取电源状态

#define _SI4432_PORT_SHUT_

#define _SI4432_STANDBY_
#define _SI4432_SET_NEED_FREQ_
#define _SI4432_OPEN_			   			//打开芯片电源
#define _SI4432_CLOSE_			   			//关闭芯片电源
#define _SI4432_SET_MODE_					//设置工作模式
#define _SI4432_SET_FREQUENCY_				//设置频率
#define _SI4432_SET_FHSS_					//设置频率，FHSS方式
#define _SI4432_SET_FREQUENCY_DEVIATION_	//设置频率偏离
#define _SI4432_SET_DATA_RATE_				//设置数据通信率
#define _SI4432_SET_OUT_POWER_				//设置通信速率
#define _SI4432_MODULATION_ 				//设置调制
#define _SI4432_SET_PACKET_HANDLER_			//设置包处理
#define _SI4432_SET_RX_MODEM_				//设置RX MODEM
#define _SI4432_SET_SYS_TIMING_				//设置发送接收时序
#define _SI4432_SET_RECV_TIME_OUT_ 			//设置超时时间
//#define _SI4432_SET_GPIO_					//设置GPIO 
//#define _SI4432_SET_LDO_					//设置LDO
//#define _SI4432_SET_LDCM_					//设置低占空比模式

//#define _SI4432_ENABLE_TX_INT_			//使能发送中断
//#define _SI4432_ENABLE_RX_INT_			//使能接收中断
//#define _SI4432_ENABLE_SYNCWORD_INT_ 		//使能同步字中断
//#define _SI4432_ENABLE_INVALIDPRE_INT_	//使能前导码无效中断
//#define _SI4432_ENABLE_VALIDPRE_INT_		//使能前导码有效中断


//#define _SI4432_IS_CHANNEL_CLEARH_		//判断信道是否空闲
//#define _SI4432_TX_FIFO_CLEAR_				//清除发送缓冲区
#define _SI4432_SEND_						//发送
#define _SI4432_SEND_CARRY_				//发送载波
#define _SI4432_RX_FIFO_CLEAR_				//清除接收缓冲区
#define _SI4432_RECEIVE_					//接收
#define _SI4432_READ_			   			//读取接收数据

//#define _SI4432_RESET_					//复位
//#define _SI4432_SET_CALIBRATION_			//设置校准
//#define _SI4432_CALIBRATION_				//校准

//----------------------------------------
//#define _SI4432_SET_ADC_  				//设置ADC
//#define _SI4432_GET_ADC_  				//获取ADC值
#define _SI4432_SET_RSSI_GATE_				//设置RSSI
//#define _SI4432_GET_RSSI_					//获取RSSI
//#define _SI4432_SET_TS_					//设置温度传感器
//#define _SI4432_GET_TEMP					//获取温度值
//#define _SI4432_SET_WUT_					//设置WUT
//#define _SI4432_SET_LBD_					//设置低电压检测
//#define _SI4432_SET_MCU_CLK_				//设置输出时钟

//********编译控制完毕*******************************************************


//--------主要功能函数-------------------------------------------------------
// 初始化函数
SI4432_EXT void 	Si4432PortInit() 					reentrant;		//Si4432端口初始化 
SI4432_EXT void 	Si4432VariInit() 					reentrant;		//Si4432全局变量初始化
SI4432_EXT SI4432DCB* Si4432GetDCB()					reentrant;		//设备设备控制参数
SI4432_EXT void Si4432SetDCBToChip(SI4432DCB *pBrfDcb)  reentrant;

// 状态获取函数
SI4432_EXT uint16 	Si4432GetIntStatus(void)			reentrant;		//读中断状态寄存器：INTERRUPT_STATUS_1 INTERRUPT_STATUS_2
SI4432_EXT uint8 	Si4432GetDevStatus(void)			reentrant;		//读设备状态
SI4432_EXT uint8 	Si4432GetEZMacStatus(void) 			reentrant;		//获取EzMac状态
SI4432_EXT uint8 	Si4432GetPowerStatus(void) 			reentrant;		//读电源状态

// 初始化设置相关函数
SI4432_EXT void 	Si4432SetNeedFreq(uint16 nFreq) 	reentrant;
SI4432_EXT void 	Si4432Open() 						reentrant;		//打开Si4432电源
SI4432_EXT void 	Si4432Close() 						reentrant;		//关闭Si4432电源
SI4432_EXT void		Si4432Standby()  					reentrant;		//进入空闲状态
SI4432_EXT void 	Si4432Ready()  						reentrant;		//进入Ready状态
SI4432_EXT void 	Si4432SetMode(uint8 nMode) 			reentrant;		//设置工作模式
SI4432_EXT void 	Si4432SetFreq(uint16 nFreq) 		reentrant;		//具体频率值方式设置频率
SI4432_EXT void 	Si4432SetFHSS(uint16 nBaseFreq, uint8 nStep, uint8 nCh) 	 reentrant;	//FHSS方式设置频率
SI4432_EXT void 	Si4432SetFreqDeviation(uint16 nFD) 					   		 reentrant;	//设置调制频率偏离值
SI4432_EXT void 	Si4432SetDataRate(uint8 ndr)								 reentrant;	//设置数据通信率
SI4432_EXT void 	Si4432SetOutPower(uint8 ndbm) 								 reentrant;	//设置输出功率
SI4432_EXT void 	Si4432SetModulation(uint8 mMT, uint8 nMDS, BOOL bManChe)  	 reentrant;	//设置调制模式
SI4432_EXT void 	Si4432SetPacketHandler(SI4432PH *pSi4432PH) 				 reentrant;	//设置包处理
SI4432_EXT void 	Si4432SetRXModem(SI4432RM *pSi4432RM) 						 reentrant;	//设置接收Modem
SI4432_EXT void 	Si4432SetSysTiming(uint8 bEnVCO, uint8 nPLLTS, uint8 nPLLTO) reentrant;	//设置发送接收时序
SI4432_EXT void 	Si4432SetRecvTimeOut(uint16 nTimeOut) 	reentrant; 	//设置接收超时时间

SI4432_EXT void 	Si4432EnableInvalidPreInt(BOOL bEnable) reentrant;	//设置错误前导码中断
SI4432_EXT void 	Si4432EnableValidPreInt(BOOL bEnable) 	reentrant;	//设置前导码中断  

SI4432_EXT void 	Si4432SetCOLC(float fLC) 				reentrant;	//设置晶振负载电容
SI4432_EXT void 	Si4432SetGPIO(void) 					reentrant;	//设置GPIO
SI4432_EXT void 	Si4432SetLDCM(void) 					reentrant;	//设置低占空比模式

//中断控制
SI4432_EXT void 	Si4432EnableTxInt(BOOL nEnable) 		reentrant;	//设置发送
SI4432_EXT void 	Si4432EnableRxInt(BOOL nEnable) 		reentrant;	//设置接收
SI4432_EXT void 	Si4432EnableSyncWordInt(BOOL bEnable) 	reentrant;	//设置同步字中断

//发送接收相关函数
SI4432_EXT BOOL 	Si4432IsChannelClear()					reentrant;	//Si4432判断信道是否空闲
SI4432_EXT void 	Si4432TxFifoClear(void) 				reentrant;	//清除TXFIFO的内容
SI4432_EXT void 	Si4432RxFifoClear(void)		 			reentrant;	//清除RXFIFO的内容
SI4432_EXT void 	Si4432Send(uint8 *pd,uint16 nd) 		reentrant;	//Si4432发送数据
SI4432_EXT void 	Si4432SendCarry(uint16 nCycles) 		reentrant;	//发送载波，供测试用
SI4432_EXT void 	Si4432SendCarryEnd() 					reentrant;	//停止发送载波，供测试用
SI4432_EXT uint8 	Si4432Receive() 						reentrant;	//Si4432接收程序
SI4432_EXT void 	Si4432ReadData(uint8 *pd,uint16 nd)		reentrant;	//Si4432读接收数据
static void 		Si4432_rw(uint8 addr, uint8 *pd, uint16 nd, uint8 bNeedRead) reentrant;//Si4432读写函数

// 其他功能函数
SI4432_EXT void 	Si4432Reset(void) 				reentrant;		//软件复位，所有寄存器恢复默认值
SI4432_EXT void 	Si4432SetCalibration(void) 		reentrant;		//设置校准 
SI4432_EXT void 	Si4432Calibrate(void) 			reentrant;		//校准
SI4432_EXT void 	Si4432SetLDO(void) 				reentrant;		//设置LDO

//--------附加功能函数--------------------------------------------------------
SI4432_EXT void 	Si4432SetADC(void) 				reentrant;		//设置ADC
SI4432_EXT uint8 	Si4432GetADC(void) 				reentrant;		//获取ADC值
SI4432_EXT void 	Si4432SetRSSIGate(uint8 nStep) 	reentrant;		//设置无线信号强度指示门限值，单位：0.5dB
SI4432_EXT uint8 	Si4432GetRSSI(void) 			reentrant;		//获取无线信号强度
SI4432_EXT void 	Si4432SetTS(void) 				reentrant;		//设置温度传感器
SI4432_EXT float 	Si4432GetTemp(void) 			reentrant;		//读温度值
SI4432_EXT void 	Si4432SetWUT(uint32 nTimeOut)  	reentrant;		//设置唤醒定时器
SI4432_EXT void 	Si4432SetLBD(float nVoltage) 	reentrant;		//设置低电压检测
SI4432_EXT void 	Si4432SetMCUClk(void) 			reentrant;		//设置输出时钟

//端口引脚控制
void Si4432PortShut() reentrant;
void Si4432PortOpen() reentrant;

//********函数声明结束********************************************************

#endif
