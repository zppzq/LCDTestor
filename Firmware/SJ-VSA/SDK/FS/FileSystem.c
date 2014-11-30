/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: FileSystem.c
**��   ��   ��: ��п�(kady1984@163.com)
**�� �� ��  ��: 2008��04��19��
**����޸�����: 2008��04��19��
**��        ��: CF��������ʺ���
*****************************************************************************************************************/
#include <string.h>
#include <ctype.h>
#include "reg51.h"
#include "SdSet.h"
#include "SdSectServer.h"
#include "FileSystem.h"


//�ṹ�嶨��=====================================================================================================
//WORD������
typedef union
{
	unsigned int i;
	unsigned char c[2];
}WORD_UNION, UNICODE;

//�ļ��нṹ����
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
		char FileName[11];					 //�ļ���(BYTE0-BYTE7�����ƣ�BYTE8-BYTE10����չ��)
		BYTE Attrib;					 //�ļ�����
		BYTE Reserved[10];				 //ϵͳ����
		WORD_UNION UpdateTime; 				 //����޸�ʱ��
		WORD_UNION UpdateDate; 				 //����޸�����
		unsigned StartingCluster; 		 //��ʾ�ļ��Ŀ�ʼ�غ�
		DWORD FileSize;					 //��ʾ�ļ��ĳ���
	} sfn;

} CDirEntry;

//�ļ���ѯ��Ϣ����
typedef struct
{
	unsigned long 	Block;
	unsigned int 	Offset;
	unsigned char 	FindEmpty;
	CDirEntry* 		Direntry;
} CFileFindInfo;

//�ļ�Ԥ��ѯ�ṹ
typedef struct
{
	unsigned long Block;
	unsigned int Offset;
} PREV_SEARCH;

//�곣��========================================================================================================
//�ļ�����
#define ATTRIB_READ_ONLY 	0x01
#define ATTRIB_HIDDEN		0x02
#define ATTRIB_SYSTEM		0x04
#define ATTRIB_LABEL		0x08
#define ATTRIB_SUBDIR		0x10
#define ATTRIB_ARCHIVE		0x20

//�ļ�����������
#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2

//�ļ���״̬
#define DIRECTORY_EXISTS		1
#define NO_PLACE_FOR_DIRECTORY	2
#define DIRNAME_LENGTH_ERROR	3

//�ڲ�ȫ�ֱ�������===============================================================================================
static CFileFindInfo xdata gFileFindInfo;			//�ļ���ѯ��Ϣ
static unsigned long xdata CurrentDirBlock;			//��ǰĿ¼�����Ķ�
static unsigned char xdata gFilePathName[200];		//��ǰ�ļ�·������


