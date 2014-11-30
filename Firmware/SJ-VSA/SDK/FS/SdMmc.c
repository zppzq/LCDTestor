/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: SdMmc.c
**��   ��   ��: ��п�(kady1984@163.com)
**�� �� ��  ��: 2008��04��19��
**����޸�����: 2008��04��19��
**��        ��: CF��������ʺ���
*****************************************************************************************************************/
#include <stdio.h>
#include <intrins.h>
#include "c8051f340.h"
#include "SdSet.h"
#include "SdMmc.h"

//================================================================================================================

#define ERROR_CODE	0xFFFF
#define BUFFER_SIZE 16

//COMMAND�ṹ�����Ҫ�õ���һЩ����
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

//SD��������ʼ�ͽ���������
#define     START_SBR      0xFE
#define     START_MBR      0xFE
#define     START_SBW      0xFE
#define     START_MBW      0xFC
#define     STOP_MBW       0xFD

//SD��д�����ظ��źŵ�����
#define     DATA_RESP_MASK 0x11

//SD��æ�ź�����
#define     BUSY_BIT       0x80

//����壬��COMMAND��ߵĲ�����
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

//�ڲ�Ҫʹ�õ������嶨��
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

//�����������ԵĽṹ��
typedef struct
{
	unsigned char command_byte; 		// ������Ĳ�����
	unsigned char arg_required; 		// �������Ƿ������
	unsigned char CRC; 					// �������CRC�ֽ�
	unsigned char trans_type; 			// ������Ĳ�������(���ơ�������д)
	unsigned char response; 			// ������Ļظ���ʽ
	unsigned char var_length; 			// Indicates varialble length transfer;
} COMMAND;

//SD�������п��õ�SPI����
COMMAND code command_list[25] =
{
	{0,	NO ,0x95,CMD,R1 ,NO}, // CMD0;  GO_IDLE_STATE: 			��λSD��
	{1,	NO ,0xFF,CMD,R1 ,NO}, // CMD1;  SEND_OP_COND: 			��ʼ��SD��
	{9,	NO ,0xFF,RD ,R1 ,NO}, // CMD9;  SEND_CSD: 				��ȡSD������ϸ����
	{10,NO ,0xFF,RD ,R1 ,NO}, // CMD10; SEND_CID: 				��ȡSD����ID
	{12,NO ,0xFF,CMD,R1 ,NO}, // CMD12; STOP_TRANSMISSION: 		ǿ��ֹͣ�����ݿ������
	{13,NO ,0xFF,CMD,R2 ,NO}, // CMD13; SEND_STATUS: 			��ȡ����״̬�Ĵ���
	{16,YES,0xFF,CMD,R1 ,NO}, // CMD16; SET_BLOCKLEN: 			���ö�д���ֽڳ���
	{17,YES,0xFF,RD ,R1 ,NO}, // CMD17; READ_SINGLE_BLOCK: 		д��һ�����ݿ飬������SET_BLOCKLEN��
	{18,YES,0xFF,RD ,R1 ,YES}, // CMD18; READ_MULTIPLE_BLOCK: 	��ȡ������ݿ飬ֱ����STOP_TRANSMISSIONֹͣ
	{24,YES,0xFF,WR ,R1 ,NO}, // CMD24; WRITE_BLOCK: 			д��һ�����ݿ飬������SET_BLOCKLEN��
	{25,YES,0xFF,WR ,R1 ,YES}, // CMD25; WRITE_MULTIPLE_BLOCK: 	д�������ݿ飬ֱ��ֹͣ�������
	{27,NO ,0xFF,CMD,R1 ,NO}, // CMD27; PROGRAM_CSD: 			��CSD�пɱ�̸�д��λ���и�д
	{28,YES,0xFF,CMD,R1b,NO}, // CMD28; SET_WRITE_PROT: 		д����SD����ߵĵ�ַ�飬Ҫָ����ַ
	{29,YES,0xFF,CMD,R1b,NO}, // CMD29; CLR_WRITE_PROT: 		���SD����ߵ�д������Ҫָ����ַ
	{30,YES,0xFF,CMD,R1 ,NO}, // CMD30; SEND_WRITE_PROT: 		��ѯд�����ĵ�ַ
	{32,YES,0xFF,CMD,R1 ,NO}, // CMD32; TAG_SECTOR_START: 		������Ҫ�������׸���д��ĵ�ַ
	{33,YES,0xFF,CMD,R1 ,NO}, // CMD33; TAG_SECTOR_END: 		������Ҫ��������������Ǹ���ĵ�ַ
	{34,YES,0xFF,CMD,R1 ,NO}, // CMD34; UNTAG_SECTOR: 			����Ƿ��ڲ�����
	{35,YES,0xFF,CMD,R1 ,NO}, // CMD35; TAG_ERASE_GROUP_START;
	{36,YES,0xFF,CMD,R1 ,NO}, // CMD36; TAG_ERASE_GROUP_END;
	{37,YES,0xFF,CMD,R1 ,NO}, // CMD37; UNTAG_ERASE_GROUP;
	{38,YES,0xFF,CMD,R1b,NO}, // CMD38; ERASE: 					����Ŀ����������TAG_SECTOR_START��TAG_SECTOR_END�趨��
	{42,YES,0xFF,CMD,R1b,NO}, // CMD42; LOCK_UNLOCK;			���������
	{58,NO ,0xFF,CMD,R3 ,NO}, // CMD58; READ_OCR: 				��ȡOCR�Ĵ���������
	{59,YES,0xFF,CMD,R1 ,NO} // CMD59; CRC_ON_OFF: 				�л�CRC��ѡ��Ŀ��͹�״̬
};

