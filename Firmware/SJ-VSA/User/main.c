/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: mian.c
**创   建   人: 杨承凯
**创 建 日  期: 2008年2月25日
**最后修改日期: 2008年3月11日
**描        述: 主程序源文件
**注        意: 请在OsCpu\OS_CFG.H中设置合适的OS_MAX_TASKS个数，OS_MAX_EVENTS个数等。
**版 本 说  明：编译前确认：1.电路版本号；2.ID号FLASH编译
*****************************************************************************************************************/

/**********系统性能********************
//Keil ARM 编译器
//处理器时钟：72MHz
//线程切换时间10us
//uint32 算数运算与逻辑运算：30个指令周期
//float 加减乘运算：90个指令周期；除法：150个指令周期
//memcpy 2K数据，耗时22us
//for循环数组复制2K数据，耗时750us!!!
//nand flash(K9F1G08U0A) 写入2k数据，耗时550us
//nand flash(K9F1G08U0A) 读取2k数据，耗时800us
*/

#include "includes.h"
#include "stdarg.h"
//#include "DataManage.h"
//#include "DataAcq.h"
//#include "CommApp.h"
//#include "DS18B20\DS18B20.h"
//#include "Bsp\LowPower.h"
//#include "PowerManager.h"


//宏参数定义=========================================================================
#define  TASK_STK_SIZE                  128      /* Size of each task's stacks (# of WORDs)            */



//任务定义===========================================================================
//任务堆栈
OS_STK TaskStartStk[TASK_STK_SIZE];						//启动任务的堆栈
OS_STK TaskCommStk[TASK_STK_SIZE];						//通信任务的堆栈
OS_STK TaskDataAcqStk[TASK_STK_SIZE];					//传感器任务的堆栈
OS_STK TaskMonitorStk[TASK_STK_SIZE];					//电源电压监测任务堆栈
OS_STK TaskDataStoreStk[TASK_STK_SIZE];					//数据存储任务堆栈
OS_STK TaskADStk[TASK_STK_SIZE];						//AD数据读取任务堆栈
OS_STK TaskSampleStk[TASK_STK_SIZE];					//采样任务堆栈
OS_STK TaskDiskStk[TASK_STK_SIZE];						//磁盘管理任务堆栈
OS_STK TaskPowerKeyStk[TASK_STK_SIZE];					//开关任务堆栈
OS_STK TaskTemperatureStk[TASK_STK_SIZE];				//温度任务堆栈
OS_STK TaskDataManageStk[TASK_STK_SIZE];				//参数管理任务堆栈
OS_STK TaskLCDStk[TASK_STK_SIZE];						//LCD任务堆栈

//任务函数
void TaskStart(void *nouse) reentrant;					//启动任务					
void TaskComm(void *nouse) reentrant;					//通信任务
void TaskMonitor(void *nouse) reentrant;				//监测任务
void TaskDataAcq(void *nouse) reentrant;				//传感器任务
void TaskDataStore(void *nouse) reentrant;				//存储任务
void TaskAD(void *nouse) reentrant;						//AD数据读取任务
void TaskSample(void *nouse) reentrant;					//采样任务
void TaskDisk(void *nouse) reentrant;					//磁盘管理任务
void TaskPowerKey(void *nouse) reentrant;				//按键访问任务
void TaskTemperature(void *nouse) reentrant;			//温度检测任务
void TaskDataManage(void *nouse) reentrant;				//参数管理任务
void TaskLCD(void *nouse) reentrant;					//LCD任务
//-----------------------------------------------------------------------------------

//入口程序============================================================================
//初始化硬件，初始化系统，建立启动任务
int main(void)
{
	//不要再修改此函数，用户初始化内容请放在TaskStart函数里
	BSP_Init();  											//硬件初始化
	OSInit();												//系统初始化
	OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1],0);		//创建启动任务
	OSStart();												//启动系统
	return 0;
}

