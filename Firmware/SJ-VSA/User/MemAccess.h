/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: MemAccess.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2007��1��16��
**����޸�����: 2007��3��30��
**��        ��: ���ݷ��ʶ���ͷ�ļ�
********************************************************************************************************/
#ifndef		_MEM_ACCESS_H_
#define		_MEM_ACCESS_H_


//�������Ͷ���-------------------------------------------------------------------

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


//���ݷ���-----------------------------------------------------------------
//ע����˱���ģʽ��C51Ĭ��Ϊ��˱��룩
//
//�õ�һ���ֵĸ��ֽ�
#define HIBYTE(x)	(*((uint8*)(&x)))

//�õ�һ���ֵĵ��ֽ�
#define LOBYTE(x)	(*((uint8*)(&x)+1))

//�õ�ĳ�������ĵ�i���ֽڣ��͵�ַ��ʼ
#define LABYTE(val,i)		(*((uint8*)(&val)+i))


//
//
//��data����ȡX��ַ�ϵ�8λ����
#define  DMEM_B( x )  ( *( (uint8 volatile data*) (x) ) ) 

//��data����ȡx��ַ�ϵ�16λ����
#define  DMEM_W( x )  ( *( (uint16 volatile data*) (x) ) )

//��data����ȡx��ַ�ϵ�32λ����
#define  DMEM_L( x )  ( *( (uint32 volatile data*) (x) ) )

//��data����ȡx��ַ�ϵ�32λ������
#define  DMEM_F( x )  ( *( (fp32 volatile data*) (x) ) )

//
//
//��pdata����ȡX��ַ�ϵ�8λ����
#define  PMEM_B( x )  ( *( (uint8 volatile pdata*) (x) ) ) 

//��pdata����ȡx��ַ�ϵ�16λ����
#define  PMEM_W( x )  ( *( (uint16 volatile pdata*) (x) ) )

//��pdata����ȡx��ַ�ϵ�32λ����
#define  PMEM_L( x )  ( *( (uint32 volatile pdata*) (x) ) )

//��pdata����ȡx��ַ�ϵ�32λ������
#define  PMEM_F( x )  ( *( (fp32 volatile pdata*) (x) ) )

//
//
//��xdata����ȡX��ַ�ϵ�8λ����
#define  XMEM_B( x )  ( *( (uint8 volatile xdata*) (x) ) ) 

//��xdata����ȡx��ַ�ϵ�16λ����
#define  XMEM_W( x )  ( *( (uint16 volatile xdata*) (x) ) )

//��xdata����ȡx��ַ�ϵ�32λ����
#define  XMEM_L( x )  ( *( (uint32 volatile xdata*) (x) ) )

//��xdata����ȡx��ַ�ϵ�32λ������
#define  XMEM_F( x )  ( *( (fp32 volatile xdata*) (x) ) )

//
//
//��code����ȡX��ַ�ϵ�8λ����
#define  CMEM_B( x )  ( *( (uint8 volatile code*) (x) ) ) 

//��code����ȡx��ַ�ϵ�16λ����
#define  CMEM_W( x )  ( *( (uint16 volatile code*) (x) ) )

//��code����ȡx��ַ�ϵ�32λ����
#define  CMEM_L( x )  ( *( (uint32 volatile code*) (x) ) )

//��code����ȡx��ַ�ϵ�32λ������
#define  CMEM_F( x )  ( *( (fp32 volatile code*) (x) ) )




//��ַ����------------------------------------------------------------------
//
//
//�õ�һ�����ڽṹ���е�ƫ����
#define FPOS( type, field ) 	( (uint16) (&(( type *) 0)-> field) )

//�õ�һ�����ڽṹ����ռ���ֽ���
#define FSIZ( type, field ) 	sizeof( ((type *) 0)->field ) 


//--------------------------------------------------------------------------
#endif
