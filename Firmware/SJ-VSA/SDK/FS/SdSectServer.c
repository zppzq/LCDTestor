/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: SdSectServer.c
**��   ��   ��: ��п�(kady1984@163.com)
**�� �� ��  ��: 2008��04��19��
**����޸�����: 2008��04��19��
**��        ��: CF����������
*****************************************************************************************************************/
#include <stdio.h>
#include "SdSet.h"
#include "SdMmc.h"			
#include "SdSectServer.h"


//�곣������======================================================================================================
#define HIDDEN_SECTORS 	0x00 
#define DIRENTRY_SIZE 	0x20

//���ݶ���========================================================================================================
BYTE xdata Scratch[PHYSICAL_BLOCK_SIZE]; 					//���ݶ�д������ 
CDbrCompact xdata MBR;										//���������ṹ

//�ⲿ���ݶ���====================================================================================================
extern unsigned long PHYSICAL_BLOCKS;


//��������========================================================================================================
/*****************************************************************************************************************
* ��	�ƣ�SectRead()
* ��	�ܣ���ȡ���ݵ�������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned SectRead(unsigned long sector) reentrant
{
	unsigned xdata error;

	unsigned char xdata loopguard = 0;
	while ((error = SdBlockRead(sector + HIDDEN_SECTORS, Scratch)) != 0);
	
	return error;
}

/*****************************************************************************************************************
* ��	�ƣ�htonl()
* ��	�ܣ����ֽڵ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�htons()
* ��	�ܣ�˫�ֽڵ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned htons(unsigned w) reentrant
{
	unsigned rtn=0;
	rtn|=((w&0xFF00u)>>8);
	rtn|=((w&0x00FFu)<<8);
	return rtn;
}

/*****************************************************************************************************************
* ��	�ƣ�SectValidate()
* ��	�ܣ�����������Ƿ���Ч
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�SectWrite()
* ��	�ܣ�������������д������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SectWrite(unsigned long sector) reentrant
{
	
	SdBlockWrite(sector+HIDDEN_SECTORS, Scratch);


	//�����PC������ʽ���ˣ��������¼�����������
	if (sector==0)
	{
		SectValidate();
	}
}

/*****************************************************************************************************************
* ��	�ƣ�SectInit()
* ��	�ܣ�������ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SectInit(void) reentrant
{
	SdInit();					//SD����ʼ��
 	SectRead(0);				//��ȡ��һ������(������)
	SectValidate();				//��¼����������
}

/*****************************************************************************************************************
* ��	�ƣ�SectRootDir()
* ��	�ܣ���ȡ��Ŀ¼��������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef _USE_FILE_SYSTEM_
unsigned SectRootDir(void) reentrant
{
	return MBR.hidden_sectors + MBR.reserved_sectors + MBR.fat_copies*MBR.sectors_per_fat;
}

/*****************************************************************************************************************
* ��	�ƣ�SectRootDirLast()
* ��	�ܣ���ȡ��Ŀ¼�����һ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned SectRootDirLast(void)  reentrant
{
	return SectRootDir() - 1 + (MBR.root_directory_entries*DIRENTRY_SIZE) / PHYSICAL_BLOCK_SIZE;
}

/*****************************************************************************************************************
* ��	�ƣ�SectFat1()
* ��	�ܣ���ȡ�ļ�������1�ĵ�һ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned SectFat1(void) reentrant
{
	return MBR.hidden_sectors + MBR.reserved_sectors;
}

/*****************************************************************************************************************
* ��	�ƣ�SectFat2()
* ��	�ܣ���ȡ�ļ�������2�ĵ�һ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned SectFat2(void) reentrant
{
	return MBR.hidden_sectors + MBR.reserved_sectors + MBR.sectors_per_fat;
}


/*****************************************************************************************************************
* ��	�ƣ�SectFileData()
* ��	�ܣ��ļ����ݿ�ʼ�ĵ�һ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned SectFileData(void) reentrant
{
	return SectRootDirLast() + 1 - (MBR.sectors_per_cluster*2);
}


/*****************************************************************************************************************
* ��	�ƣ�SectWriteMultiFat()
* ��	�ܣ�д�ļ�������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SectWriteMultiFat(unsigned long sector) reentrant
{
	if (sector<SectFat1() || sector>=SectRootDir())
	{
		//û�����ļ��������ڣ���ͨ��д
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



