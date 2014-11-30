#ifndef __INCLUDES_H__
#define __INCLUDES_H__


//预先包含的头文件************************************************************************************************
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_it.h"

#include <ucos_ii.h>
#include "os_cpu.h"
#include "os_cfg.h"
#include "MemAccess.h"

//data type define--------------------------------------
typedef INT8U   BYTE ;          //兼容以前版本的数据类型
typedef INT16U  WORD ;
typedef INT32U  LONG;
typedef	INT32U  DWORD;

typedef	INT8U	uint8;		
typedef	INT8S	int8;		
typedef	INT16U	uint16;		
typedef	INT16S	int16;		
typedef	INT32U	uint32;	
typedef	INT32S	int32;		
typedef	FP32	fp32;		
typedef	FP64	fp64;		
#define reentrant
#define SYS_OPT_SEG
#define idata
#define xdata
#define _nop_()

//user add********************************************************************************************************
//板级开发包头文件，根据实际用到的板级驱动用户自增减
//注：此处包含的头文件将决定函数BspInit()中所要选择编译的内容
#include "bsp\CpuPortAccess.h"
#include "bsp\Serial.h"
#include "bsp\SPI.h"
//#include "bsp\fsmc_nand.h"
#include "Bsp\Lights.h"
//#include "Bsp\RTC.h"
#include "bsp\bsp.h"
#include "LCD_HSD080.h"

//通信协议公共头文件
//#include "Common\BrgCmdDef.h"
#include "Common\SysVersionDef.h"
//#include "..\ColBus\ColBusSlaveAps.h"



//#include "Bsp\HardwareSpi.h"
//#include "Bsp\seri1.h"
//#include "Bsp\seri2.h"
//#include "Bsp\pwm16.h"
//#include "Bsp\PCA0.h"

//Memorizer
//#include "Memorizer\MemorierInterface.h"

//Memorizer
//#include "Disk\Disk.h"

//AD & DA
//#include "AD_DA\AD_DAInterface.h"

//无线
//#include "Wireless\WirelessInterface.h"

//显示
//#include "GUI\LCDDisp.h"

//键盘
//#include "Input\KeyDriver.h"

//USB通信
//#include "Communication\UsbInclude.h"

//USB SCIS文件服务
//#include "FS\SdScsi.h"

//文件系统
//#include "FS\FileSystem.h"


#endif		    