//��������=======================================================================================================
/*****************************************************************************************************************
* ��	�ƣ�FileSysInit()
* ��	�ܣ��ļ�ϵͳ��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void FileSysInit() reentrant
{
	//��ȡ��Ŀ¼��������
	CurrentDirBlock = SectRootDir();
	
	//��ʼ����ǰ�ļ�·��
	strcpy(gFilePathName,"\\");
}

/*****************************************************************************************************************
* ��	�ƣ�FileNameMatch()
* ��	�ܣ��ж��ļ����Ƿ�ƥ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static unsigned char FileNameMatch(char* FileName, char* DirentryName) reentrant
{
	//��������
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
* ��	�ƣ�FatChain()
* ��	�ܣ�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�FatChainAlloc()
* ��	�ܣ��������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static unsigned FatChainAlloc(unsigned from,unsigned nr) reentrant
{
	unsigned* xdata fat_table=Scratch;
	unsigned xdata sect,sect_prev=0;
	unsigned xdata index;
	unsigned xdata alloced=0xFFFF;

	//Ѱ�ҿ����FAT
	for(sect = SectFat1(); nr && (sect < SectFat2()); sect++)
	{
		//��ȡ����
		SectRead(sect);
		
		//����ǰ����
		for(index = ((sect == SectFat1()) ? 2 : 0); index<SectBlockSize()/2; index++)
		{
			if(fat_table[index] == 0x0000)
			{
				//�ҵ��յ�FAT
				fat_table[index]=ntohs(alloced);// Allocate it (refer to previously 
				
				//����
				alloced = (sect-SectFat1()) * (SectBlockSize()/2) + index;
				
				if(!--nr) break;
			}
		}

		//���Ƶ�FAT2
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
* ��	�ƣ�FatChainFree()
* ��	�ܣ��ͷŴ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�GetClusterOfParentDirectory()
* ��	�ܣ���ȡ�ϼ�Ŀ¼�Ŀ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static unsigned GetClusterOfParentDirectory() reentrant
{
	if(CurrentDirBlock == SectRootDir()) return 0;
	return (CurrentDirBlock - SectFileData()) / MBR.sectors_per_cluster;
}

/*****************************************************************************************************************
* ��	�ƣ�GetClusterFromSector()
* ��	�ܣ���ȡ�������ڵĴ�
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static unsigned GetClusterFromSector(unsigned long sector) reentrant
{
	if(sector < (SectFileData() + 2*MBR.sectors_per_cluster)) return 0;
	return ((sector - SectFileData()) / MBR.sectors_per_cluster);
}

/*****************************************************************************************************************
* ��	�ƣ�GetFirstSector()
* ��	�ܣ���ȡ���µĵ�һ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static unsigned long GetFirstSector(unsigned cluster) reentrant
{
	if(cluster >= 2) return SectFileData() + cluster*MBR.sectors_per_cluster;
	else return SectRootDir();
}

/*****************************************************************************************************************
* ��	�ƣ�GetFirstBlockOfNextCluster()
* ��	�ܣ���������һ���صĵ�һ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static unsigned long GetFirstBlockOfNextCluster(unsigned cluster) reentrant
{
	xdata unsigned long ret = FatChain(cluster, MBR.sectors_per_cluster);
	if(ret != 0xFFFFFFFF)
	return ret + SectFileData();
	return ret;
}

/*****************************************************************************************************************
* ��	�ƣ�GetNextCluster()
* ��	�ܣ���������һ���صĵ�һ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static unsigned GetNextCluster(unsigned cluster) reentrant
{
	unsigned* xdata fat_table=Scratch;

	SectRead(SectFat1() + cluster/(SectBlockSize()/2));

	return ntohs(fat_table[cluster%(SectBlockSize()/2)]);
}

/*****************************************************************************************************************
* ��	�ƣ�GetFileName()
* ��	�ܣ���ȡ�ļ���
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�GetFirstBlockDirectoryCluster()
* ��	�ܣ���ȡ�ļ��еĵ�һ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�ClearCluster()
* ��	�ܣ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�FillDirEntry()
* ��	�ܣ�����ļ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef _USE_DIR_ACCESS_
static void FillDirEntry(CDirEntry* direntry,char* dir_name) reentrant
{
	xdata BYTE i;
	for( i = 0; i < 10; i++)
	direntry->sfn.Reserved[i] = 0;

	direntry->sfn.UpdateTime.i = gFileFindInfo.Direntry->sfn.UpdateDate.i = 0;
	direntry->sfn.FileSize = 0;

	//����ļ���
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
* ��	�ƣ�FileFindValid()
* ��	�ܣ�����Ƿ�Ϊ��Ч���ļ�
* ��ڲ�����pFindInfo - �ļ���Ϣ
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�FileFindNext()
* ��	�ܣ�������һ���ļ�
* ��ڲ�����pFindInfo - �ļ���Ϣ
* ���ڲ�������
* ˵	������
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
				//����һ�������ļ���ռ�õ�����	
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
* ��	�ƣ�FileFindFirst()
* ��	�ܣ����ҵ�һ���ļ�
* ��ڲ�����pFindInfo - �ļ���Ϣ
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
BYTE FileFindFirst(CFileFindInfo* pFindInfo,BYTE empty) reentrant
{
	pFindInfo->Block = CurrentDirBlock;		//��ʼ����ʱ����ָ���Ŀ¼

	SectRead(pFindInfo->Block);				//��ȡ����

	//��ȡ�ļ�����ڽṹ
	pFindInfo->FindEmpty = empty;			
	pFindInfo->Offset = 0;
	pFindInfo->Direntry = (CDirEntry*)(Scratch + pFindInfo->Offset);
	
	if(FileFindValid(pFindInfo)) return 1;
	
	return FileFindNext(pFindInfo);
}

/*****************************************************************************************************************
* ��	�ƣ�ChangeDir()
* ��	�ܣ������ļ���
* ��ڲ�����DirName - ����
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�MakeDir()
* ��	�ܣ������ļ���
* ��ڲ�����DirName - ����
* ���ڲ�������
* ˵	������
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
	
	//����ļ����Ƿ����
	FileFindFirst(&gFileFindInfo, 0);
	while(FileFindNext(&gFileFindInfo))
	{
		if(FileNameMatch(DirName,gFileFindInfo.Direntry->sfn.FileName))
		{
			return DIRECTORY_EXISTS;
		}
	}
	if(!FileFindFirst(&gFileFindInfo,1)) return NO_PLACE_FOR_DIRECTORY;;

	//��д�ļҼ����
	FillDirEntry(gFileFindInfo.Direntry,DirName);

	//�����ļ��п�ʼ��
	gFileFindInfo.Direntry->sfn.StartingCluster = htons(start_cluster);
	
	//�����ļ�������
	gFileFindInfo.Direntry->sfn.Attrib = ATTRIB_SUBDIR;

	//������д��SD��
	SectWrite(gFileFindInfo.Block);

	//����ļ������
	ClearCluster(start_cluster);
	dir_sectors = SectFileData() + (start_cluster * MBR.sectors_per_cluster);

	//����"."���
	entry = (CDirEntry*)Scratch;
	FillDirEntry(entry,".");
	entry->sfn.StartingCluster = htons(start_cluster);
	entry->sfn.Attrib = ATTRIB_SUBDIR;

	//����".."���
	entry = (CDirEntry*)&Scratch[32];
	FillDirEntry(entry,"..");
	entry->sfn.StartingCluster = htons(GetClusterOfParentDirectory());
	entry->sfn.Attrib = ATTRIB_SUBDIR;

	//д�����ݵ�SD��
	SectWrite(dir_sectors);

	//����
	return 0;
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�RemoveDir()
* ��	�ܣ�ɾ���ļ���
* ��ڲ�����DirName - ����
* ���ڲ�������
* ˵	������
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
					//���Ϊɾ��״̬
					gFileFindInfo.Direntry->sfn.FileName[0]=0xE5; 	//0xE5Ϊɾ�����
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
				gFileFindInfo.Direntry->sfn.FileName[0]=0xE5; 	//0xE5Ϊɾ�����
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
	gFileFindInfo.Direntry->sfn.FileName[0]=0xE5; 	//0xE5Ϊɾ�����
	SectWrite(gFileFindInfo.Block);
	FatChainFree(ntohs(gFileFindInfo.Direntry->sfn.StartingCluster));
	return 1;
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�FileCreate()
* ��	�ܣ������ļ�
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
static BYTE FileCreate(CFileFindInfo *pFindInfo, char* filename) reentrant
{
	xdata BYTE i,j;

	//Ѱ�ҵ�һ���յ��ļ������
	if(!FileFindFirst(pFindInfo,1)) return 0;

	//��д�ļ������
	for( i = 0; i < 10; i++)
	{
		pFindInfo->Direntry->sfn.Reserved[i] = 0;
	}

	pFindInfo->Direntry->sfn.UpdateTime.i = pFindInfo->Direntry->sfn.UpdateDate.i = 0;
	pFindInfo->Direntry->sfn.StartingCluster =
	pFindInfo->Direntry->sfn.FileSize = 0;

	//��д�ļ�����
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

	//��������
	pFindInfo->Direntry->sfn.Attrib = ATTRIB_ARCHIVE;

	//д��������
	SectWrite(pFindInfo->Block);

	return 1;
}

/*****************************************************************************************************************
* ��	�ƣ�FileOpen()
* ��	�ܣ����ļ�
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef _FILE_OPEN_
int FileOpen(FILE* f,char* filename,char* mode) reentrant
{
	//�ô򿪱�־Ϊ0
	f->IsOpen = 0;

	//�����д�ļ�����ɾ����ǰ���ļ�
	if( mode[0] == 'w' )
	{
		FileDelete(filename);
	}

	//Ѱ���ļ�
	FileFindFirst(&gFileFindInfo, 0);
	while(!FileNameMatch(filename, gFileFindInfo.Direntry->sfn.FileName))
	{
		if(!FileFindNext(&gFileFindInfo))
		{
			if(mode[0] == 'r')
			{
				return 0; 		//û�ҵ��ļ�
			}
			if( mode[0] == 'w' || mode[0] == 'a' )
			{
				if(!FileCreate(&gFileFindInfo, filename))
				{
					return 0; 	//�����ļ�ʧ��
				}
				else
				{
					break;
				}
			}
		}
	}

	//�����ļ�����
	f->SectorDirentry = gFileFindInfo.Block;
	f->OffsetDirentry = gFileFindInfo.Offset;
	f->ClusterStart = f->SectorCurrent = ntohs(gFileFindInfo.Direntry->sfn.StartingCluster);
	f->Attrib = gFileFindInfo.Direntry->sfn.Attrib;
	f->Size = ntohl(gFileFindInfo.Direntry->sfn.FileSize);

	//�����ļ���������
	if(mode[0]=='a') f->Pos = f->Size;
	else f->Pos = 0;

	//���ļ��򿪱�־
	return f->IsOpen = 1;
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�FileEOF()
* ��	�ܣ��ж��ļ������Ƿ��Ѿ�����ĩβ
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef _FILE_EOF_
int FileEOF(FILE* f)  reentrant
{
	if(!f->IsOpen) return 1;
	return (f->Pos >= f->Size);
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�FileRead()
* ��	�ܣ����ļ�����
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef _FILE_READ_
unsigned FileRead(FILE* f,BYTE* buffer,unsigned count)	 reentrant
{
	unsigned xdata cnt,total_cnt=0;
	if(!f->IsOpen || !count) return 0;

	//�ж��Ƿ��û�������Scratch�����������ļ���������������з���
	if((buffer >= Scratch) && (buffer < Scratch+SectBlockSize()))
	count = min(count, SectBlockSize()-f->Pos % SectBlockSize());

	//��SD��������
	while(count && !FileEOF(f))
	{
		f->SectorCurrent = FatChain(f->ClusterStart,f->Pos/SectBlockSize());

		SectRead(SectFileData() + f->SectorCurrent);

		cnt=min(SectBlockSize()-f->Pos%SectBlockSize(),count);
		cnt=min(cnt,f->Size-f->Pos);

		//����
		OS_ENTER_CRITICAL();
		memmove(buffer,Scratch+f->Pos%SectBlockSize(),cnt);
		OS_EXIT_CRITICAL();
		
		//����
		total_cnt+=cnt;
		f->Pos+=cnt;
		count-=cnt;
		buffer+=cnt;
	}

	return total_cnt;
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�FileWrite()
* ��	�ܣ�д���ݵ��ļ�
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef _FILE_WRITE_
unsigned FileWrite(FILE* f, BYTE* buffer, unsigned count)  reentrant
{
	xdata unsigned cnt,total_cnt=0,xtra,alloced;
	xdata CDirEntry* entry;
	if(!f->IsOpen || !count) return 0;

	//����ռ�
	if(f->Pos+count>f->Size)
	{
		xtra = (1+((f->Pos+count-1)/SectBlockSize())/MBR.sectors_per_cluster);
		if (f->Size > 0) xtra -= ( 1 +(( f->Size - 1 ) / SectBlockSize() ) / MBR.sectors_per_cluster);

		if(xtra > 0)
		{
			if(0xFFFF == (alloced=FatChainAlloc(f->SectorCurrent/MBR.sectors_per_cluster,xtra)))
			return 0;
		}

		//���Ĵ��ļ����ļ������
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

	//д���ݵ�SD��
	while(count && !FileEOF(f))
	{
		f->SectorCurrent = FatChain(f->ClusterStart,f->Pos/SectBlockSize());
		SectRead(SectFileData() + f->SectorCurrent);

		cnt = min(SectBlockSize()-f->Pos%SectBlockSize(),count);
		cnt = min(cnt,f->Size-f->Pos);

		//����
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
* ��	�ƣ�FileClose()
* ��	�ܣ��ر��ļ�
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
#ifdef _FILE_OPEN_
void FileClose(FILE* f) reentrant
{
	f->IsOpen = 0;
}
#endif

/*****************************************************************************************************************
* ��	�ƣ�FileClose()
* ��	�ܣ�ɾ���ļ�
* ��ڲ�������
* ���ڲ�������
* ˵	������
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

	//������ļ��У���ɾ��
	if(gFileFindInfo.Direntry->sfn.Attrib & (ATTRIB_SUBDIR|ATTRIB_LABEL)) return 0;

	//��Ǵ��ļ���ɾ��
	gFileFindInfo.Direntry->sfn.FileName[0] = 0xE5;		//0xE5Ϊɾ�����
	SectWrite(gFileFindInfo.Block);
	FatChainFree(ntohs(gFileFindInfo.Direntry->sfn.StartingCluster));

	return 1;
}
#endif

//end of file


				 

