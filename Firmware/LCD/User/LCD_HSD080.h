#ifndef 	_LCD_H_
#define 	_LCD_H_

#ifdef		_LCD_C_
#define		LCD_EXT
#else
#define		LCD_EXT		extern
#endif
//****************************************************************************************************************
void LCDInit(void) reentrant;
void PostLCD(uint16 nTicksSpan) reentrant;
void LCDProcess(void) reentrant;
void LCDDisplayRed(void);
//****************************************************************************************************************
#endif
