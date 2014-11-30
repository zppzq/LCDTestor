/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: SdMmc.c
**创   建   人: 杨承凯(kady1984@163.com)
**创 建 日  期: 2008年04月19日
**最后修改日期: 2008年04月19日
**描        述: CF卡物理访问函数
*****************************************************************************************************************/
#include <stdio.h>
#include <intrins.h>
#include "c8051f340.h"
#include "SdSet.h"
#include "SdMmc.h"

//================================================================================================================

#define ERROR_CODE	0xFFFF
#define BUFFER_SIZE 16

//COMMAND结构体里边要用到的一些命令
#define     EMPTY  0
#define     YES   1
#define     NO    0
#define     CMD   0
#define     RD    1
#define     WR    2
#define     R1    0
#define     R1b   1
#define     R2    2
#define     R3    3

//SD操作的起始和结束命令字
#define     START_SBR      0xFE
#define     START_MBR      0xFE
#define     START_SBW      0xFE
#define     START_MBW      0xFC
#define     STOP_MBW       0xFD

//SD卡写操作回复信号的掩码
#define     DATA_RESP_MASK 0x11

//SD卡忙信号掩码
#define     BUSY_BIT       0x80

//命令定义，即COMMAND里边的操作码
#define     GO_IDLE_STATE            0
#define     SEND_OP_COND             1
#define     SEND_CSD                 2
#define     SEND_CID                 3
#define     STOP_TRANSMISSION        4
#define     SEND_STATUS              5
#define     SET_BLOCKLEN             6
#define     READ_SINGLE_BLOCK        7
#define     READ_MULTIPLE_BLOCK      8
#define     WRITE_BLOCK              9
#define     WRITE_MULTIPLE_BLOCK    10
#define     PROGRAM_CSD             11
#define     SET_WRITE_PROT          12
#define     CLR_WRITE_PROT          13
#define     SEND_WRITE_PROT         14
#define     TAG_SECTOR_START        15
#define     TAG_SECTOR_END          16
#define     UNTAG_SECTOR            17
#define     TAG_ERASE_GROUP_START   18
#define     TAG_ERASE_GROUP_END     19
#define     UNTAG_ERASE_GROUP       20
#define     ERASE                   21
#define     LOCK_UNLOCK             22
#define     READ_OCR                23
#define     CRC_ON_OFF              24

//内部要使用的联合体定义
typedef union
{
	long l;
	unsigned char b[4];
} LONG_UNION;

typedef union
{
	int i;
	unsigned char b[2];
} INT_UNION;

typedef union
{
	unsigned long l;
	unsigned char b[4];
} ULONG_UNION;

typedef union
{
	unsigned int i;
	unsigned char b[2];
} UINT_UNION;

//描述命令属性的结构体
typedef struct
{
	unsigned char command_byte; 		// 此命令的操作码
	unsigned char arg_required; 		// 此命令是否带参数
	unsigned char CRC; 					// 此命令的CRC字节
	unsigned char trans_type; 			// 此命令的操作类型(控制、读、或写)
	unsigned char response; 			// 此命令的回复方式
	unsigned char var_length; 			// Indicates varialble length transfer;
} COMMAND;

