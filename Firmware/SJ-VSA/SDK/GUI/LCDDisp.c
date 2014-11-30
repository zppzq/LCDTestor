/****************************************�ļ���Ϣ**************************************************                      
**��   ��   ��: LCDDisp.c
**��   ��   ��: �ư�Դ
**�� �� ��  ��: 2007��7��16��
**����޸�����: 2007��8�� 6��
**��        ��: LCD��ʾԴ�ļ�

**��		��: ��п�
**�� ��  �� ��: 2008��4��4��
***************************************************************************************************/
#define	_LCDDisp_C_

#include "..\BSP\bsp.h"
#include "LCDDriver.h"
#include "LCDDisp.h"


//LCDָ���
//����ָ��
#define  LCDClear               0x01            //����ָ��
#define  LCDResetAC             0x03            //��ַ��λ
#define  LCDDispOn              0x0C            //��ʾ��,����,������ʾ��
#define  LCDOpenExtIns          0x34            //����չָ��
#define  LCDOpenGraph           0x36            //����ͼ
#define  LCDBaseIns             0x30            //���û�������ָ��
#define  LCDSetCGRAMAddr(x)     0x40|(x&0x3F)   //����CGRAM��ַ
#define  LCDSetDDRAMAddr(x)     0x80|(x&0x3F)   //����DDRAM��ַ
//��չָ��
#define  LCDCloseExtIns         0x32            //����չָ�� ������ͼ
#define  LCDDispReverse(x)      0x04|(x&0x03)   //���巴ת��ʾ���� x 0:��ת��һ�� 
                                                //1:��ת�ڶ��� 2,3:���о�����ת
#define  LCDGraphCol(x)         0x80|(x&0x0F)   //ͼ����ʾ�е�ַ
#define  LCDGraphRow(y)         0x80|(y&0x1F)   //ͼ����ʾ�е�ַ



/****************************************************************************
* ��	�ƣ�GraphInit()
* ��	�ܣ�ͼ����ʾ��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef GRAPH_INIT
void GraphInit() reentrant
{
	LcdWriteCmd( LCDBaseIns );
    OSTimeDly(1);

    LcdWriteCmd( LCDClear );
    OSTimeDly(1);
    
    LcdWriteCmd( LCDDispOn );
    OSTimeDly(1);

    LcdWriteCmd( LCDOpenGraph );
    OSTimeDly(1);

	GraphClear();
}
#endif

/****************************************************************************
* ��	�ƣ�GraphClear()
* ��	�ܣ�ͼ����������
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef GRAPH_INIT
void GraphClear() reentrant
{
	INT8U temp,i,j;
    temp=0x80;
    for(i=0;i<32;i++)          //ͼ������
    {
        LcdWriteCmd(temp);
        LcdWriteCmd(0x80);
        for(j=0;j<16;j++)
        {
  	        LcdWriteData(0x00);
            LcdWriteData(0x00);
        }
        temp++;
        LcdDelaynus(10);
    }
	
	LcdWriteCmd( LCDBaseIns );
    LcdWriteCmd( LCDClear );
    LcdWriteCmd( LCDOpenGraph );
}
#endif


/****************************************************************************
* ��	�ƣ�DispStr()
* ��	�ܣ���ʾ�ַ�������
* ��ڲ�����pstr ����ʾ���ַ������ݣ��Կ��ַ�����(0x00)
* ���ڲ�������
* ˵	�����������ĺ�ASCII��;
****************************************************************************/
#ifdef LCDDISP_STRING
void DispStr(INT8U nAddr,INT8U *pstr) reentrant
{
    INT8U i,adtemp,Index;
	INT8U ChsRoEng;		//1,Chinese; 0,English

    LcdWriteCmd( LCDCloseExtIns );
    LcdDelaynus(100);
    switch( nAddr/8 )
    {
        case 0 : adtemp = nAddr;break;
        case 1 : adtemp = nAddr + 8;break;
        case 2 : adtemp = nAddr - 8;break;
        case 3 : adtemp = nAddr;break;
        default: break;
    }

    LcdWriteCmd( LCDSetDDRAMAddr(adtemp) );
    LcdDelaynus(20);

	ChsRoEng = (pstr[0]&0x80 > 0)?1:0;
    for(i = 0,Index = 0; pstr[Index] != 0; i++,Index++)
    {
		if(((pstr[Index]&0x80) > 0) && (ChsRoEng == 0))
		{
			if((i&0x01) > 0)
			{
				LcdWriteData(' ');
				i++;
			}
			ChsRoEng = 1;
		}
		if((pstr[Index]&0x80) == 0)
		{
			ChsRoEng = 0;
		}

		LcdWriteData( *(pstr + Index));

        if(nAddr + (i>>1)  == 7 && (i&0x01) == 1)
        {
            LcdDelaynus(50);
            LcdWriteCmd( LCDSetDDRAMAddr(16) );
            LcdDelaynus(20);
        }
        else if(nAddr + (i>>1)  == 15 && (i&0x01) == 1)
        {
            LcdDelaynus(50);
            LcdWriteCmd( LCDSetDDRAMAddr(8) );
            LcdDelaynus(20);
        }
        else if(nAddr + (i>>1)  == 23 && (i&0x01) == 1)
        {
            LcdDelaynus(50);
            LcdWriteCmd( LCDSetDDRAMAddr(24) );
            LcdDelaynus(20);
        }
        else if(nAddr + (i>>1)  == 31 && (i&0x01) == 1)
        {
            break;
        }
               
    }
    LcdDelaynus(100);
    LcdWriteCmd( LCDOpenGraph );
    LcdDelaynus(100);
}
#endif

