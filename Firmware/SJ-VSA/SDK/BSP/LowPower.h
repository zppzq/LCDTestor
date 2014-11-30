#ifndef __LOWPOWER_H__
#define __LOWPOWER_H__

//�궨��--------------------------------------------------------------------------
#define	RTCCLK         ((long)32768)		//ʵʱʱ��Ƶ��(Hz)

#define	SUSPEND			0x40				//����SUSPENDģʽʱд��PMU0CF��ֵ
#define	SLEEP			0x80				//����SLEEPģʽʱд��PMU0CF��ֵ

//���뿪��------------------------------------------------------------------------
#define	_MCU_ENTER_SLEEP_


void CLKConfig_Enable(void);
void RTC_Alarm_IntConfig(void);
void RTC_Configuration(void);
void MCUSleep(void) reentrant;
void MCUEnterSleep(void) reentrant;
void MCUExitSleep(void) reentrant;




#endif //__LOWPOWER_H__



