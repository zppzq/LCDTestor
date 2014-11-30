/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: BrgCmdDef.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年12月18日
**最后修改日期: 2008年12月18日
**描        述: 定义通信的命令字
*****************************************************************************************************************/
//
#define KD_USE_CHINES_RES

#define DYNC_FIRST_DUMP				40


//频率分配=======================================================================
#define FREQ_HOST_DEFAULT			4332						//上层默认频率
//#define FREQ_DEVICE_DEFAULT			4330						//下层默认频率
#define FREQ_DEVICE_DEFAULT			4420					//下层默认频率
//#define FREQ_DEVICE_DEFAULT			4421					//下层默认频率


//采样率参数
#define SAMPLE_RATE_SYNC			20							//同步采样率
#define SAMPLE_SLIP_SYNC_RT			100							//实时同步采样率(100微秒)

//通信DNS分配====================================================================
#define BRG_DNS_HOST				0x55555551				
#define BRG_DNS_RELAY				0x55555552
#define BRG_DNS_RELAYLINE			0xAAAAAAA5
#define BRG_DNS_COLLECTOR			0xAAAAAAA6

//通信地址分配===================================================================
#define BRG_DERECTION_DOWN			0x8000						//通信下行标志
#define RELAY_ROL_BITS				10							//中转器ID左移位数			
#define	RELAY_NET_ID				0x7C00						//中转器广播ID
#define	COLLECTOR_NET_ID			0x03FF						//采集器广播ID
#define	BRG_CHANNEL_OFFSET			2							//通道号偏移量
#define COLLECTOR_CHANNEL_MASK		0xFF						//采集器通道掩码
#define UPDATA_AREA_BYTES			46							//上行数据区字节数
#define	GPRS_UPDATA_AREA_BYTES		1280						//GPRS上行数据区字节数
#define NRF_UP_LEN					32							//上行数据字节数
#define NRF_DOWN_LEN				16							//下行数据字节数
#define BRG_UP_MAX_LEN				80							//上行数据字节数
#define BRG_DOWN_MAX_LEN			64							//下行数据字节数
#define BRG_ADDR_LEN				3							//地址的字节数
#define BRG_CRC_LEN					2							//CRC校验的字节数
#define BRG_ADRC_LEN				(BRG_ADDR_LEN+BRG_CRC_LEN)	//地址和CRC的总字节数
#define BRG_CMD_OFFSET				BRG_ADDR_LEN				//指令字节偏移量
#define BRG_SYNC_MASK				0xFC00						//同步功能码的掩码

//长度定义=======================================================================
//#define DYNAMIC_FRAME_COUNT		12		//动态数据每帧长度
#define RELAY_BUSY_WAIT_MAX			20		//中转器等待采集器忙的最大次数	
#define RELAY_DYNC_DATA_FRAMES		100		//中转器动态数据缓冲区需存放的数据帧数

//命令字类型域===================================================================
//掩码
#define BRG_CMD_MODE_AREA		0xE0		//命令模式区
//分区段
#define BRG_CMD_MODE_LR			0x20		//本地回复
#define BRG_CMD_MODE_TR			0x40		//传输接收后回复
#define BRG_CMD_MODE_TR2		0x60		//传输接收后回复
#define BRG_CMD_MODE_TA			0xA0		//传输接收后回复还自加传输
#define BRG_CMD_MODE_TO			0xC0		//只下传不上传
#define BRG_CMD_MODE_RP			0xE0		//上传报告信息

//协议字功能划分=================================================================
//1类，内部通信，即该类命令字用于对最近设备的操作，不转发
//前三位二进制为001
#define BRG_ATTACH_DEVICE		0x20		//连接下级设备
#define BRG_HOST_BATTERY		0x21		//主机电池电量
#define BRG_REPORT_SET			0x22		//报告操作设置
#define BRG_HOST_FREG			0x23		//设置主设备频率

//2类，每次都需要无条件转发的通信，并且要回复接收到的数据
//前三位二进制为010, 011
#define BRG_SET_REG      		0x40        //中转器寄存器配置
#define BRG_CONNECT_DEVICE		0x41        //连接设备
#define BRG_READ_STATE   		0x42        //读状态指令
#define BRG_SET_FREQ     		0x43        //子网频率配置
#define BRG_SET_ZERO     		0x44        //采集器校零
#define BRG_KEEP_ALIVE   		0x45        //保持唤醒
#define BRG_STATIC_OPERATION	0x46		//静态操作
#define BRG_DYNC_OPERATION		0x47		//动态操作
#define BRG_ERASE_FLASH			0x48        //擦除采集器FLASH
#define BRG_CLR_LINK_BIT		0x49		//清除连接标志
#define BRG_RELAY_RAIN			0x4A		//让中转器发送载波
#define	BRG_CALI_VALUE			0x4B		//校准采集器
#define BRG_SEND_CARRY			0x4C		//发送载波
#define BRG_GET_POWER			0x4D		//获取电源数值(0-100)
#define BRG_RELAY_RAIN_EX		0x4F		//扩展唤醒信号
#define BRG_SLEEP_CONTROL		0x50		//休眠控制


