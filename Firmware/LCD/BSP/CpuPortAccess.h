/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: CpuPortAccess.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��1��18��
**��        ��: ������
**����޸�����: 2010��9��17��
**��        ��: �����˹���CPU��һЩ������IO����
********************************************************************************************************/
#ifndef		_CPU_PORT_ACCESS_H_
#define		_CPU_PORT_ACCESS_H_

//private======================================================================
// #define CONNECT3(a,b,c)			(a##b##c)
// #define CONNECT2(a,b)			(a##b)

#define PORT(x)		GPIO##x
#define BIT(x)		(1<<x)

// #define _GETPORTIO(b)			CONNECT3(P,b,MDOUT)
// #define _GETPORT(b)				CONNECT2(P,b)
// 
// #define GETPORTIO(SOMEBIT)		_GETPORTIO(SOMEBIT##_PORT)
// #define GETPORT(SOMEBIT)		_GETPORT(SOMEBIT##_PORT)

//public======================================================================
/////////////////////////////IO�˿�����/////////////////////////////////
#define GPIO_ANALOG_INPUT			0
#define GPIO_FLOATING_INPUT			4
#define GPIO_PULL_UP_DOWN_INPUT		8
#define GPIO_PUSH_PULL_10M_OUTPUT	1
#define GPIO_PUSH_PULL_2M_OUTPUT	2
#define GPIO_PUSH_PULL_50M_OUTPUT	3
#define GPIO_OPEN_DRAIN_10M_OUTPUT	5
#define GPIO_OPEN_DRAIN_2M_OUTPUT	6
#define GPIO_OPEN_DRAIN_50M_OUTPUT	7
#define AFIO_PUSH_PULL_10M_OUTPUT	9
#define AFIO_PUSH_PULL_2M_OUTPUT	10
#define AFIO_PUSH_PULL_50M_OUTPUT	11
#define AFIO_OPEN_DRAIN_10M_OUTPUT	13
#define AFIO_OPEN_DRAIN_2M_OUTPUT	14
#define AFIO_OPEN_DRAIN_50M_OUTPUT	15

///////////////////////////����IO�˿�����/////////////////////////////////
#define SET_IO_ATTRI(port, bit, attri)  	(*(&(port->CRL) + ((bit)>>3))) &= ~(0x0FUL << 4*(bit&0x07)); \
											(*(&(port->CRL) + ((bit)>>3))) |= (attri << 4*(bit&0x07))


#define MakePushPull(bit)			SET_IO_ATTRI(bit##_PORT, bit, GPIO_PUSH_PULL_50M_OUTPUT)
#define MakeOpenDrain(bit)			SET_IO_ATTRI(bit##_PORT, bit, GPIO_OPEN_DRAIN_50M_OUTPUT)

#define SetHi(bit)					(bit##_PORT##->BSRR) = (1<<bit)
#define SetLo(bit)					(bit##_PORT##->BRR)  = (1<<bit)

#define GetSignal(bit)				(((bit##_PORT##->IDR & (1<<bit)) == 0x00)?0:1)	 

#define PortOpen()

#define DataInitZero(x) 			memset(x, 0, sizeof(x))			//��x�����ݿռ��ʼ��Ϊ0
#define DataInit(x, nData)			memset(x, nData, sizeof(x))
		
//************************************************************************
#endif