//SD卡里所有可用的SPI命令
COMMAND code command_list[25] =
{
	{0,	NO ,0x95,CMD,R1 ,NO}, // CMD0;  GO_IDLE_STATE: 			复位SD卡
	{1,	NO ,0xFF,CMD,R1 ,NO}, // CMD1;  SEND_OP_COND: 			初始化SD卡
	{9,	NO ,0xFF,RD ,R1 ,NO}, // CMD9;  SEND_CSD: 				获取SD卡的详细数据
	{10,NO ,0xFF,RD ,R1 ,NO}, // CMD10; SEND_CID: 				获取SD卡的ID
	{12,NO ,0xFF,CMD,R1 ,NO}, // CMD12; STOP_TRANSMISSION: 		强制停止多数据块读操作
	{13,NO ,0xFF,CMD,R2 ,NO}, // CMD13; SEND_STATUS: 			读取卡的状态寄存器
	{16,YES,0xFF,CMD,R1 ,NO}, // CMD16; SET_BLOCKLEN: 			设置读写的字节长度
	{17,YES,0xFF,RD ,R1 ,NO}, // CMD17; READ_SINGLE_BLOCK: 		写入一个数据块，长度由SET_BLOCKLEN定
	{18,YES,0xFF,RD ,R1 ,YES}, // CMD18; READ_MULTIPLE_BLOCK: 	读取多个数据块，直到被STOP_TRANSMISSION停止
	{24,YES,0xFF,WR ,R1 ,NO}, // CMD24; WRITE_BLOCK: 			写入一个数据块，长度由SET_BLOCKLEN定
	{25,YES,0xFF,WR ,R1 ,YES}, // CMD25; WRITE_MULTIPLE_BLOCK: 	写入多个数据块，直到停止命令被发送
	{27,NO ,0xFF,CMD,R1 ,NO}, // CMD27; PROGRAM_CSD: 			对CSD中可编程改写的位进行改写
	{28,YES,0xFF,CMD,R1b,NO}, // CMD28; SET_WRITE_PROT: 		写保护SD卡里边的地址组，要指定地址
	{29,YES,0xFF,CMD,R1b,NO}, // CMD29; CLR_WRITE_PROT: 		清除SD卡里边的写保护，要指定地址
	{30,YES,0xFF,CMD,R1 ,NO}, // CMD30; SEND_WRITE_PROT: 		查询写保护的地址
	{32,YES,0xFF,CMD,R1 ,NO}, // CMD32; TAG_SECTOR_START: 		设置需要擦除的首个可写块的地址
	{33,YES,0xFF,CMD,R1 ,NO}, // CMD33; TAG_SECTOR_END: 		设置需要连续擦除的最后那个块的地址
	{34,YES,0xFF,CMD,R1 ,NO}, // CMD34; UNTAG_SECTOR: 			检查是否处于擦除区
	{35,YES,0xFF,CMD,R1 ,NO}, // CMD35; TAG_ERASE_GROUP_START;
	{36,YES,0xFF,CMD,R1 ,NO}, // CMD36; TAG_ERASE_GROUP_END;
	{37,YES,0xFF,CMD,R1 ,NO}, // CMD37; UNTAG_ERASE_GROUP;
	{38,YES,0xFF,CMD,R1b,NO}, // CMD38; ERASE: 					擦除目标扇区，有TAG_SECTOR_START和TAG_SECTOR_END设定的
	{42,YES,0xFF,CMD,R1b,NO}, // CMD42; LOCK_UNLOCK;			锁定或解锁
	{58,NO ,0xFF,CMD,R3 ,NO}, // CMD58; READ_OCR: 				读取OCR寄存器的内容
	{59,YES,0xFF,CMD,R1 ,NO} // CMD59; CRC_ON_OFF: 				切换CRC可选项的开和关状态
};

//内部的全局变量
unsigned long xdata  PHYSICAL_SIZE; 							//SD卡的大小，字节数，在初始化的时候确定
unsigned long xdata  PHYSICAL_BLOCKS; 							//SD卡的块数目
unsigned char xdata Is_Initialized; 							//指示是否已经初始化
char xdata  LOCAL_BLOCK[BUFFER_SIZE];							//数据缓冲区

//在汇编里定义的两个读写函数
extern SD_READ_BYTES_ASM(unsigned char* pchar, unsigned int len);
extern SD_WRITE_BYTES_ASM(unsigned char* pchar, unsigned int len);

//宏操作定义，给结束码，然后返回
#define BACK_FROM_ERROR 		\
{								\
	SPI0DAT = 0xFF;	 			\
	while(!SPIF){} 				\
	SPIF = 0; 					\
	NSSMD0 = 1;					\
	SPI0DAT = 0xFF;				\
	while(!SPIF){}  			\
	SPIF = 0;  					\                      
	return ERROR_CODE;			\
}


/*****************************************************************************************************************
* 名	称：SdInterfaceInit()
* 功	能：SD卡接口初始化函数(SPI初始化)
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SdInterfaceInit (void) reentrant
{
	SPI0CFG = 0x70;	
	SPI0CN = 0x0F;
	SPI0CKR = 1;
	NSSMD0 = 1;

	P0MDOUT = 0x0D; 				//set SCK,MOSI as a push-pull
	XBR0 = 0x02; 					//SPI enabled
}

/*****************************************************************************************************************
* 名	称：SdWaitNs()
* 功	能：SD延时函数(单位ns，系统时钟48MHz)
* 入口参数：count - 要延时的ns数
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SdWaitNs(unsigned int count) reentrant
{
	count /= 20;

	while(count--);
}

/*****************************************************************************************************************
* 名	称：SdWaitMs()
* 功	能：SD延时函数(单位ms，系统时钟48MHz)
* 入口参数：count - 要延时的ns数
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SdWaitMs(unsigned int count) reentrant
{
	int i,j;
	for(i=0;i<count;i++)
	{
		for(j=0;j<1000;j++)
		{
			SdWaitNs(1000);
		}
	}
}

/*****************************************************************************************************************
* 名	称：WriteReadSpiByte()
* 功	能：读写SPI总线上一个字节
* 入口参数：byte - 写入值
* 出口参数：输出值
* 说	明：无
*****************************************************************************************************************/
unsigned char WriteReadSpiByte(unsigned char byte) reentrant
{
	unsigned char ret;
	SPI0DAT = byte;
	while(!SPIF);
	SPIF = 0;
	ret = SPI0DAT;
	return ret;
}

