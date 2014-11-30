/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: Nrzi.c
**��   ��   ��: ��п�
**�� �� ��  ��: 2009��7��7��
**����޸�����: 
**��        ��: ���򲻹������Դ�ļ�
*****************************************************************************************************************/
#include "..\Global.h"

//���������
#define NRZI_UNCHANGE_MAX	6

//���򲻹������

uint8 NrziEncode0(uint8* pIn, uint8 nIn, uint8* pOut)
{
	uint8 i, nSrcBitCount, nCodeBitCount, nUnchangeCount;
	uint8 nSrcByte;
	uint8 nCodeByte;
	uint8 nCodeCurBit;
	uint8 nCodeByteCount;

	nCodeCurBit = 1;			//һ��ʼĬ��Ϊ�ߵ�ƽ
	nCodeBitCount = 0;			//�Ѿ�����λ�ĸ���
	nCodeByteCount = 0;			//�Ѿ������ֽڵĸ���
	nUnchangeCount = 0;			//����1�ĸ���
	nCodeByte = 0;				//�������ֽ�

	//ѭ������
	for(i = 0; i < nIn; i++)
	{
		//��ȡԴ�ֽ�
		nSrcByte = pIn[i];

		for(nSrcBitCount = 0; nSrcBitCount < 8; nSrcBitCount++)
		{
			//����Դ���ݵõ���ǰλ(�Ӹ�λ��)========================================
			//��ǰԴ����λΪ0
			if((nSrcByte & 0x80) == 0)					
			{
				nCodeCurBit = !nCodeCurBit;				//����
				nUnchangeCount = 0;						//��ƽ�����������	
			}
			
			//��ǰԴ����λΪ1 
			else										//��ǰԴ����λΪ1
			{
				nUnchangeCount++;						//��ƽ�����������
			}

			//Դ��������һλ
			nSrcByte <<= 1;

			//�γ�Ŀ������(�ӵ�λ��)================================================
			nCodeByte <<= 1;							//Ŀ�����ݸ�׼��һλ
			if(nCodeCurBit == 0) nCodeByte &= ~0x01;	//����λ
			else			 nCodeByte |= 0x01;	
			nCodeBitCount++;

			//����Ѿ���һ���ֽڣ��л�����һ�ֽ�
			if(nCodeBitCount >= 8)
			{
				pOut[nCodeByteCount++] = nCodeByte;		//�л��ֽ�
				nCodeBitCount = 0;						//����λ��������				
			}

			//�������1�ĸ����������ƣ�������
			if(nUnchangeCount >= NRZI_UNCHANGE_MAX)
			{
				nCodeCurBit = !nCodeCurBit;					//����(�������)
				nUnchangeCount = 0;							//����1��������

				//�γ�Ŀ������
				nCodeByte <<= 1;							//Ŀ�����ݸ�׼��һλ
				if(nCodeCurBit == 0) nCodeByte &= ~0x01;	//����λ
				else			 nCodeByte |= 0x01;	
				nCodeBitCount++;

				//����Ѿ���һ���ֽڣ��л�����һ�ֽ�
				if(nCodeBitCount >= 8)
				{
					pOut[nCodeByteCount++] = nCodeByte;		//�л��ֽ�
					nCodeBitCount = 0;						//����λ��������				
				}
			}
		}
	}

	//�������һ���ֽ�8λδ�������
	if(nCodeBitCount != 0)
	{
		//�����Ѿ������λ�Ƶ�ͷ��
		nCodeByte = nCodeByte << (8 - nCodeBitCount);

		//���뵽����ֽ�
		pOut[nCodeByteCount++] = nCodeByte;
	}
	

	return nCodeByteCount;
}

//���򲻹������
uint8 NrziDecode0(uint8* pIn, uint8 nIn, uint8* pOut)
{
	uint8 i, nSrcBitCount, nDecodeBitCount, nUnchangeCount;
	uint8 nSrcByte;
	uint8 nDecodeByte;
	uint8 nDecodePerBit,nDecodeCurBit, nDecodeBit;
	uint8 nDecodeByteCount;

	nDecodePerBit = 1;				//һ��ʼĬ��Ϊ�ߵ�ƽ
	nDecodeBit = 1;					//�������ݳ�ʼ��Ϊ1
	nDecodeBitCount = 0;			//�Ѿ�����λ�ĸ���
	nDecodeByteCount = 0;			//�Ѿ������ֽڵĸ���
	nUnchangeCount = 0;				//����1�ĸ���
	nDecodeByte = 0;				//�������ֽ�

	//ѭ������
	for(i = 0; i < nIn; i++)
	{
		//��ȡԴ�ֽ�
		nSrcByte = pIn[i];

		for(nSrcBitCount = 0; nSrcBitCount < 8; nSrcBitCount++)
		{
			//����Դ���ݵõ���ǰλ(�Ӹ�λ��)========================================
			nDecodeCurBit = ((nSrcByte & 0x80) == 0) ? 0 : 1;	//��ȡ��ǰ����λ
			nSrcByte <<= 1;										//Դ��������һλ

			//�����λ==============================================================
			//����������
			if(nDecodeCurBit != nDecodePerBit)
			{				
				//���������ͬλ���ڵ���NRZI_UNCHANGE_MAX��������λ
				if(nUnchangeCount >= NRZI_UNCHANGE_MAX)
				{
					nUnchangeCount = 0;
					nDecodePerBit = nDecodeCurBit;
					continue;
				}
				
				nUnchangeCount = 0;						//��ƽ�����������
				nDecodePerBit = nDecodeCurBit;			//��¼ǰһλ
				
				//����λΪ0
				nDecodeBit = 0;
			}
			
			//û��������
			else										//��ǰԴ����λΪ1
			{
				nUnchangeCount++;						//��ƽ�����������
				nDecodePerBit = nDecodeCurBit;			//��¼ǰһλ

				//����λΪ1
				nDecodeBit = 1;	
			}

			//�γ�Ŀ������(�ӵ�λ��)================================================
			nDecodeByte <<= 1;								//Ŀ�����ݸ�׼��һλ
			if(nDecodeBit == 0) nDecodeByte &= ~0x01;		//����λ
			else			 nDecodeByte |= 0x01;	
			nDecodeBitCount++;

			//����Ѿ���һ���ֽڣ��л�����һ�ֽ�
			if(nDecodeBitCount >= 8)
			{
				pOut[nDecodeByteCount++] = nDecodeByte;		//�л��ֽ�
				nDecodeBitCount = 0;						//����λ��������				
			}

		}
	}

	//�������һ���ֽ�8λδ�������
	//if(nDecodeBitCount != 0)
	//{
	//	//�����ֽڣ�����
	//}
	
	return nDecodeByteCount;
}