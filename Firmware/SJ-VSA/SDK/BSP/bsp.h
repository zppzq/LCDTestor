/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210B-EVAL Evaluation Board
*
* Filename      : bsp.h
* Version       : V1.00
* Programmer(s) : Brian Nagel
*********************************************************************************************************
*/

#ifndef  __BSP_H__
#define  __BSP_H__

#define		SYSTEM_VERSION			SJ_RFQC_SC_4D_3_3_4			


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   BSP_GLOBALS
#define  BSP_EXT
#else
#define  BSP_EXT  extern
#endif

#define TP_CS()  GPIO_ResetBits(GPIOG,GPIO_Pin_11)
#define TP_DCS() GPIO_SetBits(GPIOG,GPIO_Pin_11)
#define SYSCLK 72000000

//#define _BIG_ENDIAN_
#ifdef _BIG_ENDIAN_
#define htons(n)	n
#define htonl(n)	n
#define ntohs(n)	n
#define ntohl(n)	n
#else
#define htons(n)  _htons(n)
#define htonl(n)  _htonl(n)  
#define ntohs(n)  _htons(n)
#define ntohl(n)  _htonl(n) 
#define htonf(n)  _htonf(n) 
#define ntohf(n)  _htonf(n)
#endif

/*
#define SetSCK() GPIO_SetBits(GPIOA,GPIO_Pin_5) 
#define ClearSCK() GPIO_ResetBits(GPIOA,GPIO_Pin_5) 
#define SetMOSI()  GPIO_SetBits(GPIOA,GPIO_Pin_7)
#define ClearMOSI() GPIO_ResetBits(GPIOA,GPIO_Pin_7)
#define ReadMISO() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)*/
#define WaitTPReady() while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)==0)

#define POWER_UV_LIMIT 12
uint16 _htons(uint16 n);
uint32 _htonl(uint32 n);
float  _htonf(float f);	

void BSP_Init(void);
void BSP_IntDisAll(void);
void GPIODisable(void);
void GPIOEnable(void);
void BspVariInit(void);

#endif                                                          /* End of module include.                               */