/*****************************************************************************************************************
* 名	称：SdCommandExec()
* 功	能：SD卡命令执行函数
* 入口参数：
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned int SdCommandExec(unsigned char cmd_loc, unsigned long argument, unsigned char *pchar) reentrant
{
	unsigned char loopguard; 								//等待的重试次数
	COMMAND current_command; 								//当前要使用的命令
	ULONG_UNION long_arg; 									//参数区
	static unsigned int current_blklen = 512; 				//记录读写长度
	unsigned long old_blklen = 512; 						//用与保存读写长度
	unsigned int counter = 0; 								//循环操作要用的索引
	UINT_UNION card_response; 								//用于保存回复的数据
	unsigned char data_resp; 								//数据读写的应答信号
	unsigned char dummy_CRC; 								//用于保存CRC字节，CRC无意义，只是占个位置

	//查询命令                                   
	current_command = command_list[cmd_loc];
	card_response.i = 0;


	//参数的初始化处理========================================================================================
	//命令为SET_BLOCKLEN(16)表明是设置后面所有数据块读写长度，参数区指长度
	if(current_command.command_byte == 16)
	{
		current_blklen = argument;
	}

 	//命令为SEND_CSD(9)或SEND_CSD(10)表明要读一个16字节的寄存器，
	//此时段长度必须强制设置为16，执行玩之后还得恢复以前的段长度
	if((current_command.command_byte == 9) || (current_command.command_byte == 10))
	{
		old_blklen = current_blklen;
		current_blklen = 16;
	}


	//选择SD卡===============================================================================================
	NSSMD0 = 0; 					//选中SD卡


	//发送命令===============================================================================================
	//写命令字段
	WriteReadSpiByte(current_command.command_byte | 0x40);

	//写参数字段
	if(current_command.arg_required == YES) long_arg.l = argument;
	else long_arg.l = 0;
	for(counter = 0; counter <= 3; counter++)
	{
		WriteReadSpiByte(long_arg.b[counter]);
	}

	//写校验字段，哑字节，只是填充位置(只有CMD0是真的)
	WriteReadSpiByte(current_command.CRC);


	//应答===================================================================================================
	//如果应答方式为R1方式
	if(current_command.response == R1)
	{
		//接收R1应答
		loopguard=0;
		do
		{
			card_response.b[0] = WriteReadSpiByte(0xFF);
			if(!++loopguard) break;
			if(card_response.b[0] & BUSY_BIT)
			{
				SdWaitNs(700);
			}
		}while((card_response.b[0] & BUSY_BIT));//等待处理结束

		//如果超时，返回
		if(!loopguard)
		{
			return card_response.b[0];
		}
	}

	//如果应答方式为R1b方式
	else if(current_command.response == R1b)
	{
		loopguard = 0;
		do
		{
			card_response.b[0] = WriteReadSpiByte(0xFF);
			if(!++loopguard) break;
		}
		while((card_response.b[0] & BUSY_BIT));//等待处理结束

		while(WriteReadSpiByte(0xff) == 0x00);//附加的忙信号，等待处理结束(非零表示结束)

	}

	//如果应答方式为R2方式
	else if(current_command.response == R2)
	{
		loopguard=0;
		do
		{
			card_response.b[0] = WriteReadSpiByte(0xFF);
			if(!++loopguard) break;
		}while((card_response.b[0] & BUSY_BIT));//等待处理结束
		if(!loopguard) BACK_FROM_ERROR;

		card_response.b[1] = WriteReadSpiByte(0xFF);//读第二个回复字节
	}

	//如果应答方式为R3方式
	else
	{
		//R3回复
		loopguard=0;
		do
		{
			card_response.b[0] = WriteReadSpiByte(0xFF);
			if(!++loopguard) break;
		}while((card_response.b[0] & BUSY_BIT));
		if(!loopguard)	BACK_FROM_ERROR;
		
		//读取剩下的4个字节(OCR的内容)
		for(counter = 0; counter <= 3; counter++) 
		{
			*pchar++ = WriteReadSpiByte(0xFF);
		}
	}


	//判断传输模式==========================================================================================
	switch(current_command.trans_type)
	{
		//读命令
		case RD:
			loopguard = 0;
			while((WriteReadSpiByte(0xFF)) != START_SBR)
			{
				SdWaitNs(700);
				if(!++loopguard) BACK_FROM_ERROR;
			}
			counter = 0;
			EA = 0;
			SD_READ_BYTES_ASM(pchar, current_blklen);
			EA = 1;
			dummy_CRC = WriteReadSpiByte(0xFF);
			break;

		//写命令
		case WR:
			WriteReadSpiByte(0xFF);
			WriteReadSpiByte(START_SBW);
			EA = 0;
			SD_WRITE_BYTES_ASM(pchar, current_blklen);
			EA = 1;
	
			//读应答信号
			loopguard = 0;
			do
			{
				data_resp = WriteReadSpiByte(0xFF);
				if(!++loopguard) break;
			}while((data_resp & DATA_RESP_MASK) != 0x01);
			if(!loopguard) BACK_FROM_ERROR;
	
			while(WriteReadSpiByte(0xFF) == 0x00);
			WriteReadSpiByte(0xFF);
			break;

		default: break;
	}


	//结束命令===================================================================================================
	WriteReadSpiByte(0xFF); 		//同步时钟，必要的(kady注)
	NSSMD0 = 1; 					//取消SD卡选择
	WriteReadSpiByte(0xFF); 		//同步时钟，必要的(kady注)

	//恢复读写长度
	if((current_command.command_byte == SEND_CSD)||(current_command.command_byte == SEND_CID))
	{
		current_blklen = old_blklen;
	}


	//返回应答信号===============================================================================================
	return card_response.i;
}

/*****************************************************************************************************************
* 名	称：SdInit()
* 功	能：SD卡初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SdInit(void) reentrant
{
	xdata unsigned loopguard;
	xdata UINT_UNION card_status;
	xdata unsigned char counter = 0;
	unsigned char *pchar; 
	unsigned int c_size,bl_len;
	unsigned char c_mult;

	//SPI初始化以后要延时=======================================================================================
	Is_Initialized = 0;
	SdWaitMs(100);
	pchar = (unsigned char xdata*)LOCAL_BLOCK;

	
	//SD卡的启动时钟============================================================================================
	for(counter = 0; counter < 10; counter++)
	{
		WriteReadSpiByte(0xFF);
	}
	SdWaitMs(1);


	//复位SD卡==================================================================================================
	card_status.i = SdCommandExec(GO_IDLE_STATE, EMPTY, EMPTY);


	//启动SD卡，等待初始化完成==================================================================================
	loopguard=0;
	do
	{
		SdWaitMs(1);
		card_status.i = SdCommandExec(SEND_OP_COND, EMPTY, EMPTY);
		if(!++loopguard) return;
	}while ((card_status.b[0] & 0x01));


	//读OCR寄存器================================================================================================
	loopguard=0;
	do
	{
		card_status.i = SdCommandExec(READ_OCR,EMPTY,pchar);
		if(!++loopguard) return;
	}while(!(*pchar&0x80));


	//读状态寄存器===============================================================================================
   	card_status.i = SdCommandExec(SEND_STATUS,EMPTY,EMPTY);


	//读CSD寄存器================================================================================================
	card_status.i = SdCommandExec(SEND_CSD,EMPTY,pchar);

	//解析CSD寄存器里边的内容
	bl_len = 1 << (pchar[5] & 0x0f);
	c_size = ((pchar[6] & 0x03) << 10) | (pchar[7] << 2) | ((pchar[8] &0xc0) >> 6);
	c_mult = (((pchar[9] & 0x03) << 1) | ((pchar[10] & 0x80) >> 7));

	//计算SD卡的大小
	PHYSICAL_BLOCKS = (unsigned long)(c_size+1)*(1 << (c_mult+2));
	PHYSICAL_SIZE = PHYSICAL_BLOCKS * bl_len;

	
	//设置读写数据的块长度=======================================================================================
	card_status.i = SdCommandExec(SET_BLOCKLEN, (unsigned long)PHYSICAL_BLOCK_SIZE, EMPTY);


	//完成=======================================================================================================
	Is_Initialized = 1;

}

/*****************************************************************************************************************
* 名	称：SdBlockRead()
* 功	能：SD卡块读取函数
* 入口参数：address - 地址，wdata - 数据区
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned int SdBlockRead(unsigned long address, unsigned char *pchar) reentrant
{
	unsigned int card_status; 			// Stores MMC status after each MMC command;
	
	address *= PHYSICAL_BLOCK_SIZE;

	card_status = SdCommandExec(READ_SINGLE_BLOCK, address, pchar);

	return card_status;
}

/*****************************************************************************************************************
* 名	称：SdBlockWrite()
* 功	能：SD卡块读取函数
* 入口参数：address - 地址，wdata - 数据区
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned char SdBlockWrite(unsigned long address,unsigned char *wdata) reentrant
{
	unsigned int card_status;

	address *= PHYSICAL_BLOCK_SIZE;
	card_status = SdCommandExec(WRITE_BLOCK, address, wdata);

	return card_status;
}

/*****************************************************************************************************************
* 名	称：SdSectors()
* 功	能：获取扇区的数目
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
unsigned long SdSectors(void) reentrant
{
	return PHYSICAL_BLOCKS;
}

