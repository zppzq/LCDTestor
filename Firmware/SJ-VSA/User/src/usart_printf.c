/****************************************************************************
* Copyright (C), 2010 安富莱电子 www.armfly.com
*
* 文件名: usart_printf.c
* 内容简述: 本模块实现printf和scanf函数重定向到串口1
*	实现重定向，只需要添加2个函数
		int fputc(int ch, FILE *f);
		int fgetc(FILE *f);
*
* 文件历史:
* 版本号  日期       作者    说明
* v0.1    2009-12-27 armfly  创建该文件
*
*/

/* Includes ------------------------------------------------------------------*/
#include "includes.h"

/*******************************************************************************
	函数名：PrintfLogo
	输  入: 例程名称和例程最后更新日期
	输  出:
	功能说明：
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
	函数名：fputc
	输  入:
	输  出:
	功能说明：
	重定义putc函数，这样可以使用printf函数从串口1打印输出
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
	函数名：fputc
	输  入:
	输  出:
	功能说明：
	重定义getc函数，这样可以使用scanff函数从串口1输入数据
*/
int fgetc(FILE *f)
{
	/* 等待串口1输入数据 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
	{}

	return (int)USART_ReceiveData(USART1);
}

