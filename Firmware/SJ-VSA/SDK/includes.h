#ifndef __INCLUDES_H__
#define __INCLUDES_H__


//Ԥ�Ȱ�����ͷ�ļ�************************************************************************************************
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_it.h"

#include <ucos_ii.h>
#include "os_cpu.h"
#include "os_cfg.h"
#include "MemAccess.h"

//data type define--------------------------------------
typedef INT8U   BYTE ;          //������ǰ�汾����������
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
//�弶������ͷ�ļ�������ʵ���õ��İ弶�����û�������
//ע���˴�������ͷ�ļ�����������BspInit()����Ҫѡ����������
#include "bsp\CpuPortAccess.h"
#include "bsp\Serial.h"
#include "bsp\SPI.h"
//#include "bsp\fsmc_nand.h"
#include "Bsp\Lights.h"
//#include "Bsp\RTC.h"
#include "bsp\bsp.h"
#include "LCD_HSD080.h"

//ͨ��Э�鹫��ͷ�ļ�
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

//����
//#include "Wireless\WirelessInterface.h"

//��ʾ
//#include "GUI\LCDDisp.h"

//����
//#include "Input\KeyDriver.h"

//USBͨ��
//#include "Communication\UsbInclude.h"

//USB SCIS�ļ�����
//#include "FS\SdScsi.h"

//�ļ�ϵͳ
//#include "FS\FileSystem.h"


#endif		    