//3类，每次都需要转发的通信，并且回复，在转发后自动发送下一帧数据，
//前三位二进制位101
#define BRG_SYNC_DYNAMIC 		0xA0        //动态采集同步信号
#define BRG_READ_DYNAMIC 		0xA1        //读动态数据

//4类，只用于下行，不上行
//前三位二进制为110
#define BRG_WAKE_UP      		0xC0        //唤醒指令

//5类，一般只用于上行的报告指令
//前三位二进制为111
#define BRG_ERR_REPORT      	0xE0        //错误报告
#define BRG_REASON_REPORT		0xE1		//可行的报告

//上行数据的状态字参数===========================================================
#define	DEVICE_DATA_VALID		0x01
#define	DEVICE_STATE_BUSY		0x02
#define	DEVICE_DYNC_OVER		0x04

//静态数据上行时状态字定义=======================================================
#define	STATIC_DATA_OK			0x01

//采集器配置字定义===============================================================
#define LOCAL_ZEROLIZED_BIT		0x01		//本地调零
#define STATIC_PREPARE_BIT		0x02		//静态预采集

//采集器运行时状态字=============================================================
#define COLLECTOR_STATE_MASK	0xFFFFFF
#define COLLECTOR_OK			0x80		//采集器是否连接上
#define COLLECTOR_ZERO			0x40		//采集器是否已经校零
#define	COLLECTOR_STATIC		0x20		//采集器是否处于静态采集
#define	COLLECTOR_DYNAMIC		0x10		//采集器是否处于动态采集
#define	COLLECTOR_BATTLOW		0x08		//采集器是否处于低电压状态
#define	COLLECTOR_FLASH_ERASED	0x04		//Flash是否已经擦除
#define	COLLECTOR_FREG_CONFIGED	0x02		//采集器频率是否已配置
#define COLLECTOR_VALUE_CALI	0x01		//校正位
#define COLLECTOR_DATA_PREP		0x0100		//预采集位
#define COLLECTOR_DATA_OK		0x0400		//采集器数据准读取完成位
#define COLLECTOR_ZERO_PREP		0x0200		//预调零位

//缓冲区操作定义=================================================================
//通信帧分配
#define BRG_FRAME_COUNT		6			//数据帧个数
#define BUFF_HOST_DOWN		0			//从PC接收到的
#define BUFF_DOWN_POOL		1 			//发送缓冲
#define BUFF_DEVICE_TRCV	2 			//设备收发区
#define BUFF_DEVICE_POOL	3			//设备缓冲区
#define BUFF_UP_POOL		4			//接收缓冲
#define BUFF_HOST_UP		5			//向PC发送数据

//通信帧状态字
#define ST_FRAME_EMPTY		0x00		//为空
#define ST_FRAME_READY		0x01		//准备好
#define ST_FRAME_TRANCE		0x02		//正在通信中

//通信帧操作字定义
#define OP_NONE				0x00		//空操作
#define OP_DOWN_BUFF		0x01		//操作下行区
#define OP_UP_BUFF			0x02		//操作上下区	
#define OP_SRC_SET			0x04		//设置源标志
#define OP_DES_SET			0x10		//设置目标区标志
#define	OP_SHIFT_CMD		OP_DOWN_BUFF|OP_SRC_SET|OP_DES_SET
#define OP_CPY_CMD			OP_DOWN_BUFF|OP_DES_SET	
#define OP_SHIFT_ALL		OP_DOWN_BUFF|OP_UP_BUFF|OP_SRC_SET|OP_DES_SET

//指令解析======================================================================
//BRG_ATTACH_DEVICE
#define ATTACH_DERECT			0x21			//设置为直连网络
#define ATTACH_RELAY			0x22			//设置为中转网

#define ATTCH_DERECT_SYS_WORD	0x7E2DD4E7
#define ATTCH_RELAY_SYS_WORD	0xD4E77E2D

//BRG_SET_REG 
#define RELAY_REG0				0x00
#define RELAY_REG1				0x01
#define RELAY_REG2				0x02

//BRG_REPORT_SET
#define ERR_REPORT_SET			0x01
#define ERR_REPORT_OPEN			0x01
#define ERR_REPORT_COLOSE		0x00

