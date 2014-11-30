/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: Nrzi.c
**创   建   人: 杨承凯
**创 建 日  期: 2009年7月7日
**最后修改日期: 
**描        述: 反向不归零编码源文件
*****************************************************************************************************************/
#include "..\Global.h"

//宏参数定义
#define NRZI_UNCHANGE_MAX	6

//反向不归零编码

uint8 NrziEncode0(uint8* pIn, uint8 nIn, uint8* pOut)
{
	uint8 i, nSrcBitCount, nCodeBitCount, nUnchangeCount;
	uint8 nSrcByte;
	uint8 nCodeByte;
	uint8 nCodeCurBit;
	uint8 nCodeByteCount;

	nCodeCurBit = 1;			//一开始默认为高电平
	nCodeBitCount = 0;			//已经编码位的个数
	nCodeByteCount = 0;			//已经编码字节的个数
	nUnchangeCount = 0;			//连续1的个数
	nCodeByte = 0;				//编码后的字节

	//循环编码
	for(i = 0; i < nIn; i++)
	{
		//获取源字节
		nSrcByte = pIn[i];

		for(nSrcBitCount = 0; nSrcBitCount < 8; nSrcBitCount++)
		{
			//处理源数据得到当前位(从高位出)========================================
			//当前源数据位为0
			if((nSrcByte & 0x80) == 0)					
			{
				nCodeCurBit = !nCodeCurBit;				//跳变
				nUnchangeCount = 0;						//电平不变次数归零	
			}
			
			//当前源数据位为1 
			else										//当前源数据位为1
			{
				nUnchangeCount++;						//电平不变次数增加
			}

			//源数据左移一位
			nSrcByte <<= 1;

			//形成目标数据(从低位入)================================================
			nCodeByte <<= 1;							//目标数据给准备一位
			if(nCodeCurBit == 0) nCodeByte &= ~0x01;	//置零位
			else			 nCodeByte |= 0x01;	
			nCodeBitCount++;

			//如果已经满一个字节，切换到下一字节
			if(nCodeBitCount >= 8)
			{
				pOut[nCodeByteCount++] = nCodeByte;		//切换字节
				nCodeBitCount = 0;						//编码位个数清零				
			}

			//如果连续1的个数超过限制，加入零
			if(nUnchangeCount >= NRZI_UNCHANGE_MAX)
			{
				nCodeCurBit = !nCodeCurBit;					//跳变(加入的零)
				nUnchangeCount = 0;							//连续1个数清零

				//形成目标数据
				nCodeByte <<= 1;							//目标数据给准备一位
				if(nCodeCurBit == 0) nCodeByte &= ~0x01;	//置零位
				else			 nCodeByte |= 0x01;	
				nCodeBitCount++;

				//如果已经满一个字节，切换到下一字节
				if(nCodeBitCount >= 8)
				{
					pOut[nCodeByteCount++] = nCodeByte;		//切换字节
					nCodeBitCount = 0;						//编码位个数清零				
				}
			}
		}
	}

	//处理最后一个字节8位未满的情况
	if(nCodeBitCount != 0)
	{
		//将来已经编码的位移到头部
		nCodeByte = nCodeByte << (8 - nCodeBitCount);

		//加入到输出字节
		pOut[nCodeByteCount++] = nCodeByte;
	}
	

	return nCodeByteCount;
}

//反向不归零解码
uint8 NrziDecode0(uint8* pIn, uint8 nIn, uint8* pOut)
{
	uint8 i, nSrcBitCount, nDecodeBitCount, nUnchangeCount;
	uint8 nSrcByte;
	uint8 nDecodeByte;
	uint8 nDecodePerBit,nDecodeCurBit, nDecodeBit;
	uint8 nDecodeByteCount;

	nDecodePerBit = 1;				//一开始默认为高电平
	nDecodeBit = 1;					//解码数据初始化为1
	nDecodeBitCount = 0;			//已经编码位的个数
	nDecodeByteCount = 0;			//已经编码字节的个数
	nUnchangeCount = 0;				//连续1的个数
	nDecodeByte = 0;				//解码后的字节

	//循环编码
	for(i = 0; i < nIn; i++)
	{
		//获取源字节
		nSrcByte = pIn[i];

		for(nSrcBitCount = 0; nSrcBitCount < 8; nSrcBitCount++)
		{
			//处理源数据得到当前位(从高位出)========================================
			nDecodeCurBit = ((nSrcByte & 0x80) == 0) ? 0 : 1;	//获取当前数据位
			nSrcByte <<= 1;										//源数据左移一位

			//解码该位==============================================================
			//发生了跳变
			if(nDecodeCurBit != nDecodePerBit)
			{				
				//如果连续相同位大于等于NRZI_UNCHANGE_MAX，跳过此位
				if(nUnchangeCount >= NRZI_UNCHANGE_MAX)
				{
					nUnchangeCount = 0;
					nDecodePerBit = nDecodeCurBit;
					continue;
				}
				
				nUnchangeCount = 0;						//电平不变次数归零
				nDecodePerBit = nDecodeCurBit;			//记录前一位
				
				//解码位为0
				nDecodeBit = 0;
			}
			
			//没发生跳变
			else										//当前源数据位为1
			{
				nUnchangeCount++;						//电平不变次数增加
				nDecodePerBit = nDecodeCurBit;			//记录前一位

				//解码位为1
				nDecodeBit = 1;	
			}

			//形成目标数据(从低位入)================================================
			nDecodeByte <<= 1;								//目标数据给准备一位
			if(nDecodeBit == 0) nDecodeByte &= ~0x01;		//置零位
			else			 nDecodeByte |= 0x01;	
			nDecodeBitCount++;

			//如果已经满一个字节，切换到下一字节
			if(nDecodeBitCount >= 8)
			{
				pOut[nDecodeByteCount++] = nDecodeByte;		//切换字节
				nDecodeBitCount = 0;						//编码位个数清零				
			}

		}
	}

	//处理最后一个字节8位未满的情况
	//if(nDecodeBitCount != 0)
	//{
	//	//空余字节，丢弃
	//}
	
	return nDecodeByteCount;
}