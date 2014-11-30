/****************************************************************************
* Copyright (C), 2010 ���������� www.armfly.com
*
* �ļ���: usart_printf.c
* ���ݼ���: ��ģ��ʵ��printf��scanf�����ض��򵽴���1
*	ʵ���ض���ֻ��Ҫ���2������
		int fputc(int ch, FILE *f);
		int fgetc(FILE *f);
*
* �ļ���ʷ:
* �汾��  ����       ����    ˵��
* v0.1    2009-12-27 armfly  �������ļ�
*
*/

/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/*******************************************************************************
	��������PrintfLogo
	��  ��: �������ƺ���������������
	��  ��:
	����˵����
*/
void PrintfLogo(char *strName, char *strDate)
{
	printf("*************************************************************\r\n");
	printf("* Example Name : %s\r\n", strName);
	printf("* Update Date  : %s\r\n", strDate);
	printf("* StdPeriph_Lib Version : V3.1.2\r\n");
	printf("* \r\n");
	printf("* Copyright www.armfly.com \r\n");
	printf("* QQ    : 1295744630 \r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("*************************************************************\r\n");
}


/*******************************************************************************
	��������fputc
	��  ��:
	��  ��:
	����˵����
	�ض���putc��������������ʹ��printf�����Ӵ���1��ӡ���
*/
int fputc(int ch, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(USART1, (uint8_t) ch);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}

/*******************************************************************************
	��������fputc
	��  ��:
	��  ��:
	����˵����
	�ض���getc��������������ʹ��scanff�����Ӵ���1��������
*/
int fgetc(FILE *f)
{
	/* �ȴ�����1�������� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
	{}

	return (int)USART_ReceiveData(USART1);
}

