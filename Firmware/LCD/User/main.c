/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: mian.c
**创   建   人: 
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

//#include "DataManage.h"
//#include "DataAcq.h"
//#include "CommApp.h"
//#include "DS18B20\DS18B20.h"
//#include "Bsp\LowPower.h"
//#include "PowerManager.h"


//宏参数定义=========================================================================




//入口程序============================================================================
//初始化硬件，初始化系统，建立启动任务
int main(void)
{
	//不要再修改此函数，用户初始化内容请放在TaskStart函数里
//	while (1);
	BSP_Init();  											//硬件初始化
	LCDInit();
	LightsInit();

	while (1)
	{
		//LCDDisplayRed();
/*		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020;
		GPIOF->BRR  = 0x00000020;
    	GPIOF->BSRR = 0x00000020; */
		test();
	}
}

