/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: FileSystem.c
**创   建   人: 杨承凯(kady1984@163.com)
**创 建 日  期: 2008年04月19日
**最后修改日期: 2008年04月19日
**描        述: CF卡物理访问函数
*****************************************************************************************************************/
#include <string.h>
#include <ctype.h>
#include "reg51.h"
#include "SdSet.h"
#include "SdSectServer.h"
#include "FileSystem.h"


//结构体定义=====================================================================================================
//WORD联合体
typedef union
{
	unsigned int i;
	unsigned char c[2];
}WORD_UNION, UNICODE;

//文件夹结构定义
typedef union
{
	struct
	{
		BYTE seq_nr; 					
		UNICODE unicode1_5[5]; 		
		BYTE Attrib; 				
		BYTE Type; 					
		BYTE Checksum; 				
		UNICODE unicode6_11[6];
		unsigned StartingCluster; 	
		UNICODE unicode12_13[2];
	} lfn;
	struct
	{
		char FileName[11];					 //文件名(BYTE0-BYTE7：名称；BYTE8-BYTE10：扩展名)
		BYTE Attrib;					 //文件属性
		BYTE Reserved[10];				 //系统保留
		WORD_UNION UpdateTime; 				 //最近修改时间
		WORD_UNION UpdateDate; 				 //最近修改日期
		unsigned StartingCluster; 		 //表示文件的开始簇号
		DWORD FileSize;					 //表示文件的长度
	} sfn;

} CDirEntry;

//文件查询信息定义
typedef struct
{
	unsigned long 	Block;
	unsigned int 	Offset;
	unsigned char 	FindEmpty;
	CDirEntry* 		Direntry;
} CFileFindInfo;

//文件预查询结构
typedef struct
{
	unsigned long Block;
	unsigned int Offset;
} PREV_SEARCH;

//宏常量========================================================================================================
//文件属性
#define ATTRIB_READ_ONLY 	0x01
#define ATTRIB_HIDDEN		0x02
#define ATTRIB_SYSTEM		0x04
#define ATTRIB_LABEL		0x08
#define ATTRIB_SUBDIR		0x10
#define ATTRIB_ARCHIVE		0x20

//文件中数据索引
#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2

//文件夹状态
#define DIRECTORY_EXISTS		1
#define NO_PLACE_FOR_DIRECTORY	2
#define DIRNAME_LENGTH_ERROR	3

//内部全局变量定义===============================================================================================
static CFileFindInfo xdata gFileFindInfo;			//文件查询信息
static unsigned long xdata CurrentDirBlock;			//当前目录所处的段
static unsigned char xdata gFilePathName[200];		//当前文件路进名称


