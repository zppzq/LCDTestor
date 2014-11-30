#ifndef __INCLUDES_H__
#define __INCLUDES_H__


//预先包含的头文件************************************************************************************************
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_it.h"
//#include "lights.h"

//data type define--------------------------------------
typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */


typedef INT8U   BYTE ;          //兼容以前版本的数据类型
typedef INT16U  WORD ;
typedef INT32U  LONG;
typedef	INT32U  DWORD;

typedef	INT8U	uint8;		
typedef	INT8S	int8;		
typedef	INT16U	uint16;		
typedef	INT16S	int16;		
typedef	INT32U	uint32;	
typedef	INT32S	int32;		
typedef	FP32	fp32;		
typedef	FP64	fp64;		
#define reentrant
#define SYS_OPT_SEG
#define idata
#define xdata
#define _nop_()

//user add********************************************************************************************************
//板级开发包头文件，根据实际用到的板级驱动用户自增减
//注：此处包含的头文件将决定函数BspInit()中所要选择编译的内容
#include "LCD_HSD080.h"
#include "./BSP/bsp.h"


#endif		    
