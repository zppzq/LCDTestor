/****************************************Copyright (c)************************************************************
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: global.h
**创   建   人: 杨承凯
**创 建 日  期: 2007年1月15日
**最后修改日期: 2007年4月3日
**描        述: 包括一些基本的头文件
*****************************************************************************************************************/
#ifndef		_GLOBAL_H_
#define		_GLOBAL_H_

//全局宏定义
#define SYS_OPT_SEG		idata

//预先包含的头文件************************************************************************************************
#include <stdlib.h>
#include <string.h>
#include <intrins.h>
#include "bsp\bsp.h"
#include "OsCommon\Includes.h"

//通信协议公共头文件
#include "..\..\..\Common\BrgCmdDef.h"

//user add********************************************************************************************************
//板级开发包头文件，根据实际用到的板级驱动用户自增减
//注：此处包含的头文件将决定函数BspInit()中所要选择编译的内容
#include "Bsp\spi.h"
#include "Bsp\seri1.h"
//#include "Bsp\seri2.h"
//#include "Bsp\pwm16.h"
#include "Bsp\PCA0.h"
#include "Bsp\Lights.h"
#include "Bsp\RTC.h"
#include "Bsp\PowerKey.h"

//Memorizer
#include "Memorizer\MemorierInterface.h"

//AD & DA
#include "AD_DA\AD_DAInterface.h"

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

//CRC校验
#include "Communication\CrcCheck.h"

#include "..\UserApp\VWSensorMulti.h"

// 是否使用调试模式
//#define	F350_TEST


//****************************************************************************************************************
#endif