/****************************************************************************
* ��	�ƣ�LocateXY()
* ��	�ܣ��趨��ǰ����
* ��ڲ�������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef LOCATEXY
static void LocateXY(XLabel ParaX, YLabel ParaY) reentrant
{
	XLabel x;
	YLabel y;
	x = ((ParaX&0x7F) >> 4) |((ParaY&0x20) >> 2);
	y = ParaY & 0x1F;

	LcdWriteCmd(LCDGraphRow(y));
    LcdDelaynus(5);
	LcdWriteCmd(LCDGraphCol(x));
    LcdDelaynus(8);
}
#endif



/****************************************************************************
* ��	�ƣ�DispPoint()
* ��	�ܣ���ʾ��
* ��ڲ�������
* ���ڲ�������
* ˵	������ʾȫ�ֱ���CurrentPoint�����ص�
****************************************************************************/
#ifdef DISP_POINT
void DispPoint(XLabel x, YLabel y) reentrant
{
	INT8U	CurrentData[2];
	INT16U   temp;
	
	if((x > 127) || (y > 63)) return;
	
	LocateXY(x,y);
    LcdReadData(CurrentData,2);
    temp = 0x8000 >> (x &0x0F);
    LocateXY(x,y);
    CurrentData[0] = CurrentData[0] | HIBYTE(temp);
    LcdWriteData(CurrentData[0] );
    CurrentData[1] = CurrentData[1] | LOBYTE(temp);
    LcdWriteData(CurrentData[1] ); 
    
	LcdDelaynus(7);
}
#endif


/****************************************************************************
* ��	�ƣ�HoriLine()
* ��	�ܣ���ʾˮƽֱ�ߺ���
* ��ڲ�����length ����ʾˮƽֱ�ߵĳ���
* ���ڲ�������
* ˵	�����Ե�ǰ����Ϊ��� ���Ϊ1������
****************************************************************************/
#ifdef  HORI_LINE
void HoriLine(XLabel ParaX, YLabel ParaY, INT8U length) reentrant
{
    length += ParaX;
	for(;ParaX <= length;ParaX++)  
	{
        DispPoint(ParaX, ParaY);
	}
}
#endif


