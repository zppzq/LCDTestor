/****************************************Copyright (c)************************************************************
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: global.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��1��15��
**����޸�����: 2007��4��3��
**��        ��: ����һЩ������ͷ�ļ�
*****************************************************************************************************************/
#ifndef		_GLOBAL_H_
#define		_GLOBAL_H_

//ȫ�ֺ궨��
#define SYS_OPT_SEG		idata

//Ԥ�Ȱ�����ͷ�ļ�************************************************************************************************
#include <stdlib.h>
#include <string.h>
#include <intrins.h>
#include "bsp\bsp.h"
#include "OsCommon\Includes.h"

//ͨ��Э�鹫��ͷ�ļ�
#include "..\..\..\Common\BrgCmdDef.h"

//user add********************************************************************************************************
//�弶������ͷ�ļ�������ʵ���õ��İ弶�����û�������
//ע���˴�������ͷ�ļ�����������BspInit()����Ҫѡ����������
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

//CRCУ��
#include "Communication\CrcCheck.h"

#include "..\UserApp\VWSensorMulti.h"

// �Ƿ�ʹ�õ���ģʽ
//#define	F350_TEST


//****************************************************************************************************************
#endif
