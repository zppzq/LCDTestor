#ifndef __LOWPOWER_H__
#define __LOWPOWER_H__

//宏定义--------------------------------------------------------------------------
#define	RTCCLK         ((long)32768)		//实时时钟频率(Hz)

#define	SUSPEND			0x40				//进入SUSPEND模式时写入PMU0CF的值
#define	SLEEP			0x80				//进入SLEEP模式时写入PMU0CF的值

//编译开关------------------------------------------------------------------------
#define	_MCU_ENTER_SLEEP_


void CLKConfig_Enable(void);
void RTC_Alarm_IntConfig(void);
void RTC_Configuration(void);
void MCUSleep(void) reentrant;
void MCUEnterSleep(void) reentrant;
void MCUExitSleep(void) reentrant;




#endif //__LOWPOWER_H__



