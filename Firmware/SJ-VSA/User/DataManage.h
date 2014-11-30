/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: DataManage.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年7月21日
**最后修改日期: 2008年7月21日
**描        述: 采集器数据管理头文件
********************************************************************************************************/
#ifndef		_DATA_MANAGE_H_
#define		_DATA_MANAGE_H_

#ifdef		_DATA_MANAGE_C_
#define		DATA_MANAGE_EXT
#else
#define		DATA_MANAGE_EXT	extern
#endif

//终端参数结构体
//终端参数结构
//将需要存储的参数定义在此结构体内
typedef struct _STerminalParam
{
	uint32 nDataCfgKey;
	float arrADStep[DATA_ACQ_COUNT];
	float arrZeroVol[DATA_ACQ_COUNT];  
}STerminalParam;

//全局终端参数
extern STerminalParam g_TerminalParam;

//编译控制=========================================================================
#define _DATA_MNG_PARAM_CONFIGED_
#define _DATA_MNG_DATA_LOAD_
#define _DATA_MNG_DATA_STORE_
#define _DATA_MNG_MEMORIZER_ERASE_
#define _DATA_MNG_MEMORIZER_ERASE_TO_SECT_
#define _DATA_MNG_ZERO_CONFIGED_

//函数声明
void DataManageInit(void);

void ParamLoad(void);
void ParamStore(void);
BOOL IsParamEmpty(void);
void ParamSetDefault(void);

void PostParamStoreEvent(void);
void PostDataEraseEvent(uint32 nBlockToBeErase);
void ResetDataEraseBlock(void);


void MemorizerErase(void) reentrant;


BOOL MemDataLoad(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint32 nlen,uint8 nChannelID);
BOOL MemDataStore(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint32 nlen,uint8 nChannelID);


//任务处理
void DataManagerProcess(void);


//文件结束
#endif
