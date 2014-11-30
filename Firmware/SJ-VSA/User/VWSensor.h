/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------�� �� �� Ϣ--------------------------------------------------------**
**��   ��   ���� VWSensor.h												 **
**��   ��   �ˣ� ��С��														 **
**�� ��  ʱ �䣺 2008-09-28													 **
**����޸�ʱ�䣺													 		 **
**��        ���� 														 	 **
******************************************************************************/

#ifndef 	_VW_SENSOR_H_
#define 	_VW_SENSOR_H_

#ifdef		_VW_SENSOR_C_
#define		VW_SENSOR_EXT
#else
#define		VW_SENSOR_EXT		extern
#endif

// -------- ������� ----------------------------------------------------------
#define _VW_SENSOR_INIT_
#define _VW_SENSOR_GET_VALUE_


#define VWS_MIN_FREQ				400.00
#define VWS_MAX_FREQ				6000.00

#define VWS_SYNT_MAXTIMES			25			// г���������������û��������ǿ���˳�
#define VWS_COLLECT_COUNT			20			// Ҫʰȡ���Ҳ��ĸ���

#define FREQ_VALID_LIMIT			50			// ɸѡƵ��ʱ��Ƶ�ʷ�Χ
//#define FREQ_OVER_VALUE_LIMIT		0.08		// ����Ƶ����Ч�ķ�Χ
#define FREQ_STEP_INVALID			50			// ����Ч��ƽʱ��Ƶ�ʲ���
#define FREQ_STEP_NO_SUCCEED		50			// �������50�β��Ե�Ƶ�ʲ���
#define FREQ_STEP_LESS_TIMES		50			// ���ܴﵽ��Ч������Ƶ�ʲ���


VW_SENSOR_EXT void		VWSWakenSetFreq(float fFreq) reentrant;
VW_SENSOR_EXT void 		VWSWakenStart() reentrant;
VW_SENSOR_EXT void 		VWSWakenStop() reentrant;
VW_SENSOR_EXT void 		VWSensorInit() reentrant;
VW_SENSOR_EXT void 		VWSCollectStart() reentrant;
VW_SENSOR_EXT void 		VWSCollectStop() reentrant;
VW_SENSOR_EXT int16		VWSGetValue() reentrant;
VW_SENSOR_EXT float 	VWSGetFreq() reentrant;
VW_SENSOR_EXT void 		VWSDelay(unsigned long nDly) reentrant;
VW_SENSOR_EXT void 		VWSetParam(fp32* pParam) reentrant;
VW_SENSOR_EXT void 		VWSSetZero() reentrant;

#endif