/****************************************************************************
* ��	�ƣ�VertLine()
* ��	�ܣ���ʾ��ֱֱ�ߺ���
* ��ڲ�����length ����ʾ��ֱֱ�ߵĳ���
* ���ڲ�������
* ˵	�����Ե�ǰ����Ϊ��� ���Ϊ1������
****************************************************************************/
#ifdef  VERT_LINE
void VertLine(XLabel ParaX, YLabel ParaY, INT8U length) reentrant
{
	length += ParaY;
	for(;ParaY <= length;ParaY++)  
    {
        DispPoint(ParaX,ParaY); 
    }
}
#endif


/****************************************************************************
* ��	�ƣ�SkewLine()
* ��	�ܣ���ʾֱ�ߺ���
* ��ڲ�����deltaX x�����ϵ������  deltaY y�����ϵ������
* ���ڲ�������
* ˵	�����Ե�ǰ����Ϊ��� ���Ϊ1������            
****************************************************************************/
#ifdef  SKEW_LINE
void SkewLine(XLabel ParaX, YLabel ParaY, INT8S deltaX,INT8S deltaY) reentrant
{
	register INT8U t;
	INT8S incx,incy;
	INT8U basex=0,basey=0,distance;
	//������������
	if(deltaX > 0) incx = 1;
	else  incx = -1;
	if(deltaY > 0) incy = 1;
	else  incy = -1;
	//�����ĸ�����Ƚϴ�
	deltaX *= incx;
	deltaY *= incy;
	if(deltaX > deltaY) distance = deltaX;
	else  distance = deltaY;
	 //��ʼ����
	for (t=0; t<=distance+1; t++)
	{
        DispPoint(ParaX,ParaY);
		basex += deltaX;
		basey += deltaY;
		if(basex >= distance)
		{
			basex -=distance;
			ParaX += incx;
		}
		if(basey >= distance)
		{
			basey -=distance;
			ParaY += incy;
		}
	}
}
#endif



/****************************************************************************
*��	   �ƣ�Circle()
*��    �ܣ���ʾԲ����
*��ڲ������뾶Rx
*���ڲ�������
*˵    �����Ե�ǰ����ΪԲ�� ���Ϊ1������ ��ѧ����(X-Ox)^2+(Y-Oy)^2=Rx^2     
/****************************************************************************/
#ifdef	CIRCLE
void circle(XLabel ParaX, YLabel ParaY, INT8U Rx) reentrant
{
    INT8U  xt,yt,Ox,Oy;
    INT16U rr,rs,xx;
    Ox = ParaX;
    Oy = ParaY;
    yt = Rx;
    rr = Rx * Rx + 1; //���� 1 ��������
    rs = ( yt + ( yt >> 1 ) ) >> 1;   //(*0.75)�ֿ�1/8Բ������
    for ( xt = 0;xt <= rs;xt++)
    {
        xx = xt * xt;
        while (( yt * yt )>( rr - xx )) yt--;

        ParaX = Ox + xt;    //��һ����
        ParaY = Oy - yt;
        DispPoint(ParaX,ParaY);


        ParaX = Ox - xt;    //�ڶ�����
        DispPoint(ParaX,ParaY);

        ParaY = Oy + yt;   //��������
        DispPoint(ParaX,ParaY);

        ParaX = Ox + xt;    //��������
        DispPoint(ParaX,ParaY);

/***************45�Ⱦ�����һ��***************/

        ParaX = Ox + yt;    //��һ����
        ParaY = Oy - xt;
        DispPoint(ParaX,ParaY);

        ParaX = Ox - yt;    //�ڶ�����
        DispPoint(ParaX,ParaY);

        ParaY = Oy + xt;    //��������
        DispPoint(ParaX,ParaY);

        ParaX = Ox + yt;    //��������
        DispPoint(ParaX,ParaY);

     }
}
#endif

