/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: SdSectServer.c
**创   建   人: 杨承凯(kady1984@163.com)
**创 建 日  期: 2008年04月19日
**最后修改日期: 2008年04月19日
**描        述: CF扇区服务函数
*****************************************************************************************************************/
#include <stdio.h>
#include "SdSet.h"
#include "SdMmc.h"			
#include "SdSectServer.h"


//宏常量定义======================================================================================================
#define HIDDEN_SECTORS 	0x00 
#define DIRENTRY_SIZE 	0x20

//数据定义========================================================================================================
BYTE xdata Scratch[PHYSICAL_BLOCK_SIZE]; 					//数据读写缓冲区 
CDbrCompact xdata MBR;										//主引导区结构

//外部数据定义====================================================================================================
extern unsigned long PHYSICAL_BLOCKS;


//函数定义========================================================================================================
/*****************************************************************************************************************
* 名	称：SectRead()
* 功	能：读取数据到缓冲区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned SectRead(unsigned long sector) reentrant
{
	unsigned xdata error;

	unsigned char xdata loopguard = 0;
	while ((error = SdBlockRead(sector + HIDDEN_SECTORS, Scratch)) != 0);
	
	return error;
}

/*****************************************************************************************************************
* 名	称：htonl()
* 功	能：四字节倒序
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
DWORD htonl(DWORD d) reentrant
{
	DWORD rtn=0;
	rtn|=((d&0xFF000000L)>>24);
	rtn|=((d&0x00FF0000L)>> 8);
	rtn|=((d&0x0000FF00L)<< 8);
	rtn|=((d&0x000000FFL)<<24);
	return rtn;
}

/*****************************************************************************************************************
* 名	称：htons()
* 功	能：双字节倒序
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned htons(unsigned w) reentrant
{
	unsigned rtn=0;
	rtn|=((w&0xFF00u)>>8);
	rtn|=((w&0x00FFu)<<8);
	return rtn;
}

/*****************************************************************************************************************
* 名	称：SectValidate()
* 功	能：检查引导区是否有效
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SectValidate(void) reentrant
{
	unsigned fat_sec = 0;

	CDbr* bootrecord = Scratch;
	MBR.valid=0;
	MBR.hidden_sectors = 0;

	if((bootrecord->signature[0]!=0x55) || (bootrecord->signature[1]!=0xAA))
	{
		return;
	}
	if(PHYSICAL_BLOCK_SIZE != ntohs(bootrecord->bytes_per_sector))
	{
		goto Check_MBR;
	}
	if(bootrecord->filesystem[0]!='F' || 
	   bootrecord->filesystem[1]!='A' || 
	   bootrecord->filesystem[2]!='T' || 
	   bootrecord->filesystem[3]!='1' || 
	   bootrecord->filesystem[4]!='6')
	{
		goto Check_MBR;
	}

	// Make a permanent copy of the important fields of the bootrecord:
	MBR.fat_copies = bootrecord->fat_copies;
	MBR.root_directory_entries = ntohs(bootrecord->root_directory_entries);
	MBR.number_of_sectors = ntohs(bootrecord->number_of_sectors);
	MBR.sectors_per_fat = ntohs(bootrecord->sectors_per_fat);
	MBR.total_sectors = ntohl(bootrecord->total_sectors);
	MBR.reserved_sectors = ntohs(bootrecord->reserved_sectors);
	MBR.sectors_per_cluster = bootrecord->sectors_per_cluster;
	MBR.valid=1;
	return;

Check_MBR:
	// checks if this sector is not a MBR
	if((Scratch[0x1Be] == 0x80) || (Scratch[0x1Be] == 0x00))
	{
		//	partition is active
		fat_sec = *(unsigned*)&Scratch[0x1c6];
		fat_sec = ntohs(fat_sec);
		SectRead(fat_sec);
		SectValidate();
		MBR.hidden_sectors = fat_sec;
	}
}

/*****************************************************************************************************************
* 名	称：SectWrite()
* 功	能：将缓冲区数据写入扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SectWrite(unsigned long sector) reentrant
{
	
	SdBlockWrite(sector+HIDDEN_SECTORS, Scratch);


	//如果被PC机给格式化了，必须重新加载扇区参数
	if (sector==0)
	{
		SectValidate();
	}
}

/*****************************************************************************************************************
* 名	称：SectInit()
* 功	能：扇区初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SectInit(void) reentrant
{
	SdInit();					//SD卡初始化
 	SectRead(0);				//读取第一个扇区(引导区)
	SectValidate();				//记录引导区参数
}

/*****************************************************************************************************************
* 名	称：SectRootDir()
* 功	能：获取根目录所在扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _USE_FILE_SYSTEM_
unsigned SectRootDir(void) reentrant
{
	return MBR.hidden_sectors + MBR.reserved_sectors + MBR.fat_copies*MBR.sectors_per_fat;
}

/*****************************************************************************************************************
* 名	称：SectRootDirLast()
* 功	能：获取根目录的最后一个扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned SectRootDirLast(void)  reentrant
{
	return SectRootDir() - 1 + (MBR.root_directory_entries*DIRENTRY_SIZE) / PHYSICAL_BLOCK_SIZE;
}

/*****************************************************************************************************************
* 名	称：SectFat1()
* 功	能：获取文件描述表1的第一个扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned SectFat1(void) reentrant
{
	return MBR.hidden_sectors + MBR.reserved_sectors;
}

/*****************************************************************************************************************
* 名	称：SectFat2()
* 功	能：获取文件描述表2的第一个扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned SectFat2(void) reentrant
{
	return MBR.hidden_sectors + MBR.reserved_sectors + MBR.sectors_per_fat;
}


/*****************************************************************************************************************
* 名	称：SectFileData()
* 功	能：文件数据开始的第一个扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned SectFileData(void) reentrant
{
	return SectRootDirLast() + 1 - (MBR.sectors_per_cluster*2);
}


/*****************************************************************************************************************
* 名	称：SectWriteMultiFat()
* 功	能：写文件描述表
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SectWriteMultiFat(unsigned long sector) reentrant
{
	if (sector<SectFat1() || sector>=SectRootDir())
	{
		//没有在文件描述表内，普通读写
		SectWrite(sector);
	}
	else
	{
		while(sector >= SectFat2())
		{
			sector -= MBR.sectors_per_fat;
		}
		while(sector < SectRootDir())
		{
			SectWrite(sector);
			sector += MBR.sectors_per_fat;
		}
	}
}
#endif



