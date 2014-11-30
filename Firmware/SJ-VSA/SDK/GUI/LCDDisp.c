/****************************************文件信息**************************************************                      
**文   件   名: LCDDisp.c
**创   建   人: 黄安源
**创 建 日  期: 2007年7月16日
**最后修改日期: 2007年8月 6日
**描        述: LCD显示源文件

**改		编: 杨承凯
**改 编  日 期: 2008年4月4日
***************************************************************************************************/
#define	_LCDDisp_C_

#include "..\BSP\bsp.h"
#include "LCDDriver.h"
#include "LCDDisp.h"


//LCD指令定义
//基本指令
#define  LCDClear               0x01            //清屏指令
#define  LCDResetAC             0x03            //地址归位
#define  LCDDispOn              0x0C            //显示开,光标关,反白显示关
#define  LCDOpenExtIns          0x34            //开扩展指令
#define  LCDOpenGraph           0x36            //开绘图
#define  LCDBaseIns             0x30            //设置基本操作指令
#define  LCDSetCGRAMAddr(x)     0x40|(x&0x3F)   //生成CGRAM地址
#define  LCDSetDDRAMAddr(x)     0x80|(x&0x3F)   //生成DDRAM地址
//扩展指令
#define  LCDCloseExtIns         0x32            //关扩展指令 保留绘图
#define  LCDDispReverse(x)      0x04|(x&0x03)   //字体反转显示控制 x 0:反转第一行 
                                                //1:反转第二行 2,3:两行均不反转
#define  LCDGraphCol(x)         0x80|(x&0x0F)   //图形显示列地址
#define  LCDGraphRow(y)         0x80|(y&0x1F)   //图形显示行地址



/****************************************************************************
* 名	称：GraphInit()
* 功	能：图形显示初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
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
* 名	称：GraphClear()
* 功	能：图形清屏函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef GRAPH_INIT
void GraphClear() reentrant
{
	INT8U temp,i,j;
    temp=0x80;
    for(i=0;i<32;i++)          //图形清屏
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
* 名	称：DispStr()
* 功	能：显示字符串函数
* 入口参数：pstr 待显示的字符串数据，以空字符结束(0x00)
* 出口参数：无
* 说	明：包括中文和ASCII码;
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
* 名	称：LocateXY()
* 功	能：设定当前坐标
* 入口参数：无
* 出口参数：无
* 说	明：无
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
* 名	称：DispPoint()
* 功	能：显示点
* 入口参数：无
* 出口参数：无
* 说	明：显示全局变量CurrentPoint的像素点
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
* 名	称：HoriLine()
* 功	能：显示水平直线函数
* 入口参数：length 待显示水平直线的长度
* 出口参数：无
* 说	明：以当前坐标为起点 宽度为1个像素
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
* 名	称：VertLine()
* 功	能：显示垂直直线函数
* 入口参数：length 待显示垂直直线的长度
* 出口参数：无
* 说	明：以当前坐标为起点 宽度为1个像素
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
* 名	称：SkewLine()
* 功	能：显示直线函数
* 入口参数：deltaX x方向上的坐标差  deltaY y方向上的坐标差
* 出口参数：无
* 说	明：以当前坐标为起点 宽度为1个像素            
****************************************************************************/
#ifdef  SKEW_LINE
void SkewLine(XLabel ParaX, YLabel ParaY, INT8S deltaX,INT8S deltaY) reentrant
{
	register INT8U t;
	INT8S incx,incy;
	INT8U basex=0,basey=0,distance;
	//计算增量方向
	if(deltaX > 0) incx = 1;
	else  incx = -1;
	if(deltaY > 0) incy = 1;
	else  incy = -1;
	//区别哪个距离比较大
	deltaX *= incx;
	deltaY *= incy;
	if(deltaX > deltaY) distance = deltaX;
	else  distance = deltaY;
	 //开始画线
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
*名	   称：Circle()
*功    能：显示圆函数
*入口参数：半径Rx
*出口参数：无
*说    明：以当前坐标为圆心 宽度为1个像素 数学方程(X-Ox)^2+(Y-Oy)^2=Rx^2     
/****************************************************************************/
#ifdef	CIRCLE
void circle(XLabel ParaX, YLabel ParaY, INT8U Rx) reentrant
{
    INT8U  xt,yt,Ox,Oy;
    INT16U rr,rs,xx;
    Ox = ParaX;
    Oy = ParaY;
    yt = Rx;
    rr = Rx * Rx + 1; //补偿 1 修正方形
    rs = ( yt + ( yt >> 1 ) ) >> 1;   //(*0.75)分开1/8圆弧来画
    for ( xt = 0;xt <= rs;xt++)
    {
        xx = xt * xt;
        while (( yt * yt )>( rr - xx )) yt--;

        ParaX = Ox + xt;    //第一象限
        ParaY = Oy - yt;
        DispPoint(ParaX,ParaY);


        ParaX = Ox - xt;    //第二象限
        DispPoint(ParaX,ParaY);

        ParaY = Oy + yt;   //第三象限
        DispPoint(ParaX,ParaY);

        ParaX = Ox + xt;    //第四象限
        DispPoint(ParaX,ParaY);

/***************45度镜象画另一半***************/

        ParaX = Ox + yt;    //第一象限
        ParaY = Oy - xt;
        DispPoint(ParaX,ParaY);

        ParaX = Ox - yt;    //第二象限
        DispPoint(ParaX,ParaY);

        ParaY = Oy + xt;    //第三象限
        DispPoint(ParaX,ParaY);

        ParaX = Ox + yt;    //第四象限
        DispPoint(ParaX,ParaY);

     }
}
#endif

/****************************************************************************
* 名	称：DispLine()
* 功	能：显示直线函数
* 入口参数：pline 待显示直线的参数
* 出口参数：无
* 说	明：无           
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
		//宽度为偶时 上比下多1像素
		for(i=-(pline.Width/2);i <= (pline.Width-1)/2;i++)
		{
			ParaX = pline.Start.x;
			ParaY = pline.Start.y + i;
			HoriLine(ParaX,ParaY,deltaX);
		}
	}
	else if(deltaX == 0)
	{
		//宽度为偶时 左比右多1像素
		for(i=-(pline.Width/2);i <= (pline.Width-1)/2;i++)
		{
			ParaY = pline.Start.y;
			ParaX = pline.Start.x +i;
			VertLine(ParaX,ParaY,deltaY);
		}
	}
	else
	{
		//计算增量方向
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
* 名	称：DispRect()
* 功	能：显示矩形函数
* 入口参数：prect 待显示的矩形参数
* 出口参数：无
* 说	明：无
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
* 名	称：DispCirc()
* 功	能：显示圆函数
* 入口参数：pcirc 待显示的圆的参数
* 出口参数：无
* 说	明：无
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
* 名	称：DispBitmap()
* 功	能：显示位图函数
* 入口参数：pbmap 待显示的位图的数据
* 出口参数：无
* 说	明：无
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