/****************************************************************************
* ��	�ƣ�DispLine()
* ��	�ܣ���ʾֱ�ߺ���
* ��ڲ�����pline ����ʾֱ�ߵĲ���
* ���ڲ�������
* ˵	������           
****************************************************************************/
#ifdef  DISP_LINE
void DispLine(Line pline) reentrant
{
    INT8S deltaX,deltaY,i,incx,incy;
	XLabel ParaX;
	YLabel ParaY;
	deltaX = pline.End.x - pline.Start.x;
	deltaY = pline.End.y - pline.Start.y;
	if(deltaY == 0) 
	{
		//���Ϊżʱ �ϱ��¶�1����
		for(i=-(pline.Width/2);i <= (pline.Width-1)/2;i++)
		{
			ParaX = pline.Start.x;
			ParaY = pline.Start.y + i;
			HoriLine(ParaX,ParaY,deltaX);
		}
	}
	else if(deltaX == 0)
	{
		//���Ϊżʱ ����Ҷ�1����
		for(i=-(pline.Width/2);i <= (pline.Width-1)/2;i++)
		{
			ParaY = pline.Start.y;
			ParaX = pline.Start.x +i;
			VertLine(ParaX,ParaY,deltaY);
		}
	}
	else
	{
		//������������
		if(deltaX > 0) incx = 1;
		else  incx = -1;
		if(deltaY > 0) incy = 1;
		else  incy = -1;

		ParaX = pline.Start.x;
		ParaY = pline.Start.y;
		SkewLine(ParaX,ParaY,deltaX,deltaY);
        deltaX -= incx;
        deltaY -= incy;
		for(i=1;i <= pline.Width/2;i++)
		{
			ParaX = pline.Start.x + i * incx;
			ParaY = pline.Start.y;
			SkewLine(ParaX,ParaY,deltaX,deltaY);
			ParaX = pline.Start.x;
			ParaY = pline.Start.y + i * incy;
			SkewLine(ParaX,ParaY,deltaX,deltaY);
            deltaX -= incx;
            deltaY -= incy;
		}
	}	
}
#endif