//函数定义=======================================================================================================
/*****************************************************************************************************************
* 名	称：FileSysInit()
* 功	能：文件系统初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void FileSysInit() reentrant
{
	//获取根目录所在扇区
	CurrentDirBlock = SectRootDir();
	
	//初始化当前文件路径
	strcpy(gFilePathName,"\\");
}

/*****************************************************************************************************************
* 名	称：FileNameMatch()
* 功	能：判断文件名是否匹配
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned char FileNameMatch(char* FileName, char* DirentryName) reentrant
{
	//变量定义
	unsigned char i;
	unsigned char j = FileName[0];


	for(i=0; i<8; i++)
	{
		if (DirentryName[i] == ' ' && (FileName[i] == '\0' || FileName[i] == '.'))
		{
			if (!((j == '.') && (FileName[i] == '.'))) break;
		}

		if (tolower(DirentryName[i]) != FileName[i]) return 0;
	}

	j = i + 1;

	for (i = 8; i < 11; i++)
	{
		if((FileName[j] == '\0') && (DirentryName[i] != ' ')) return 0;

		if((DirentryName[i] == ' ') && ((FileName[j] == '\0') || (FileName[j] == '.') || (FileName[j-1] == '\0'))) break;
		
		if(tolower(DirentryName[i]) != FileName[j]) return 0;
		
		j++;
	}
	return 1;
}

/*****************************************************************************************************************
* 名	称：FatChain()
* 功	能：簇链
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned long FatChain(unsigned long from, unsigned nr) reentrant
{
	unsigned int* fat_table = Scratch;
	unsigned int SectCur = 0;
	unsigned int SectPrev = 0;
	unsigned int cluster = nr / MBR.sectors_per_cluster;

	while(cluster > 0)
	{
		SectCur = SectFat1() + from / (SectBlockSize() / 2);
		if(SectCur != SectPrev)
		{
			SectPrev = SectCur; 
			SectRead(SectCur);
		}

		from = ntohs(fat_table[from%(SectBlockSize()/2)]);

		if(!(from>=2 && from<=0xFFEF))
		{
			return 0xFFFFFFFF;
		}

		cluster--;
	}
	
	from *= MBR.sectors_per_cluster;
	from += nr%MBR.sectors_per_cluster;
	
	return from;
}

/*****************************************************************************************************************
* 名	称：FatChainAlloc()
* 功	能：分配簇链
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned FatChainAlloc(unsigned from,unsigned nr) reentrant
{
	unsigned* xdata fat_table=Scratch;
	unsigned xdata sect,sect_prev=0;
	unsigned xdata index;
	unsigned xdata alloced=0xFFFF;

	//寻找空余的FAT
	for(sect = SectFat1(); nr && (sect < SectFat2()); sect++)
	{
		//读取扇区
		SectRead(sect);
		
		//跳过前两个
		for(index = ((sect == SectFat1()) ? 2 : 0); index<SectBlockSize()/2; index++)
		{
			if(fat_table[index] == 0x0000)
			{
				//找到空的FAT
				fat_table[index]=ntohs(alloced);// Allocate it (refer to previously 
				
				//分配
				alloced = (sect-SectFat1()) * (SectBlockSize()/2) + index;
				
				if(!--nr) break;
			}
		}

		//复制到FAT2
		if(alloced != 0xFFFF) SectWriteMultiFat(sect);
	}

	while((from >= 2) && (from <= 0xFFEF) && (alloced != 0xFFFF))
	{
		sect = SectFat1() + from / (SectBlockSize() / 2);
		if(sect != sect_prev)
		{
			sect_prev = sect;
			SectRead(sect);
		}
		index = from % (SectBlockSize() / 2);
		from = ntohs(fat_table[index]);

		if(from>=0xFFF8)
		{
			fat_table[index]=ntohs(alloced);
			SectWriteMultiFat(sect);
		}
	}

	return alloced;
}

/*****************************************************************************************************************
* 名	称：FatChainFree()
* 功	能：释放簇链
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static void FatChainFree(unsigned from) reentrant
{
	unsigned* xdata fat_table=Scratch;
	unsigned xdata sect,sect_prev=0;
	unsigned xdata index;

	if(from<2) return;

	sect = SectFat1() + from/(SectBlockSize()/2);

	while(1)
	{
		if(sect!=sect_prev)
		{
			SectRead(sect_prev=sect);
		}

		index = from%(SectBlockSize()/2);

		from=ntohs(fat_table[index]);

		fat_table[index]=0x0000; // Free it

		if(!(from>=2 && from<=0xFFEF))
		{
			SectWriteMultiFat(sect_prev);
			break;
		}

		sect = SectFat1() + from/(SectBlockSize()/2);
		if(sect!=sect_prev)
		{
			SectWriteMultiFat(sect_prev);
		}
	}
}

/*****************************************************************************************************************
* 名	称：GetClusterOfParentDirectory()
* 功	能：获取上级目录的开始簇
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned GetClusterOfParentDirectory() reentrant
{
	if(CurrentDirBlock == SectRootDir()) return 0;
	return (CurrentDirBlock - SectFileData()) / MBR.sectors_per_cluster;
}

/*****************************************************************************************************************
* 名	称：GetClusterFromSector()
* 功	能：获取扇区所在的簇
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned GetClusterFromSector(unsigned long sector) reentrant
{
	if(sector < (SectFileData() + 2*MBR.sectors_per_cluster)) return 0;
	return ((sector - SectFileData()) / MBR.sectors_per_cluster);
}

/*****************************************************************************************************************
* 名	称：GetFirstSector()
* 功	能：获取簇下的第一个扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned long GetFirstSector(unsigned cluster) reentrant
{
	if(cluster >= 2) return SectFileData() + cluster*MBR.sectors_per_cluster;
	else return SectRootDir();
}

/*****************************************************************************************************************
* 名	称：GetFirstBlockOfNextCluster()
* 功	能：簇链里下一个簇的第一个扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned long GetFirstBlockOfNextCluster(unsigned cluster) reentrant
{
	xdata unsigned long ret = FatChain(cluster, MBR.sectors_per_cluster);
	if(ret != 0xFFFFFFFF)
	return ret + SectFileData();
	return ret;
}

/*****************************************************************************************************************
* 名	称：GetNextCluster()
* 功	能：簇链里下一个簇的第一个扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned GetNextCluster(unsigned cluster) reentrant
{
	unsigned* xdata fat_table=Scratch;

	SectRead(SectFat1() + cluster/(SectBlockSize()/2));

	return ntohs(fat_table[cluster%(SectBlockSize()/2)]);
}

/*****************************************************************************************************************
* 名	称：GetFileName()
* 功	能：获取文件名
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _USE_DIR_ACCESS_
static void GetFileName(char* direntry_name,char* file_name) reentrant
{
	unsigned int i;
	unsigned int j = 0;
	unsigned int k = 0;

	for(i=0; i<11; i++)
	{
		if(direntry_name[i] == ' ')
		{
			j = 1;
			continue;
		}
		if(j > 0)
		{
			file_name[k++] = '.';
			j = 0;
		}
		file_name[k++] = tolower(direntry_name[i]);
	}
	file_name[k] = 0;
}
#endif

/*****************************************************************************************************************
* 名	称：GetFirstBlockDirectoryCluster()
* 功	能：获取文件夹的第一个扇区
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned long GetFirstBlockDirectoryCluster(unsigned long sector) reentrant
{
	xdata unsigned cluster = GetClusterFromSector(sector);
	xdata unsigned next_dir_cluster = GetClusterOfParentDirectory();
	while(next_dir_cluster != cluster)
	{
		next_dir_cluster = GetNextCluster(next_dir_cluster);
		if(next_dir_cluster >= 0xfff8) return next_dir_cluster;
	}
	return GetFirstSector(cluster);
}

/*****************************************************************************************************************
* 名	称：ClearCluster()
* 功	能：清除簇
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static void ClearCluster(unsigned cluster) reentrant
{
	xdata unsigned long sector = GetFirstSector(cluster);
	xdata unsigned i;
	memset(Scratch,0,512);
	for(i=0;i<MBR.sectors_per_cluster;i++)
	{
		SectWrite(sector+i);
	}
}

/*****************************************************************************************************************
* 名	称：FillDirEntry()
* 功	能：填充文件夹入口
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _USE_DIR_ACCESS_
static void FillDirEntry(CDirEntry* direntry,char* dir_name) reentrant
{
	xdata BYTE i;
	for( i = 0; i < 10; i++)
	direntry->sfn.Reserved[i] = 0;

	direntry->sfn.UpdateTime.i = gFileFindInfo.Direntry->sfn.UpdateDate.i = 0;
	direntry->sfn.FileSize = 0;

	//填充文件名
	for( i = 0; i < 11; i++ )
	{
		direntry->sfn.FileName[i] = ' ';
	}

	for( i = 0; i < 11; i++ )
	{
		if(!dir_name[i])
		break;
		direntry->sfn.FileName[i] = toupper(dir_name[i]);
	}
}
#endif

/*****************************************************************************************************************
* 名	称：FileFindValid()
* 功	能：检查是否为有效的文件
* 入口参数：pFindInfo - 文件信息
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static unsigned char FileFindValid(CFileFindInfo* pFindInfo)  reentrant
{
	char n0 = pFindInfo->Direntry->sfn.FileName[0];
	
	if(pFindInfo->FindEmpty)
	{
		return (n0 == (char)0xE5) || (n0 == '\0');
	}

	return (n0 != (char)0xE5) && (n0 > ' ') && (pFindInfo->Direntry->sfn.Attrib != 0x0F);
}

/*****************************************************************************************************************
* 名	称：FileFindNext()
* 功	能：查找下一个文件
* 入口参数：pFindInfo - 文件信息
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
BYTE FileFindNext(CFileFindInfo* pFindInfo) reentrant
{
	xdata BYTE bRoot = (CurrentDirBlock == SectRootDir());
	unsigned long xdata dir_next_cluster_block;
	unsigned long xdata next_next_block;
	xdata new_cluster;

	do
	{
		if((pFindInfo->Offset+=sizeof(CDirEntry))>=SectBlockSize())
		{
			dir_next_cluster_block = GetFirstBlockDirectoryCluster(pFindInfo->Block);
			
			if(bRoot && (pFindInfo->Block>=SectRootDirLast()))
			{
				return 0;
			}
			else if((!bRoot) && 
					(dir_next_cluster_block != 0xffffffff) &&
					(pFindInfo->Block >= (dir_next_cluster_block + MBR.sectors_per_cluster-1)))
			{
				//读下一个被此文件夹占用的扇区	
				next_next_block = GetFirstBlockOfNextCluster(GetClusterFromSector(dir_next_cluster_block));

				if(next_next_block == (0xFFFFFFFF))
				{
					if(!pFindInfo->FindEmpty) return 0;
					else
					{
						new_cluster = FatChainAlloc(GetClusterFromSector(dir_next_cluster_block),1);
						if(new_cluster == 0xFFFF) return 0;
						next_next_block = GetFirstSector(new_cluster);
						ClearCluster(new_cluster);
					}
				}

				dir_next_cluster_block = next_next_block;
				pFindInfo->Offset=0;
				SectRead(pFindInfo->Block = dir_next_cluster_block);
			}
			else
			{
				pFindInfo->Offset=0;
				SectRead(++(pFindInfo->Block));

			}
		}

		pFindInfo->Direntry = (CDirEntry*)(Scratch + pFindInfo->Offset);
	}while(!FileFindValid(pFindInfo));

	return 1;
}

/*****************************************************************************************************************
* 名	称：FileFindFirst()
* 功	能：查找第一个文件
* 入口参数：pFindInfo - 文件信息
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
BYTE FileFindFirst(CFileFindInfo* pFindInfo,BYTE empty) reentrant
{
	pFindInfo->Block = CurrentDirBlock;		//初始化的时候是指向根目录

	SectRead(pFindInfo->Block);				//读取扇区

	//获取文件夹入口结构
	pFindInfo->FindEmpty = empty;			
	pFindInfo->Offset = 0;
	pFindInfo->Direntry = (CDirEntry*)(Scratch + pFindInfo->Offset);
	
	if(FileFindValid(pFindInfo)) return 1;
	
	return FileFindNext(pFindInfo);
}

/*****************************************************************************************************************
* 名	称：ChangeDir()
* 功	能：进入文件夹
* 入口参数：DirName - 名称
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _CHANGE_DIR_
BYTE ChangeDir(char* DirName) reentrant
{
	FileFindFirst(&gFileFindInfo, 0);
	while(!FileNameMatch(DirName, gFileFindInfo.Direntry->sfn.FileName))
	{
		if(!FileFindNext(&gFileFindInfo))
		{
			return 0;
		}
	}
	if(gFileFindInfo.Direntry->sfn.Attrib & ATTRIB_SUBDIR)
	{
		if(gFileFindInfo.Direntry->sfn.StartingCluster == 0x00)
		CurrentDirBlock = SectRootDir();
		else
		CurrentDirBlock = SectFileData() +
		(htons(gFileFindInfo.Direntry->sfn.StartingCluster) * MBR.sectors_per_cluster);
		if(!strcmp(DirName,".")) return 1;
		if(!strcmp(DirName,".."))
		{
			xdata unsigned char* next,*pos = strstr(gFilePathName,"\\");

			while((next = strstr(pos,"\\")) != (NULL))
			pos = next+1;

			if(pos!=(gFilePathName+1))
			pos--;

			*pos = '\0';
		}
		else
		{
			xdata unsigned len = strlen(gFilePathName);
			if(gFilePathName[len-1] != '\\')
			{
				strcpy(&gFilePathName[len],"\\");
				len++;
			}
			strcpy(&gFilePathName[len], DirName);
		}
		return 1;
	}
	return 0;
}
#endif

/*****************************************************************************************************************
* 名	称：MakeDir()
* 功	能：创建文件夹
* 入口参数：DirName - 名称
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _MAKE_DIR_
BYTE MakeDir(char* DirName) reentrant
{
	xdata unsigned long dir_sectors;
	xdata CDirEntry* entry;
	xdata unsigned start_cluster;
	unsigned max_len = strlen(DirName);

	if((DirName == NULL) || (max_len == 0) || (max_len > 8))
	{
		return DIRNAME_LENGTH_ERROR;
	}
	start_cluster = FatChainAlloc(0,1);
	
	//检查文件夹是否存在
	FileFindFirst(&gFileFindInfo, 0);
	while(FileFindNext(&gFileFindInfo))
	{
		if(FileNameMatch(DirName,gFileFindInfo.Direntry->sfn.FileName))
		{
			return DIRECTORY_EXISTS;
		}
	}
	if(!FileFindFirst(&gFileFindInfo,1)) return NO_PLACE_FOR_DIRECTORY;;

	//填写文家夹入口
	FillDirEntry(gFileFindInfo.Direntry,DirName);

	//设置文件夹开始簇
	gFileFindInfo.Direntry->sfn.StartingCluster = htons(start_cluster);
	
	//设置文件夹属性
	gFileFindInfo.Direntry->sfn.Attrib = ATTRIB_SUBDIR;

	//将数据写入SD卡
	SectWrite(gFileFindInfo.Block);

	//清除文件夹入口
	ClearCluster(start_cluster);
	dir_sectors = SectFileData() + (start_cluster * MBR.sectors_per_cluster);

	//创建"."入口
	entry = (CDirEntry*)Scratch;
	FillDirEntry(entry,".");
	entry->sfn.StartingCluster = htons(start_cluster);
	entry->sfn.Attrib = ATTRIB_SUBDIR;

	//创建".."入口
	entry = (CDirEntry*)&Scratch[32];
	FillDirEntry(entry,"..");
	entry->sfn.StartingCluster = htons(GetClusterOfParentDirectory());
	entry->sfn.Attrib = ATTRIB_SUBDIR;

	//写入数据到SD卡
	SectWrite(dir_sectors);

	//返回
	return 0;
}
#endif

/*****************************************************************************************************************
* 名	称：RemoveDir()
* 功	能：删除文件夹
* 入口参数：DirName - 名称
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _REMOVE_DIR_
BYTE RemoveDir(char* DirName) reentrant
{
	unsigned dir_deep = 0;
	PREV_SEARCH prev_dir_block[40];
	char first_part_of_dir[20];
	char dir_tmp_name[20];
	char* tmp;

	// error if someone tries to removw root directory
	if(!strcmp(DirName,"\\")) return 0;

	if((tmp = strstr(DirName,"\\")) == NULL)
	{
		strcpy(first_part_of_dir,DirName);
	}
	else
	{
		if(tmp == DirName)
		{
			tmp = strstr(&DirName[1],"\\");
			if(tmp != NULL)
			*tmp = 0;
			strcpy(first_part_of_dir,&DirName[1]);
			if(tmp != NULL)
			*tmp = '\\';
		}
		else
		{
			*tmp = 0;
			strcpy(first_part_of_dir,DirName);
			*tmp = '\\';
		}
	}

	if(!ChangeDir(DirName)) return 0;
	if(!FileFindFirst(&gFileFindInfo,0)) return 0;
	while(1)
	{
		if(gFileFindInfo.Direntry->sfn.FileName[0]!=(char)0xE5)
		{
			if(!(gFileFindInfo.Direntry->sfn.Attrib & ATTRIB_LABEL))
			{
				if(gFileFindInfo.Direntry->sfn.Attrib & (ATTRIB_SUBDIR))
				{
					if(!FileNameMatch(".",gFileFindInfo.Direntry->sfn.FileName) &&
							!FileNameMatch("..",gFileFindInfo.Direntry->sfn.FileName))
					{
						prev_dir_block[dir_deep].Block = gFileFindInfo.Block;
						prev_dir_block[dir_deep].Offset = gFileFindInfo.Offset;
						GetFileName(gFileFindInfo.Direntry->sfn.FileName,dir_tmp_name);
						ChangeDir(dir_tmp_name);
						FileFindFirst(&gFileFindInfo,0);
						dir_deep++;
					}
				}
				else
				{
					//标记为删除状态
					gFileFindInfo.Direntry->sfn.FileName[0]=0xE5; 	//0xE5为删除标记
					SectWrite(gFileFindInfo.Block);
					FatChainFree(ntohs(gFileFindInfo.Direntry->sfn.StartingCluster));
					SectRead(gFileFindInfo.Block);
				}
			}
		}
		if(!FileFindNext(&gFileFindInfo))
		{
			if(dir_deep)
			{
				dir_deep--;
				ChangeDir("..");
				gFileFindInfo.Block = prev_dir_block[dir_deep].Block;
				gFileFindInfo.Offset = prev_dir_block[dir_deep].Offset;
				SectRead(gFileFindInfo.Block);
				gFileFindInfo.Direntry=(CDirEntry*)(Scratch+gFileFindInfo.Offset);
				gFileFindInfo.Direntry->sfn.FileName[0]=0xE5; 	//0xE5为删除标记
				SectWrite(gFileFindInfo.Block);
				FatChainFree(ntohs(gFileFindInfo.Direntry->sfn.StartingCluster));
				SectRead(gFileFindInfo.Block);
				gFileFindInfo.Direntry=(CDirEntry*)(Scratch+gFileFindInfo.Offset);
			}
			else
			{
				ChangeDir("..");
				break;
			}
		}
	}
	FileFindFirst(&gFileFindInfo,0);
	while(!FileNameMatch(first_part_of_dir,gFileFindInfo.Direntry->sfn.FileName))
	{
		if(!FileFindNext(&gFileFindInfo))
		{
			return 0;
		}
	}
	gFileFindInfo.Direntry->sfn.FileName[0]=0xE5; 	//0xE5为删除标记
	SectWrite(gFileFindInfo.Block);
	FatChainFree(ntohs(gFileFindInfo.Direntry->sfn.StartingCluster));
	return 1;
}
#endif

/*****************************************************************************************************************
* 名	称：FileCreate()
* 功	能：创建文件
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
static BYTE FileCreate(CFileFindInfo *pFindInfo, char* filename) reentrant
{
	xdata BYTE i,j;

	//寻找第一个空的文件夹入口
	if(!FileFindFirst(pFindInfo,1)) return 0;

	//填写文件夹入口
	for( i = 0; i < 10; i++)
	{
		pFindInfo->Direntry->sfn.Reserved[i] = 0;
	}

	pFindInfo->Direntry->sfn.UpdateTime.i = pFindInfo->Direntry->sfn.UpdateDate.i = 0;
	pFindInfo->Direntry->sfn.StartingCluster =
	pFindInfo->Direntry->sfn.FileSize = 0;

	//填写文件名称
	for( i = 0; i < 11; i++ )
	{
		pFindInfo->Direntry->sfn.FileName[i] = ' ';
	}

	for( j = 0; j < 20; j++ )
	{
		if(!filename[j] || filename[j] == '.') break;
		if( j < 8 )	pFindInfo->Direntry->sfn.FileName[j] = toupper(filename[j]);
	}

	if( filename[j] == '.' )
	{
		for( i = 0; i < 3; i++ )
		{
			if(!filename[j+i+1] || filename[j+i+1]=='.') break;
			pFindInfo->Direntry->sfn.FileName[8+i] = toupper(filename[j+i+1]);
		}
	}

	//设置属性
	pFindInfo->Direntry->sfn.Attrib = ATTRIB_ARCHIVE;

	//写入新数据
	SectWrite(pFindInfo->Block);

	return 1;
}

/*****************************************************************************************************************
* 名	称：FileOpen()
* 功	能：打开文件
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _FILE_OPEN_
int FileOpen(FILE* f,char* filename,char* mode) reentrant
{
	//置打开标志为0
	f->IsOpen = 0;

	//如果是写文件，先删除以前的文件
	if( mode[0] == 'w' )
	{
		FileDelete(filename);
	}

	//寻找文件
	FileFindFirst(&gFileFindInfo, 0);
	while(!FileNameMatch(filename, gFileFindInfo.Direntry->sfn.FileName))
	{
		if(!FileFindNext(&gFileFindInfo))
		{
			if(mode[0] == 'r')
			{
				return 0; 		//没找到文件
			}
			if( mode[0] == 'w' || mode[0] == 'a' )
			{
				if(!FileCreate(&gFileFindInfo, filename))
				{
					return 0; 	//创建文件失败
				}
				else
				{
					break;
				}
			}
		}
	}

	//设置文件参数
	f->SectorDirentry = gFileFindInfo.Block;
	f->OffsetDirentry = gFileFindInfo.Offset;
	f->ClusterStart = f->SectorCurrent = ntohs(gFileFindInfo.Direntry->sfn.StartingCluster);
	f->Attrib = gFileFindInfo.Direntry->sfn.Attrib;
	f->Size = ntohl(gFileFindInfo.Direntry->sfn.FileSize);

	//设置文件数据索引
	if(mode[0]=='a') f->Pos = f->Size;
	else f->Pos = 0;

	//置文件打开标志
	return f->IsOpen = 1;
}
#endif

/*****************************************************************************************************************
* 名	称：FileEOF()
* 功	能：判断文件数据是否已经到达末尾
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _FILE_EOF_
int FileEOF(FILE* f)  reentrant
{
	if(!f->IsOpen) return 1;
	return (f->Pos >= f->Size);
}
#endif

/*****************************************************************************************************************
* 名	称：FileRead()
* 功	能：读文件数据
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _FILE_READ_
unsigned FileRead(FILE* f,BYTE* buffer,unsigned count)	 reentrant
{
	unsigned xdata cnt,total_cnt=0;
	if(!f->IsOpen || !count) return 0;

	//判断是否用户调用了Scratch缓冲区来读文件，用这个缓冲区有风险
	if((buffer >= Scratch) && (buffer < Scratch+SectBlockSize()))
	count = min(count, SectBlockSize()-f->Pos % SectBlockSize());

	//从SD卡读数据
	while(count && !FileEOF(f))
	{
		f->SectorCurrent = FatChain(f->ClusterStart,f->Pos/SectBlockSize());

		SectRead(SectFileData() + f->SectorCurrent);

		cnt=min(SectBlockSize()-f->Pos%SectBlockSize(),count);
		cnt=min(cnt,f->Size-f->Pos);

		//保护
		OS_ENTER_CRITICAL();
		memmove(buffer,Scratch+f->Pos%SectBlockSize(),cnt);
		OS_EXIT_CRITICAL();
		
		//复制
		total_cnt+=cnt;
		f->Pos+=cnt;
		count-=cnt;
		buffer+=cnt;
	}

	return total_cnt;
}
#endif

/*****************************************************************************************************************
* 名	称：FileWrite()
* 功	能：写数据到文件
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _FILE_WRITE_
unsigned FileWrite(FILE* f, BYTE* buffer, unsigned count)  reentrant
{
	xdata unsigned cnt,total_cnt=0,xtra,alloced;
	xdata CDirEntry* entry;
	if(!f->IsOpen || !count) return 0;

	//分配空间
	if(f->Pos+count>f->Size)
	{
		xtra = (1+((f->Pos+count-1)/SectBlockSize())/MBR.sectors_per_cluster);
		if (f->Size > 0) xtra -= ( 1 +(( f->Size - 1 ) / SectBlockSize() ) / MBR.sectors_per_cluster);

		if(xtra > 0)
		{
			if(0xFFFF == (alloced=FatChainAlloc(f->SectorCurrent/MBR.sectors_per_cluster,xtra)))
			return 0;
		}

		//更改此文件的文件夹入口
		SectRead(f->SectorDirentry);
		entry = (CDirEntry*)(Scratch+f->OffsetDirentry);
		if((entry->sfn.FileSize==0) && (entry->sfn.StartingCluster<2 || entry->sfn.StartingCluster>=0xFFF0))
		{
			entry->sfn.StartingCluster=ntohs(f->ClusterStart=alloced);
		}
		entry->sfn.FileSize=ntohl(f->Size=f->Pos+count);
		f->Attrib=(entry->sfn.Attrib|=ATTRIB_ARCHIVE);
		SectWrite(f->SectorDirentry);
	}

	//写数据到SD卡
	while(count && !FileEOF(f))
	{
		f->SectorCurrent = FatChain(f->ClusterStart,f->Pos/SectBlockSize());
		SectRead(SectFileData() + f->SectorCurrent);

		cnt = min(SectBlockSize()-f->Pos%SectBlockSize(),count);
		cnt = min(cnt,f->Size-f->Pos);

		//保护
		OS_ENTER_CRITICAL();
		memmove(Scratch+f->Pos%SectBlockSize(),buffer,cnt);
		OS_EXIT_CRITICAL();

		SectWrite(SectFileData() + f->SectorCurrent);

		total_cnt += cnt;
		f->Pos += cnt;
		count -= cnt;
		buffer += cnt;
	}

	return total_cnt;
}
#endif

/*****************************************************************************************************************
* 名	称：FileClose()
* 功	能：关闭文件
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _FILE_OPEN_
void FileClose(FILE* f) reentrant
{
	f->IsOpen = 0;
}
#endif

/*****************************************************************************************************************
* 名	称：FileClose()
* 功	能：删除文件
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef _FILE_DELETE_
int FileDelete(char* name) reentrant
{
	FileFindFirst(&gFileFindInfo,0);
	while(!FileNameMatch(name,gFileFindInfo.Direntry->sfn.FileName))
	{
		if(!FileFindNext(&gFileFindInfo))
		{
			return 0;
		}
	}

	//如果是文件夹，不删除
	if(gFileFindInfo.Direntry->sfn.Attrib & (ATTRIB_SUBDIR|ATTRIB_LABEL)) return 0;

	//标记此文件被删除
	gFileFindInfo.Direntry->sfn.FileName[0] = 0xE5;		//0xE5为删除标记
	SectWrite(gFileFindInfo.Block);
	FatChainFree(ntohs(gFileFindInfo.Direntry->sfn.StartingCluster));

	return 1;
}
#endif

//end of file


				 

