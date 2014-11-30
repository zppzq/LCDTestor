#include "includes.h"
#include    <ucos_ii.h>
//#include    <app_cfg.h>

#define  TASK_STK_SIZE                  128      /* Size of each task's stacks (# of WORDs)            */
OS_STK        TaskStartStk[TASK_STK_SIZE];
OS_STK        Task1Stk[TASK_STK_SIZE];
OS_STK        Task2Stk[TASK_STK_SIZE];

void  Task1 (void *pdata) 
{
	pdata = pdata;                                         /* Prevent compiler warning                 */
	while(1)
	{
		OSTimeDly(10);
		pdata = pdata;                                         /* Prevent compiler warning                 */
	}
}
void  Task2Display (void *pdata) 
{
	pdata = pdata;                                         /* Prevent compiler warning                 */
	while(1)
	{
		OSTimeDly(10);
		pdata = pdata;                                         /* Prevent compiler warning                 */
	}
}
void  TaskStart (void *pdata)
{
   pdata = pdata;                                         /* Prevent compiler warning                 */
   
   BSP_Init();  
   OSTaskCreate(Task1, (void *)0, &Task1Stk[TASK_STK_SIZE - 1], 1);
   OSTaskCreate(Task2Display, (void *)0, &Task2Stk[TASK_STK_SIZE - 1], 2);
   OSTaskDel(0);      
}
/***********************************************************
 MIAN ENTRY                                               
 ***********************************************************/
int main (void) 
{
//	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
//	SysTick_CounterCmd(SysTick_Counter_Enable);
//	SysTick_ITConfig(ENABLE);
//	NVIC_InitTypeDef NVIC_InitStructure;
    SystemInit();
/*	SysTick_Config(SystemFrequency);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);	*/

    OSInit();    
    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
  for(;;) {
  	}
}