//#define BRG_SET_ZERO
#define SET_ZERO_EACH			0x01
#define CANCEL_ZERO_EACH		0x00
#define SET_ZERO_ARR			0x02
#define CANCEL_ZERO_ARR			0x03

//BRG_STATIC_OPERATION
#define BRG_START_STATIC 		0x01        //开始静态采集
#define BRG_END_STATIC   		0x02        //结束静态采集
#define BRG_READ_STATIC  		0x03        //读静态数据
#define BRG_GATHER_STATIC		0x04        //收集静态采集数据
#define BRG_PREPARE_STATIC		0x05		//预先采集
#define BRG_PREPARE_STATIC_ARR	0x06		//预先采集组
#define BRG_READ_STATIC_ARR		0x07        //读静态数据组
#define BRG_START_STATIC_ARR 	0x08        //开始静态采集组
#define BRG_END_STATIC_ARR   	0x09        //结束静态采集组

//BRG_DYNC_OPERATION
#define	BRG_DISABLE_DYNC		0x01		//禁止动态采集
#define	BRG_ENABLE_DYNC 		0x02		//使能动态采集
#define BRG_START_DYNC			0x03		//开始动态采集
#define BRG_GATHER_DYNC			0x04        //中转器收集动态数据
#define BRG_END_DYNC 			0x05        //结束动态采集
#define BRG_READ_DYNC 			0x06        //结束动态采集
#define BRG_PAUSE_DYNC			0x07		//暂停动态采集
#define BRG_PAUSE_SYNC			0x08		//暂停同步
#define BRG_PAUSE_RESUME		0x09		//暂停恢复
#define BRG_READ_INDEX			0x0A		//读取动态索引
#define BRG_ACT_DYNC			0x0B		//立刻开启动态采集
#define BRG_PAUSE_READ_INDEX	0x0C		//读取暂停后的数据索引

//BRG_RELAY_RAIN和BRG_WAKE_UP
#define WAKEUP_ONLY				0x01		//仅仅唤醒
#define WAKEUP_RESET_FREQ		0x02		//唤醒并复位频率
#define WAKEUP_SET_FREQ			0x03		//唤醒并设置频率

//BRG_ERR_REPORT
#define BRG_ERR_DOWN			0x01		//下行数据出错			
#define BRG_ERR_UP				0x02		//上行数据出错	

//BRG_CALI_VALUE
#define CALI_MODE_SELF			0x91
#define CALI_MODE_MUL			0x92
#define CALI_MODE_SET			0x93
#define CALI_MODE_FACTORY		0x94
#define CALI_MODE_GET			0x96
#define CALI_MODE_SET_VWS		0x97
#define CALI_MODE_FACTORY_VWS	0x98

//BRG_SYNC_DYNAMIC
#define SYNC_CALI_TIME			0x0400		//校准时间
#define SYNC_SAMPLE_REF			0x0800		//使用了参考频率
#define SYNC_READ_DATA			0x1000		//读数据
#define SYNC_RETURN_EMPTY		0x2000		//回复数据空
#define SYNC_DATALEN_UINT16		0x4000		//回复数据长度为2字节

//BRG_END_DYNC
#define END_DYNC_SELF			0x01
#define END_DYNC_REF			0x02

//BRG_GET_POWER
#define BRG_POWER_RELAY			0x01		//中转器电压
#define BRG_POWER_CLT			0x02		//采集器电压

//BRG_REASON_REPORT
#define BRG_REASON_BUFFERD		0x01		//被缓冲
#define BRG_REASON_EMPTY		0x02		//回传空数据

//BRG_READ_STATE
#define BRG_STATE_BATTERY 		0x01		//获取电池电压
#define BRG_STATE_SENSOR_PRE	0x02		//预查询传感器状态
#define BRG_STATE_SENSOR		0x03		//获取传感器状态
#define BRG_STATE_VERSION		0x04		//获取版本号



//关键字定义
#define BRG_ERASE_FLASH_KEY		0x3175		//Flash擦除关键字
#define BRG_CALI_KEY			0xA76E80FC	//校准关键字
#define BRG_ZERO_KEY			0x6EA7F8C0	//调零关键字
#define BRG_SLEEP_KEY			0x8EA7F6C0	//调零关键字
#define BRG_FREQ_KEY			0x5C171E0A	//频率设定关键字
#define BRG_CONNECT_KEY			0xA0E171C5	//连接设备关键字
#define BRG_RELAY_REG_KEY		0xA0E1175C	//中转器寄存器设置关键字
#define WAKEUP_RESET_KEY		0x5371		//复位唤醒关键字
#define BEAT_SET_KEY			0x32A86C5E	//休眠时间设置关键字



