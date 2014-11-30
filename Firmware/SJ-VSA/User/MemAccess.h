/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: MemAccess.h
**创   建   人: 杨承凯
**创 建 日  期: 2007年1月16日
**最后修改日期: 2007年3月30日
**描        述: 数据访问定义头文件
********************************************************************************************************/
#ifndef		_MEM_ACCESS_H_
#define		_MEM_ACCESS_H_


//数据类型定义-------------------------------------------------------------------

#define		TRUE			1
#define		FALSE			0

#define		true			1
#define		false			0

#ifndef	BWRITE		
#define	BWRITE 		0
#endif

#ifndef	BREAD
#define	BREAD 		1
#endif


//数据访问-----------------------------------------------------------------
//注：大端编译模式（C51默认为大端编译）
//
//得到一个字的高字节
#define HIBYTE(x)	(*((uint8*)(&x)))

//得到一个字的低字节
#define LOBYTE(x)	(*((uint8*)(&x)+1))

//得到某个变量的第i个字节，低地址开始
#define LABYTE(val,i)		(*((uint8*)(&val)+i))


//
//
//在data区获取X地址上的8位整型
#define  DMEM_B( x )  ( *( (uint8 volatile data*) (x) ) ) 

//在data区获取x地址上的16位整型
#define  DMEM_W( x )  ( *( (uint16 volatile data*) (x) ) )

//在data区获取x地址上的32位整型
#define  DMEM_L( x )  ( *( (uint32 volatile data*) (x) ) )

//在data区获取x地址上的32位浮点数
#define  DMEM_F( x )  ( *( (fp32 volatile data*) (x) ) )

//
//
//在pdata区获取X地址上的8位整型
#define  PMEM_B( x )  ( *( (uint8 volatile pdata*) (x) ) ) 

//在pdata区获取x地址上的16位整型
#define  PMEM_W( x )  ( *( (uint16 volatile pdata*) (x) ) )

//在pdata区获取x地址上的32位整型
#define  PMEM_L( x )  ( *( (uint32 volatile pdata*) (x) ) )

//在pdata区获取x地址上的32位浮点数
#define  PMEM_F( x )  ( *( (fp32 volatile pdata*) (x) ) )

//
//
//在xdata区获取X地址上的8位整型
#define  XMEM_B( x )  ( *( (uint8 volatile xdata*) (x) ) ) 

//在xdata区获取x地址上的16位整型
#define  XMEM_W( x )  ( *( (uint16 volatile xdata*) (x) ) )

//在xdata区获取x地址上的32位整型
#define  XMEM_L( x )  ( *( (uint32 volatile xdata*) (x) ) )

//在xdata区获取x地址上的32位浮点数
#define  XMEM_F( x )  ( *( (fp32 volatile xdata*) (x) ) )

//
//
//在code区获取X地址上的8位整型
#define  CMEM_B( x )  ( *( (uint8 volatile code*) (x) ) ) 

//在code区获取x地址上的16位整型
#define  CMEM_W( x )  ( *( (uint16 volatile code*) (x) ) )

//在code区获取x地址上的32位整型
#define  CMEM_L( x )  ( *( (uint32 volatile code*) (x) ) )

//在code区获取x地址上的32位浮点数
#define  CMEM_F( x )  ( *( (fp32 volatile code*) (x) ) )




//地址解析------------------------------------------------------------------
//
//
//得到一个域在结构体中的偏移量
#define FPOS( type, field ) 	( (uint16) (&(( type *) 0)-> field) )

//得到一个域在结构体中占的字节数
#define FSIZ( type, field ) 	sizeof( ((type *) 0)->field ) 


//--------------------------------------------------------------------------
#endif
