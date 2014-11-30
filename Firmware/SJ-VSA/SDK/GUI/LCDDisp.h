/****************************************�ļ���Ϣ**************************************************                      
**
**��   ��   ��: LCDDisp.h
**��   ��   ��: �ư�Դ
**�� �� ��  ��: 2007��7��18��
**����޸�����: 2007��8�� 1��
**��        ��: LCD��ʾͷ�ļ�

**��		��: ��п�
**�� ��  �� ��: 2008��4��4��
***************************************************************************************************/
#ifndef 	_LCDDisp_H_
#define 	_LCDDisp_H_


#ifdef		_LCDDisp_C_
#define		LCDDisp_EXT
#else
#define		LCDDisp_EXT		extern
#endif

//������ͷ�ļ�
#include "LCDDriver.h"

//�������  -----------------------------------------------------------
#define  GRAPH_INIT
//#define  LOCATEXY
//#define  DISP_POINT
//#define  HORI_LINE
//#define  VERT_LINE
//#define  SKEW_LINE
//#define  CIRCLE

#define  LCDDISP_STRING
//#define  DISP_LINE
//#define  DISP_RECT
//#define  DISP_CIRC
//#define  DISP_BITMAP


//���ݽṹ����------------------------------------------------------

//LCD���궨��---------------------------------------------------------
#define    XLabel       INT8U
#define    YLabel       INT8U


//���ص����궨��----------------------------------------------------
typedef struct 
{
	XLabel   x;     //���ص�x����
	YLabel   y;		//���ص�y����			
} Point;


//ֱ�߽ṹ����-------------------------------------------------------
typedef struct 
{
	Point     Start;	//ֱ���������
	Point     End;		//ֱ���յ�����
	INT8U	  Width;    //ֱ�߿���
} Line;

//���νṹ����-------------------------------------------------------
typedef struct 
{
	Point     Start;	//�����������
	Point     End;		//�����յ�����
	INT8U	  Width;    //���α߿����
} Rect;


//Բ�νṹ����-------------------------------------------------------
typedef struct 
{
	Point     Centre;	//Բ������
    INT8U	  r;	    //�뾶	
	INT8U	  Width;	//Բ�ܿ���
} Circ;


typedef struct tagBITMAP 
{
    INT8U   bmWidth;        //λͼ����
    INT8U   bmHeight;       //λͼ�߶�
    INT16U *bmBits;         //λͼ��ʾ������ 
} BITMAP, *PBITMAP; 


//��������--------------------------------------------------------------

/****************************************************************************
* ��	�ƣ�DispStr()
* ��	�ܣ���ʾ�ַ�������
* ��ڲ�����pstr ����ʾ���ַ������ݣ��Կ��ַ�����(0x00)
* ���ڲ�������
* ˵	�����������ĺ�ASCII��;
****************************************************************************/
LCDDisp_EXT void DispStr(INT8U nAddr,INT8U *pstr) reentrant;


/****************************************************************************
* ��	�ƣ�GraphInit()
* ��	�ܣ�ͼ����ʾ��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
LCDDisp_EXT void GraphInit() reentrant;


/****************************************************************************
* ��	�ƣ�GraphClear()
* ��	�ܣ�ͼ����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
LCDDisp_EXT void GraphClear() reentrant;

/****************************************************************************
* ��	�ƣ�LocateXY()
* ��	�ܣ��趨��ǰ����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
void LocateXY(XLabel ParaX, YLabel ParaY) reentrant;

/****************************************************************************
* ��	�ƣ�DispPoint()
* ��	�ܣ���ʾ��
* ��ڲ�������
* ���ڲ�������
* ˵	������ʾȫ�ֱ���CurrentPoint�����ص�
****************************************************************************/
LCDDisp_EXT void DispPoint(XLabel x, YLabel y) reentrant;

/****************************************************************************
* ��	�ƣ�HoriLine()
* ��	�ܣ���ʾˮƽֱ�ߺ���
* ��ڲ�����length ����ʾˮƽֱ�ߵĳ���
* ���ڲ�������
* ˵	�����Ե�ǰ����Ϊ��� ����Ϊ1������ 
****************************************************************************/
LCDDisp_EXT void HoriLine(XLabel ParaX, YLabel ParaY, INT8U length) reentrant;

/****************************************************************************
* ��	�ƣ�VertLine()
* ��	�ܣ���ʾ��ֱֱ�ߺ���
* ��ڲ�����length ����ʾ��ֱֱ�ߵĳ���
* ���ڲ�������
* ˵	�����Ե�ǰ����Ϊ��� ����Ϊ1������
****************************************************************************/
LCDDisp_EXT void VertLine(XLabel ParaX, YLabel ParaY, INT8U length) reentrant;


/****************************************************************************
* ��	�ƣ�SkewLine()
* ��	�ܣ���ʾֱ�ߺ���
* ��ڲ�����deltaX x�����ϵ������  deltaY y�����ϵ������
* ���ڲ�������
* ˵	�����Ե�ǰ����Ϊ��� ����Ϊ1������            
****************************************************************************/
LCDDisp_EXT void SkewLine(XLabel ParaX, YLabel ParaY, INT8S deltaX,INT8S deltaY) reentrant;


/****************************************************************************
*��	   �ƣ�Circle()
*��    �ܣ���ʾԲ����
*��ڲ�����Rx �뾶 
*���ڲ�������
*˵    �����Ե�ǰ����ΪԲ�� ����Ϊ1������ ��ѧ����(X-Ox)^2+(Y-Oy)^2=Rx^2     
/****************************************************************************/
LCDDisp_EXT void Circle(XLabel ParaX, YLabel ParaY, INT8U Rx) reentrant;

/****************************************************************************
* ��	�ƣ�DispLine()
* ��	�ܣ���ʾֱ�ߺ���
* ��ڲ�����pline ����ʾֱ�ߵĲ���
* ���ڲ�������
* ˵	������
****************************************************************************/
LCDDisp_EXT void DispLine(Line pline) reentrant;

/****************************************************************************
* ��	�ƣ�DispRect()
* ��	�ܣ���ʾ���κ���
* ��ڲ�����prect ����ʾ�ľ��β���
* ���ڲ�������
* ˵	������
****************************************************************************/
LCDDisp_EXT void DispRect(Rect prect) reentrant;


/****************************************************************************
* ��	�ƣ�DispCirc()
* ��	�ܣ���ʾԲ����
* ��ڲ�����pcirc ����ʾ��Բ�Ĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
LCDDisp_EXT void DispCirc(Circ pcirc) reentrant;


/****************************************************************************
* ��	�ƣ�DispBitmap()
* ��	�ܣ���ʾλͼ����
* ��ڲ�����pbmap ����ʾ��λͼ�Ĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
LCDDisp_EXT void DispBitmap(XLabel ParaX, YLabel ParaY, BITMAP pbmap) reentrant;


//************************************************
#endif