//�ڲ���ȫ�ֱ���
unsigned long xdata  PHYSICAL_SIZE; 							//SD���Ĵ�С���ֽ������ڳ�ʼ����ʱ��ȷ��
unsigned long xdata  PHYSICAL_BLOCKS; 							//SD���Ŀ���Ŀ
unsigned char xdata Is_Initialized; 							//ָʾ�Ƿ��Ѿ���ʼ��
char xdata  LOCAL_BLOCK[BUFFER_SIZE];							//���ݻ�����

//�ڻ���ﶨ���������д����
extern SD_READ_BYTES_ASM(unsigned char* pchar, unsigned int len);
extern SD_WRITE_BYTES_ASM(unsigned char* pchar, unsigned int len);

//��������壬�������룬Ȼ�󷵻�
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
* ��	�ƣ�SdInterfaceInit()
* ��	�ܣ�SD���ӿڳ�ʼ������(SPI��ʼ��)
* ��ڲ�������
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�SdWaitNs()
* ��	�ܣ�SD��ʱ����(��λns��ϵͳʱ��48MHz)
* ��ڲ�����count - Ҫ��ʱ��ns��
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SdWaitNs(unsigned int count) reentrant
{
	count /= 20;

	while(count--);
}

/*****************************************************************************************************************
* ��	�ƣ�SdWaitMs()
* ��	�ܣ�SD��ʱ����(��λms��ϵͳʱ��48MHz)
* ��ڲ�����count - Ҫ��ʱ��ns��
* ���ڲ�������
* ˵	������
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
* ��	�ƣ�WriteReadSpiByte()
* ��	�ܣ���дSPI������һ���ֽ�
* ��ڲ�����byte - д��ֵ
* ���ڲ��������ֵ
* ˵	������
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
* ��	�ƣ�SdCommandExec()
* ��	�ܣ�SD������ִ�к���
* ��ڲ�����
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned int SdCommandExec(unsigned char cmd_loc, unsigned long argument, unsigned char *pchar) reentrant
{
	unsigned char loopguard; 								//�ȴ������Դ���
	COMMAND current_command; 								//��ǰҪʹ�õ�����
	ULONG_UNION long_arg; 									//������
	static unsigned int current_blklen = 512; 				//��¼��д����
	unsigned long old_blklen = 512; 						//���뱣���д����
	unsigned int counter = 0; 								//ѭ������Ҫ�õ�����
	UINT_UNION card_response; 								//���ڱ���ظ�������
	unsigned char data_resp; 								//���ݶ�д��Ӧ���ź�
	unsigned char dummy_CRC; 								//���ڱ���CRC�ֽڣ�CRC�����壬ֻ��ռ��λ��

	//��ѯ����                                   
	current_command = command_list[cmd_loc];
	card_response.i = 0;


	//�����ĳ�ʼ������========================================================================================
	//����ΪSET_BLOCKLEN(16)���������ú����������ݿ��д���ȣ�������ָ����
	if(current_command.command_byte == 16)
	{
		current_blklen = argument;
	}

 	//����ΪSEND_CSD(9)��SEND_CSD(10)����Ҫ��һ��16�ֽڵļĴ�����
	//��ʱ�γ��ȱ���ǿ������Ϊ16��ִ����֮�󻹵ûָ���ǰ�Ķγ���
	if((current_command.command_byte == 9) || (current_command.command_byte == 10))
	{
		old_blklen = current_blklen;
		current_blklen = 16;
	}


	//ѡ��SD��===============================================================================================
	NSSMD0 = 0; 					//ѡ��SD��


	//��������===============================================================================================
	//д�����ֶ�
	WriteReadSpiByte(current_command.command_byte | 0x40);

	//д�����ֶ�
	if(current_command.arg_required == YES) long_arg.l = argument;
	else long_arg.l = 0;
	for(counter = 0; counter <= 3; counter++)
	{
		WriteReadSpiByte(long_arg.b[counter]);
	}

	//дУ���ֶΣ����ֽڣ�ֻ�����λ��(ֻ��CMD0�����)
	WriteReadSpiByte(current_command.CRC);


	//Ӧ��===================================================================================================
	//���Ӧ��ʽΪR1��ʽ
	if(current_command.response == R1)
	{
		//����R1Ӧ��
		loopguard=0;
		do
		{
			card_response.b[0] = WriteReadSpiByte(0xFF);
			if(!++loopguard) break;
			if(card_response.b[0] & BUSY_BIT)
			{
				SdWaitNs(700);
			}
		}while((card_response.b[0] & BUSY_BIT));//�ȴ��������

		//�����ʱ������
		if(!loopguard)
		{
			return card_response.b[0];
		}
	}

	//���Ӧ��ʽΪR1b��ʽ
	else if(current_command.response == R1b)
	{
		loopguard = 0;
		do
		{
			card_response.b[0] = WriteReadSpiByte(0xFF);
			if(!++loopguard) break;
		}
		while((card_response.b[0] & BUSY_BIT));//�ȴ��������

		while(WriteReadSpiByte(0xff) == 0x00);//���ӵ�æ�źţ��ȴ��������(�����ʾ����)

	}

	//���Ӧ��ʽΪR2��ʽ
	else if(current_command.response == R2)
	{
		loopguard=0;
		do
		{
			card_response.b[0] = WriteReadSpiByte(0xFF);
			if(!++loopguard) break;
		}while((card_response.b[0] & BUSY_BIT));//�ȴ��������
		if(!loopguard) BACK_FROM_ERROR;

		card_response.b[1] = WriteReadSpiByte(0xFF);//���ڶ����ظ��ֽ�
	}

	//���Ӧ��ʽΪR3��ʽ
	else
	{
		//R3�ظ�
		loopguard=0;
		do
		{
			card_response.b[0] = WriteReadSpiByte(0xFF);
			if(!++loopguard) break;
		}while((card_response.b[0] & BUSY_BIT));
		if(!loopguard)	BACK_FROM_ERROR;
		
		//��ȡʣ�µ�4���ֽ�(OCR������)
		for(counter = 0; counter <= 3; counter++) 
		{
			*pchar++ = WriteReadSpiByte(0xFF);
		}
	}


	//�жϴ���ģʽ==========================================================================================
	switch(current_command.trans_type)
	{
		//������
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

		//д����
		case WR:
			WriteReadSpiByte(0xFF);
			WriteReadSpiByte(START_SBW);
			EA = 0;
			SD_WRITE_BYTES_ASM(pchar, current_blklen);
			EA = 1;
	
			//��Ӧ���ź�
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


	//��������===================================================================================================
	WriteReadSpiByte(0xFF); 		//ͬ��ʱ�ӣ���Ҫ��(kadyע)
	NSSMD0 = 1; 					//ȡ��SD��ѡ��
	WriteReadSpiByte(0xFF); 		//ͬ��ʱ�ӣ���Ҫ��(kadyע)

	//�ָ���д����
	if((current_command.command_byte == SEND_CSD)||(current_command.command_byte == SEND_CID))
	{
		current_blklen = old_blklen;
	}


	//����Ӧ���ź�===============================================================================================
	return card_response.i;
}

/*****************************************************************************************************************
* ��	�ƣ�SdInit()
* ��	�ܣ�SD����ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
void SdInit(void) reentrant
{
	xdata unsigned loopguard;
	xdata UINT_UNION card_status;
	xdata unsigned char counter = 0;
	unsigned char *pchar; 
	unsigned int c_size,bl_len;
	unsigned char c_mult;

	//SPI��ʼ���Ժ�Ҫ��ʱ=======================================================================================
	Is_Initialized = 0;
	SdWaitMs(100);
	pchar = (unsigned char xdata*)LOCAL_BLOCK;

	
	//SD��������ʱ��============================================================================================
	for(counter = 0; counter < 10; counter++)
	{
		WriteReadSpiByte(0xFF);
	}
	SdWaitMs(1);


	//��λSD��==================================================================================================
	card_status.i = SdCommandExec(GO_IDLE_STATE, EMPTY, EMPTY);


	//����SD�����ȴ���ʼ�����==================================================================================
	loopguard=0;
	do
	{
		SdWaitMs(1);
		card_status.i = SdCommandExec(SEND_OP_COND, EMPTY, EMPTY);
		if(!++loopguard) return;
	}while ((card_status.b[0] & 0x01));


	//��OCR�Ĵ���================================================================================================
	loopguard=0;
	do
	{
		card_status.i = SdCommandExec(READ_OCR,EMPTY,pchar);
		if(!++loopguard) return;
	}while(!(*pchar&0x80));


	//��״̬�Ĵ���===============================================================================================
   	card_status.i = SdCommandExec(SEND_STATUS,EMPTY,EMPTY);


	//��CSD�Ĵ���================================================================================================
	card_status.i = SdCommandExec(SEND_CSD,EMPTY,pchar);

	//����CSD�Ĵ�����ߵ�����
	bl_len = 1 << (pchar[5] & 0x0f);
	c_size = ((pchar[6] & 0x03) << 10) | (pchar[7] << 2) | ((pchar[8] &0xc0) >> 6);
	c_mult = (((pchar[9] & 0x03) << 1) | ((pchar[10] & 0x80) >> 7));

	//����SD���Ĵ�С
	PHYSICAL_BLOCKS = (unsigned long)(c_size+1)*(1 << (c_mult+2));
	PHYSICAL_SIZE = PHYSICAL_BLOCKS * bl_len;

	
	//���ö�д���ݵĿ鳤��=======================================================================================
	card_status.i = SdCommandExec(SET_BLOCKLEN, (unsigned long)PHYSICAL_BLOCK_SIZE, EMPTY);


	//���=======================================================================================================
	Is_Initialized = 1;

}

/*****************************************************************************************************************
* ��	�ƣ�SdBlockRead()
* ��	�ܣ�SD�����ȡ����
* ��ڲ�����address - ��ַ��wdata - ������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned int SdBlockRead(unsigned long address, unsigned char *pchar) reentrant
{
	unsigned int card_status; 			// Stores MMC status after each MMC command;
	
	address *= PHYSICAL_BLOCK_SIZE;

	card_status = SdCommandExec(READ_SINGLE_BLOCK, address, pchar);

	return card_status;
}

/*****************************************************************************************************************
* ��	�ƣ�SdBlockWrite()
* ��	�ܣ�SD�����ȡ����
* ��ڲ�����address - ��ַ��wdata - ������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned char SdBlockWrite(unsigned long address,unsigned char *wdata) reentrant
{
	unsigned int card_status;

	address *= PHYSICAL_BLOCK_SIZE;
	card_status = SdCommandExec(WRITE_BLOCK, address, wdata);

	return card_status;
}

/*****************************************************************************************************************
* ��	�ƣ�SdSectors()
* ��	�ܣ���ȡ��������Ŀ
* ��ڲ�������
* ���ڲ�������
* ˵	������
*****************************************************************************************************************/
unsigned long SdSectors(void) reentrant
{
	return PHYSICAL_BLOCKS;
}

