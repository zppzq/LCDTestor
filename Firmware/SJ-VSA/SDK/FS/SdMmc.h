/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: SdMmc.h
**创   建   人: 杨承凯(kady1984@163.com)
**创 建 日  期: 2008年04月19日
**最后修改日期: 2008年04月19日
**描        述: CF卡物理访问函数
*****************************************************************************************************************/
#ifndef _SD_MMC_H_
#define _SD_MMC_H_

/*****************************************************************************************************************
* 名	称：SdInterfaceInit()
* 功	能：SD卡接口初始化函数(SPI初始化)
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SdInterfaceInit(void) reentrant;


/*****************************************************************************************************************
* 名	称：SdInit()
* 功	能：SD卡初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SdInit(void) reentrant; 


/*****************************************************************************************************************
* 名	称：SdBlockRead()
* 功	能：SD卡块读取函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned int SdBlockRead(unsigned long address, unsigned char *pchar) reentrant;


/*****************************************************************************************************************
* 名	称：SdBlockWrite()
* 功	能：SD卡块写入函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned char SdBlockWrite(unsigned long address, unsigned char *wdata) reentrant;

/*****************************************************************************************************************
* 名	称：SdSectors()
* 功	能：获取扇区的数目
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned long SdSectors(void) reentrant;

#endif