/****************************************************************************
* ��	�ƣ�DispRect()
* ��	�ܣ���ʾ���κ���
* ��ڲ�����prect ����ʾ�ľ��β���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef  DISP_RECT
void DispRect(Rect prect) reentrant
{
    INT8U temp;
    Line pline;
    temp = prect.Width;
	pline.Width = temp;
    pline.Start.x = prect.Start.x - prect.Width / 2;
    pline.Start.y = prect.Start.y ;          
    pline.End.x = prect.End.x + (prect.Width-1) / 2;
	pline.End.y = prect.Start.y;
    DispLine(pline);

    pline.Start.x = prect.Start.x;
    pline.Start.y = prect.Start.y + (prect.Width-1) / 2;          
    pline.End.x = prect.Start.x;
	pline.End.y = prect.End.y - prect.Width / 2;
    DispLine(pline);

    pline.Start.x = prect.End.x;
    pline.Start.y = prect.Start.y + (prect.Width-1) / 2;          
    pline.End.x = prect.End.x;
	pline.End.y = prect.End.y - prect.Width / 2;
    DispLine(pline);
        
    pline.Start.x = prect.Start.x - prect.Width / 2;
    pline.Start.y = prect.End.y ;          
    pline.End.x = prect.End.x + (prect.Width-1) / 2;
	pline.End.y = prect.End.y;
    DispLine(pline); 
}
#endif


/****************************************************************************
* ��	�ƣ�DispCirc()
* ��	�ܣ���ʾԲ����
* ��ڲ�����pcirc ����ʾ��Բ�Ĳ���
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef  DISP_CIRC
void DispCirc(Circ pcirc) reentrant
{
    INT8S i;
	XLabel ParaX;
	YLabel ParaY;	
	for(i=-(pcirc.Width/2);i <= (pcirc.Width-1)/2;i++)
	{
		ParaX = pcirc.Centre.x;
		ParaY = pcirc.Centre.y;
		Circle(ParaX,ParaY,pcirc.r + i);
	}
}
#endif		


/****************************************************************************
* ��	�ƣ�DispBitmap()
* ��	�ܣ���ʾλͼ����
* ��ڲ�����pbmap ����ʾ��λͼ������
* ���ڲ�������
* ˵	������
****************************************************************************/
#ifdef  DISP_BITMAP
void DispBitmap(XLabel ParaX, YLabel ParaY, BITMAP pbmap) reentrant
{
    INT8U  offsetx1,offsetx2,lenx,endy,i,tempx;
	INT8U	CurrentData[2];
	INT16U  *pdat;
	INT16U   temp1,temp2,temp3;
    

	pdat = pbmap.bmBits;
	offsetx1= ParaX % 16;
	lenx = (offsetx1+ pbmap.bmWidth) / 16;
	offsetx2 = (offsetx1+ pbmap.bmWidth) %16;
	endy = ParaY + pbmap.bmHeight;
    tempx = ParaX;

	for(;ParaY < endy;ParaY++)
	{
		if(offsetx1 == 0)
		{
			LocateXY(ParaX,ParaY);
			for(i = 0;i < lenx;i++)
			{
				LcdWriteData( HIBYTE(*pdat));   
                LcdDelaynus(3);                       
				LcdWriteData( LOBYTE(*pdat++));	
                LcdDelaynus(3);
                						
			}
			if(offsetx2 !=0)
			{
				temp2 = 0;
                ParaX += 16;
                for(i = 0;i < offsetx2;i++)
				{
					temp2 |= (0x8000 >> i );
				}
                LocateXY(ParaX,ParaY);	
                LcdReadData(CurrentData,2);
			    temp3 = CurrentData[0];
			    temp3 = (temp3 << 8) + CurrentData[1];
			    temp3 &= (~temp2);
			    temp1 = (*pdat++) &temp2 | temp3;
                LocateXY(ParaX,ParaY);					
			    LcdWriteData( HIBYTE(temp1));
                LcdDelaynus(3);
			    LcdWriteData( LOBYTE(temp1));
                LcdDelaynus(3);
                ParaX = tempx;
			}
			LcdDelaynus(15);					
		}
		else
		{
			LocateXY(ParaX,ParaY);
			LcdReadData(CurrentData,2);
			temp1 = CurrentData[0];
			temp1 = (temp1 << 8) + CurrentData[1];
            temp2 = 0;
			for(i =0;i < offsetx1;i++)
				{
					temp2 |= (0x8000 >> i ); 
				}
			temp1 &= temp2;
			temp2 = *pdat++;
			temp1 |= (temp2 >> offsetx1);
            LocateXY(ParaX,ParaY);
			LcdWriteData( HIBYTE(temp1));
            LcdDelaynus(3);
			LcdWriteData( LOBYTE(temp1));
            LcdDelaynus(3);
            temp1 = temp2;
            ParaX += 16;
					
			if(lenx > 1)
			{
				for(i = 1;i < lenx;i++)
				{
					temp2 = *pdat++;
					temp1 = (temp1 << (16-offsetx1)) | (temp2 >> offsetx1);
					LcdWriteData( HIBYTE(temp1));
                    LcdDelaynus(3);
					LcdWriteData( LOBYTE(temp1));
                    LcdDelaynus(5);
					temp1 = temp2;
                    ParaX += 16;
				}

			}

			if(offsetx2 !=0)
			{
				temp2 = 0;
                for(i = 0;i < offsetx2;i++)
				{
					temp2 |= (0x8000 >> i );
				}
                LocateXY(ParaX,ParaY);	
                LcdReadData(CurrentData,2);
			    temp3 = CurrentData[0];
			    temp3 = (temp3 << 8) + CurrentData[1];
			    temp3 &= (~temp2);
			    temp1 = (temp1 << (16-offsetx1))| temp3;
                LocateXY(ParaX,ParaY);					
			    LcdWriteData( HIBYTE(temp1));
                LcdDelaynus(3);
			    LcdWriteData( LOBYTE(temp1));
                LcdDelaynus(3);
                ParaX = tempx;
			}
            LcdDelaynus(5);			
		}
        LcdDelaynus(10);
	}	
}
#endif	