//启动任务============================================================================
//初始化其他任务，启动其他任务
void TaskStart(void *nouse) reentrant
{
	//避免编译警告
	nouse = nouse; 			 	
	//BSP参数初始化
	BspVariInit();

	//延迟100ms确认
	//OSTimeDly(OS_TICKS_PER_SEC/10);	 

	nouse = nouse; 	
	

	//亮灯，表示可以放开按键
	PowerLightOn();	
	SigLightOn(); 
	
	//等待放开
//	while(GetSignal(POWER_KEY) == 0);

	//放开后指示灯闪烁两次
	PowerLightOff();  
	SigLightOff();
	OSTimeDly(OS_TICKS_PER_SEC/2);
	PowerLightOn();
	SigLightOn();
	OSTimeDly(OS_TICKS_PER_SEC/20);
	PowerLightOff();
	SigLightOff();
	OSTimeDly(OS_TICKS_PER_SEC/5);
	PowerLightOn();
	SigLightOn();
	OSTimeDly(OS_TICKS_PER_SEC/20);
	PowerLightOff();
	SigLightOff();		
//	PowerEventClear();

/*	//电源按键检测	
	OSTaskCreate(TaskPowerKey, (void *)0, &TaskPowerKeyStk[TASK_STK_SIZE - 1], 1);	

	//创建AD转换任务
	OSTaskCreate(TaskAD, (void *)0, &TaskADStk[TASK_STK_SIZE - 1], 2);
	
	//采样任务
	OSTaskCreate(TaskSample, (void *)0, &TaskSampleStk[TASK_STK_SIZE - 1], 3);

	//磁盘管理
	OSTaskCreate(TaskDisk, (void *)0, &TaskDiskStk[TASK_STK_SIZE - 1], 4);

	//存储任务
	OSTaskCreate(TaskDataStore, (void *)0, &TaskDataStoreStk[TASK_STK_SIZE - 1], 5);

	//创建通信任务
	OSTaskCreate(TaskComm, (void *)0, &TaskCommStk[TASK_STK_SIZE - 1], 6);			

	//参数管理任务
	OSTaskCreate(TaskDataManage, (void *)0, &TaskDataManageStk[TASK_STK_SIZE - 1], 7);

	//创建传感器任务
	OSTaskCreate(TaskDataAcq, (void *)0, &TaskDataAcqStk[TASK_STK_SIZE - 1], 8);	
	
	//创建电源管理任务
	OSTaskCreate(TaskMonitor, (void *)0, &TaskMonitorStk[TASK_STK_SIZE - 1], 9);
  */
  	//创建电源管理任务
	OSTaskCreate(TaskLCD, (void *)0, &TaskLCDStk[TASK_STK_SIZE - 1], 10);

	//指示灯服务
	for(;;)
	{
		LightsProcess();
	}
}

//LCD任务
void TaskLCD(void *nouse) reentrant
{
  	nouse = nouse;

	LCDInit();

	for (;;)
	{
		LCDProcess();
		OSTimeDly(5 * OS_TICKS_PER_SEC);
	}
}


/*
//AD转换任务=============================================================
void TaskAD(void *nouse) reentrant
{
	nouse = nouse;

	for(;;)
	{		
		ADProcess();
	}
}

//数据抽样任务=============================================================
void TaskSample(void *nouse) reentrant
{
	nouse = nouse;

	for(;;)
	{
		SampleProcess();
	}
}

//磁盘管理线程
void TaskDisk(void *nouse) reentrant
{
	nouse = nouse;

	//开磁盘
	DiskOpen();	 

	for(;;)
	{
		DiskProcess();	
	}
}

//数据存储任务，该任务必须在磁盘管理任务之后启动===========================
void TaskDataStore(void *nouse) reentrant
{
	nouse = nouse;

	//初始化
	DataStoreInit();

 	for(;;)
	{
		DataStoreProcess();
	}
}

//通信任务=================================================================
void TaskComm(void *nouse) reentrant
{
	nouse = nouse;

 	//通信初始化
	UserCommInit();

	//波特率设置为57600
	SetCommBaudRate(2, 57600);

	//设置常规超时
	SetNormalRecvTimeOut();
	
	//打开通信接口
	OpenComm2();

	//循环任务处理
	for(;;)
	{ 
		//是否低电压，如果低电压，则不再进行通信
		if(IsLowPower() == TRUE)
		{
			OSTimeDly(OS_TICKS_PER_SEC*5);
			continue;
		}

		//通信事务处理
		CommProcess();
	} 
}

//参数管理任务，该任务必须在磁盘管理任务之后启动============================
void TaskDataManage(void *nouse) reentrant
{
	nouse = nouse;

	//初始化参数管理
	DataManageInit();

	//从磁盘读取参数
	ParamLoad();

	//数据管理
 	for(;;)
	{
		DataManagerProcess();
	}
}

//传感器任务，该任务必须在数据管理任务之后启动==============================
void TaskDataAcq(void *nouse) reentrant
{
	nouse = nouse;

	//数据采集初始化
	DataAcqInit();

	//数据采集参数初始化
	DataAcqParamInit();

	//是否要从参数管理器加载参数
	if(IsParamEmpty() == TRUE)
	{
		//设置到参数管理器
		DataAcqParamStore(); 
	}
	else
	{
		//从参数管理器加载
		DataAcqParamLoad();
	}

	//任务循环
	for(;;)
	{
		//传感器处理	
		DataAcqProcess();
	}
}


//电源监测任务==============================================================
void TaskMonitor(void *nouse) reentrant
{
	nouse = nouse;

	//延时1秒后执行
	OSTimeDly(OS_TICKS_PER_SEC);

	//初始化电源管理参数
	PowerManagerInit();

	//开机后第一次电压测试
	PowerTest();

	//电源管理事务
 	for(;;)
	{
		PowerManagerProcess();
	}
}


//DS18B20温度检测
uint8 TempID[5][8];
uint32 g_nTemperature[4];
void TaskTemperature(void *nouse) reentrant
{
	uint8 i;
	nouse = nouse;
	DS18B20_Configuration();
	if(TRUE == OWFirst())
	{
		memcpy(TempID[0], ROM_NO, 8);
		for(i = 1; i<5; i++)
		{
			if(TRUE == OWNext())
			{
				memcpy(TempID[i], ROM_NO, 8);
			}
			else
			{
				break;
			}
			
		}
	}
	for(i = 0;; i++)
	{
		i &= 0x03;
		g_nTemperature[i] = GetTemperature(TempID[i]);
		OSTimeDly(10);
	}
}

*/
