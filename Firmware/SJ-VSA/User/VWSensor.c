/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------�� �� �� Ϣ--------------------------------------------------------**
**��   ��   ���� VWSensor.c												 	 **
**��   ��   �ˣ� ��С��														 **
**�� ��  ʱ �䣺 2008-09-28													 **
**����޸�ʱ�䣺													 		 **
**��        ���� 														 	 **
******************************************************************************/

#define	_VWSENSOR_C_

//--------Includes-------------------------------------------------------------
#include "..\sdk\bsp\bsp.h"
#include "VWSensor.h"

//#define VWS_DEBUG		1

//Ƶ�ʲ���
static uint16 nFreqValidCount;					//����Ƶ������Ϊ��Ч��Ƶ�ʸ���
static int32 nLocalTemp;						//������ʱ����
static fp32 nFreqRecord = 400.0f; 				//��¼��һ�ε�г��ֵ
static fp32 fFreqAver;							//��ֵ
static fp32 fFreqAverValid; 					//��Ч�ľ�ֵ
static fp32 fFreqWaken;							//����Ƶ��
static fp32 fFreqWakenStep;						//��������
static uint16 nWakenIndex;						//����ķ�����������
static uint16 nWakenMax;						//����������
static float fFreqValidCheck;					//��Ч��ⷶΧ
static uint16 nSyntTimes;

//�ź���
static OS_EVENT 	*pVWSEvent;					//�¼����ƿ�
static uint8 		nVWSErr;					//�����־

static OS_EVENT 	*pTimer0Event;				//�¼����ƿ�
static uint8 		nTimer0Err;					//�����־


static fp32			VWParamG = 3.7f;
static fp32			VWParamC = 1.027459f;

static fp32			VWf0;
static fp32			VWfx;
static int16		VWPress;

//��ƽ��Ч�������
#define OUT_EN_PORT		PORT(0)
#define OUT_EN			BIT(4)

//���ŵ�����
#define FLOW_OPEN_PORT	PORT(0)
#define FLOW_OPEN		BIT(5)

//�����������
#define VWS_WAKEN_PORT	PORT(0)
#define VWS_WAKEN		BIT(3)

/******************************************************************************
**��	�ƣ� VWSensorInit()
**��	�ܣ� ʰ���ʼ��
**��ڲ����� ��
**���ڲ����� ��
**˵	���� ��
******************************************************************************/
#ifdef _VW_SENSOR_INIT_
void Timer0Init() reentrant
{
	//ʱ��ѡ��
	CKCON &= 0xF8;		
	
	//ģʽѡ��(16λ��ʱ��)
	TMOD &= 0xF0;
	TMOD |= 0x01;


	ET0 = 0;
	TR0 = 0;

	pTimer0Event = OSSemCreate(0);
}



void VWSensorInit() reentrant
{
	//��ƽ��Ч���
	MakeOpenDrain(OUT_EN);				// ��������

	//��ŵ���������
	MakePushPull(FLOW_OPEN);
	SetLo(FLOW_OPEN);

	//������������
	MakePushPull(VWS_WAKEN);
	SetLo(VWS_WAKEN);

	//�ź�����ʼ��
	pVWSEvent = OSSemCreate(0);			//�����ź���
	Timer0Init();						//��ʱ��0��ʼ��	
}
#endif

//���ò���
void VWSetParam(fp32* pParam) reentrant
{
	VWParamG = pParam[0];
	VWParamC = pParam[1];
}

//��ʼ������
//nMilisecond: 0-600
void SetTimer0(uint16 nMilisecond) reentrant
{
	uint32 nTimeSet;
	uint16 nTimeSet16;

	nMilisecond = (nMilisecond > 600) ? 600 : nMilisecond;

	nTimeSet = SYSCLK / 1000;
 	nTimeSet *= nMilisecond;
	nTimeSet /= 480;

	nTimeSet16 = nTimeSet;
	nTimeSet16 = -nTimeSet16;

	TH0 = HIBYTE(nTimeSet16);
	TL0 = LOBYTE(nTimeSet16);

	//ʹ���жϣ�����
	ET0 = 1;
	TR0 = 1;

	//�ȴ���ʱ���
	OSSemPend(pTimer0Event, 0, &nTimer0Err);
}

void Timer0ISR() interrupt 1
{
	//���ж�
	OSIntEnter();

	//��ֹ�жϣ�����
	ET0 = 0;
	TR0 = 0;
	
	//�����ź���
	OSSemPost(pTimer0Event);

	//���ж�
	OSIntExit();

}

/******************************************************************************
**��	�ƣ� VWSWakenSetFreq()
**��	�ܣ� ����Ƶ��
**��ڲ����� ��Ҫ���õ�Ƶ��
**���ڲ����� ��
**˵	���� ���Ƶ�ʣ�nPCA0CLK/2����Сʱ�ӣ�nPCA0CLK/2/65536
******************************************************************************/
void VWSWakenSetFreq(float fFreq) reentrant
{
	//������Χ
    fFreq = (fFreq < 400.0f) ? 400.0f : fFreq;
	fFreq = (fFreq > 6000.0f) ? 6000.0f : fFreq;

	//���������
	nFreq16Span = (unsigned int)(nPCA0CLK / fFreq / 2);
}

/******************************************************************************
**��	�ƣ� VWSWakenStart()
**��	�ܣ� ��ʼ����
**��ڲ����� ��
**���ڲ����� ��
**˵	���� ��
******************************************************************************/
void VWSWakenStart() reentrant
{
	if(nTestStatus == TEST_FREQ) nWakenMax = 500; 
	else nWakenMax = 300;
	nWakenIndex = 0;
	XBR1         |= 0x42;					// ʹ��CEX0��CEX1���˿�����	
	nIntType      = INT_TYPE_WAKEN;			// ָ���ж�����
	nSensorStatus = SENSOR_WAKEN;			// ���뼤��״̬
	EnableCCF1Int();						// PCA0CPM1 |= 0x01;						// ʹ��CEX1�ж�
	EnablePCA0();							// PCA0CN 	|= 0x40;						// ʹ��PCA0
}

/******************************************************************************
**��	�ƣ� VWSWakenWait()
**��	�ܣ� �ȴ��������
**��ڲ����� ��
**���ڲ����� ��
**˵	���� ��
******************************************************************************/
void VWSWakenWait() reentrant
{
	OSSemPend(pVWSEvent, 0, &nVWSErr);
}


/******************************************************************************
**��	�ƣ� VWSCollectWait()
**��	�ܣ� �ȴ�ʰ�����
**��ڲ����� ��
**���ڲ����� ��
**˵	���� ��
******************************************************************************/
BOOL VWSCollectWait() reentrant
{
	OSSemPend(pVWSEvent, 6, &nVWSErr);

	if(nVWSErr == OS_NO_ERR) 
	{
		if(nCollectOK == 1) return TRUE;
	}

	return FALSE;
}



/******************************************************************************
**��	�ƣ� VWSWakenStop()
**��	�ܣ� ֹͣʰ��
**��ڲ����� ��
**���ڲ����� ��
**˵	���� ��
******************************************************************************/
void VWSWakenStop() reentrant
{
	DisableCCF1Int();						// PCA0CPM1 &= ~0x01;						// ��ֹCEX1�ж�
	XBR1 = 0x41;							// ��ֹCEX1
	SetLo(VWS_WAKEN);						// ��������0����
	DisablePCA0();							// PCA0CN &= ~0x40;	// ��ֹPCA0
	nSensorStatus = SENSOR_IDLE;			// �������״̬
}


/******************************************************************************
**��	�ƣ� Int1ISR()
**��	�ܣ� ʰ���ƽ����ж�
**��ڲ����� ��
**���ڲ����� ��
**˵	���� ������жϱ�ʾ���Խ���ʰ�񣬵�ƽ�ﵽ��һ��ǿ��
******************************************************************************/
void Int1ISR() interrupt 2
{
	nIntType      = INT_TYPE_COLLECT;	// ָ���ж�����	
	nSensorStatus = SENSOR_COLLECT;		// ����ʰ��״̬
	nCollectIntCount  = 0;				// �жϴ�������
	nCollectOverCount = 0;				// PCA0�����������
	EnableCCF0Int();					// PCA0CPM0 |= 0x01;	// ��CEX0�ж�
	EnableCFInt();						// ʹ��PCA0����ж�
	EnablePCA0();						// ʹ��PCA0������
	EX1 = 0;							// ��ֹ�ⲿ�ж�1
}


/******************************************************************************
**��	�ƣ� VWSCollectStart()
**��	�ܣ� ��ʼʰ��
**��ڲ����� ��
**���ڲ����� ��
**˵	���� ��
******************************************************************************/
void VWSCollectStart() reentrant
{
 	// ���������Ƶ�ʣ���Ҫ���ϴ˶�
	nIntType      = INT_TYPE_COLLECT;	// ָ���ж�����	
	nSensorStatus = SENSOR_COLLECT;		// ����ʰ��״̬
	nCollectIntCount  = 0;				// �жϴ�������
	nCollectOverCount = 0;				// PCA0�����������
	PCA0CN &= 0x40;						// ����жϱ�־
	EnableCCF0Int();					// PCA0CPM0 |= 0x01;	// ��CEX0�ж�
	EnableCFInt();						// ��PCA0����ж�
	EnablePCA0();						// ʹ��PCA0������
}

/******************************************************************************
**��	�ƣ� VWSCollectStop()
**��	�ܣ� ֹͣʰ��
**��ڲ����� ��
**���ڲ����� ��
**˵	���� ��
******************************************************************************/
void VWSCollectStop() reentrant
{
	DisablePCA0();							// ��ֹPCA0������
	DisableCFInt();							// ��ֹPCA0����ж�
	DisableCCF0Int();						// ��ֹCEX0�ж�
	EX1 = 0;								// ��ֹ�ⲿ�ж�1
	nSensorStatus = SENSOR_IDLE;			// ����ʰ��״̬
}

/******************************************************************************
**��	�ƣ� VWSGetFreq()
**��	�ܣ� ��ȡʰ���Ƶ��
**��ڲ����� ��
**���ڲ����� Ƶ��
**˵	���� ��Ƶ����ʵ���
******************************************************************************/
static BOOL TryLastFreg() reentrant
{
	//��ʱ����
	uint16 i;

	//����
	VWSWakenSetFreq(nFreqRecord);		// ���ü���Ƶ��
	VWSWakenStart();					// ��ʼ����
	VWSWakenWait();						// �ȴ��������
	VWSWakenStop();						// �رռ���		

	//�ŵ�
	SetHi(FLOW_OPEN);
	SetTimer0(50);

    //���
	SetLo(FLOW_OPEN);
	SetTimer0(300);
	
	
	//ʰ��ʼ���ȴ���ֹͣ	
	VWSCollectStart();
	VWSCollectWait();
	VWSCollectStop();
	
	//���ʰ����Ч
	if(nCollectOK == 1) 
	{
		//����Ƶ��ֵ
		nFreqValidCount = 0;
		fFreqAver = 0;
		fFreqAverValid = 0;
		for(i=0; i<VWS_COLLECT_COUNT; i++)
		{
			fFreq[i]  = (float)nPCA0CLK;
			fFreq[i] /=	(float)((nCollectOverV[i+1] - nCollectOverV[i])* 65536 + nCollectValue[i+1] - nCollectValue[i]);
			fFreqAver += fFreq[i];
		}
		fFreqAver /= VWS_COLLECT_COUNT;

		// ��Ƶ�ʽ���ɸѡ
		for(i=0; i<VWS_COLLECT_COUNT; i++)
		{
			if((fFreq[i] > fFreqAver - FREQ_VALID_LIMIT) && (fFreq[i] < fFreqAver + FREQ_VALID_LIMIT))
			{
				fFreq[nFreqValidCount] = fFreq[i];				// ������֯��ЧƵ��
				fFreqAverValid += fFreq[nFreqValidCount];		// ��¼��ЧƵ��
				nFreqValidCount++;								// ��ЧƵ�ʸ�����1
			}
		}
					
		// ��ƽ��ֵ
		if(nFreqValidCount > 0)
		{
			//�����ЧƵ�ʸ�������40����ƽ��Ƶ���ڼ���Ƶ������0.1��Χ��������ȷ�����Խ���
			if(nFreqValidCount >= VWS_COLLECT_COUNT)
			{
				// �����巽ʽ����ƽ��Ƶ��
				nLocalTemp = nCollectOverV[VWS_COLLECT_COUNT] - nCollectOverV[0];
				nLocalTemp = nLocalTemp<<16;
				nLocalTemp += nCollectValue[VWS_COLLECT_COUNT];
				nLocalTemp -= nCollectValue[0];

				fFreqAver = (float)nPCA0CLK;							
				fFreqAver *= VWS_COLLECT_COUNT;
				fFreqAver /= nLocalTemp;

				//�µļ���ֵ
				nFreqRecord = fFreqAver;

				return TRUE;
			}
		}
	}

	//���ؽ��
	return FALSE;

}

/******************************************************************************
**��	�ƣ� VWSSetZero()
**��	�ܣ� ����
**��ڲ����� ��
**���ڲ����� ��
**˵	���� 
******************************************************************************/
void VWSSetZero() reentrant
{
	do
	{
		VWf0 = VWSGetFreq();
	}while(VWf0 < 200.0f);
	
	
	VWf0 *= VWf0;

	//��ʹ�ñ��ص��㣬����Ĭ�ϲο���
	VWf0 = 360000.0f;
}

/******************************************************************************
**��	�ƣ� VWSGetValue()
**��	�ܣ� ��ȡӦ��ֵ
**��ڲ����� ��
**���ڲ����� Ƶ��
**˵	���� ��Ƶ����ʵ���
******************************************************************************/
int16 VWSGetValue() reentrant
{
	do
	{
		VWfx = VWSGetFreq();
	}while(VWfx < 200.0f);
	
	
	VWfx *= VWfx;
	VWfx = VWfx - VWf0;
	VWfx = VWfx / 1000.0;
	VWfx = VWfx * VWParamG * VWParamC;
	
	VWPress = (int16)VWfx;

	return VWPress;
}

/******************************************************************************
**��	�ƣ� VWSGetFreq()
**��	�ܣ� ��ȡʰ���Ƶ��
**��ڲ����� ��
**���ڲ����� Ƶ��
**˵	���� ��Ƶ����ʵ���
******************************************************************************/
//#define VW_NO_SENSOR_DEBUG		1
float VWSGetFreq() reentrant
{
	uint16 i;
	BOOL bFreqFind = 0;				   	// �Ƿ��ҵ�Ƶ��
	nSyntTimes = 0;						// г�����

#ifdef VW_NO_SENSOR_DEBUG
	OSTimeDly(OS_TICKS_PER_SEC * 2);
	return 800.0; Error Set
#endif

	//������һ�β���ֵ	
 	//if(TRUE)
	if(TryLastFreg() == FALSE)
	{
		nTestStatus = SCAN_FREQ;		// PCA0����ʽѡ�񣬹�ϵ�����񷽲�����
	
		//�ŵ�
		SetHi(FLOW_OPEN);
		SetTimer0(200);
		//SetLo(FLOW_OPEN);
	
		//�ֵ�
		fFreqWaken = 400.0f;		    // �����ֵ
		fFreqWakenStep = 50.0;			// ���񲽽�
		while(fFreqWaken < 4000.0f)
		{
			//����	
			VWSWakenSetFreq(fFreqWaken);		// �����ʼ�ļ���Ƶ��
			VWSWakenStart();					// �Դ�Ƶ�ʼ���
			VWSWakenWait();						// �ȴ��������
			VWSWakenStop();						// �رռ���
			
			//�ŵ�
			SetHi(FLOW_OPEN);
			SetTimer0(200);

#ifdef VWS_DEBUG	
			if(GetSignal(OUT_EN) == TRUE)			// �Ƿ�����
			{
				_nop_(); 
			}
#endif
	
		    //���
			SetLo(FLOW_OPEN);
			SetTimer0(300);
	
			//�����Ч��ƽ
			if(GetSignal(OUT_EN) == FALSE)			// �Ƿ��õ�
			{
				_nop_();
				break;
			}
			
			//������һ��ɨ��
			fFreqWaken += fFreqWakenStep;
		}

	}
	else
	{
		fFreqWaken = nFreqRecord;
	}
	

	//ϸ��
	nTestStatus = TEST_FREQ;			  	// ����ʽѡ�񣬹�ϵ�����񷽲�����
	
	// -------- ��ͨ������ɨƵ ------------------------------------------------
	while(fFreqWaken <= VWS_MAX_FREQ)
	{
		//����
		VWSWakenSetFreq(fFreqWaken);		// ���ü���Ƶ��
		VWSWakenStart();
		VWSWakenWait();						// �ȴ��������
		VWSWakenStop();						// �رռ���		

		//�ŵ�
		SetHi(FLOW_OPEN);
		SetTimer0(50);

	    //���
		SetLo(FLOW_OPEN);
		SetTimer0(300);
		
		
		//ʰ��ʼ���ȴ���ֹͣ	
		VWSCollectStart();
		VWSCollectWait();
		VWSCollectStop();
		
		//���ʰ����Ч
		if(nCollectOK == 1) 
		{
			//����Ƶ��ֵ
			nFreqValidCount = 0;
			fFreqAver = 0;
			fFreqAverValid = 0;
			for(i=0; i<VWS_COLLECT_COUNT; i++)
			{
				fFreq[i]  = (float)nPCA0CLK /(float)((nCollectOverV[i+1] - nCollectOverV[i])* 65536
							 + nCollectValue[i+1] - nCollectValue[i]);
				fFreqAver += fFreq[i];
			}
			fFreqAver /= VWS_COLLECT_COUNT;

			// ��Ƶ�ʽ���ɸѡ
			for(i=0; i<VWS_COLLECT_COUNT; i++)
			{
				if((fFreq[i] > fFreqAver - FREQ_VALID_LIMIT) && (fFreq[i] < fFreqAver + FREQ_VALID_LIMIT))
				{
					fFreq[nFreqValidCount] = fFreq[i];				// ������֯��ЧƵ��
					fFreqAverValid += fFreq[nFreqValidCount];		// ��¼��ЧƵ��
					nFreqValidCount++;								// ��ЧƵ�ʸ�����1
				}
			}
						
			// ��ƽ��ֵ
			if(nFreqValidCount > 0)
			{
				fFreqAver = fFreqAverValid / nFreqValidCount;			// ��ЧƵ�ʵ�ƽ��ֵ
				fFreqAverValid = fFreqAver;		

				// �����ЧƵ�ʸ�������40����ƽ��Ƶ���ڼ���Ƶ������0.1��Χ��������ȷ�����Խ���
				if(nFreqValidCount >= VWS_COLLECT_COUNT)
				{
					// �����巽ʽ����ƽ��Ƶ��
					nLocalTemp = nCollectOverV[VWS_COLLECT_COUNT] - nCollectOverV[0];
					nLocalTemp = nLocalTemp<<16;
					nLocalTemp += nCollectValue[VWS_COLLECT_COUNT];
					nLocalTemp -= nCollectValue[0];

					fFreqAver = (float)nPCA0CLK;							
					fFreqAver *= VWS_COLLECT_COUNT;
					fFreqAver /= nLocalTemp;

					if(fFreqAver > 1000.0f) fFreqValidCheck = 0.08f;
					if(fFreqAver < 600.0f) fFreqValidCheck = 0.2f;
					else fFreqValidCheck = 0.15;
					
					if((fFreqAver < fFreqWaken + fFreqValidCheck) && (fFreqAver > fFreqWaken - fFreqValidCheck))
					{
						bFreqFind = 1;					// ��ʶ�ҵ�Ƶ��
						nFreqRecord = fFreqAver;		// ��¼Ƶ��
						SetHi(FLOW_OPEN);				// �ŵ�
						break;
					}
					else								// ��õ�Ƶ�ʳ����涨��Χ
					{
						// ������Ƶ�ʻ��ɲ⵽����ЧƵ��
						fFreqWaken = fFreqAver;
						
						//��ֹ����
						nSyntTimes++;
						if(nSyntTimes >= VWS_SYNT_MAXTIMES)
						{
							bFreqFind = 1;					// ��ʶ�ҵ�Ƶ��
							nFreqRecord = fFreqAver;		// ��¼Ƶ��
							SetHi(FLOW_OPEN);				// �ŵ�
							break;
						}
					}
				}
				else										// ��ЧƵ�ʴ�������
				{
					fFreqWaken += FREQ_STEP_LESS_TIMES;
				}
			}
			nCollectOK = 0;
		}
		else												// û���㹻��PCA0�жϴ���
		{
			fFreqWaken += FREQ_STEP_NO_SUCCEED;	
		}
	}

	// ѭ������Ƶ�ʽ���	
	if(bFreqFind == 0) fFreqAver = 0;						// û�ҵ�Ƶ�ʣ�������ЧƵ��

	//���ؽ��
	return fFreqAver;
}

/******************************************************************************
**��	�ƣ� PCA0Int()
**��	�ܣ� PCA�ж���Ӧ����
**��ڲ����� ��
**���ڲ����� ��
**˵	���� �ж��жϵ����ͣ�����ͬ�Ĵ���
******************************************************************************/
unsigned int  xdata nCollectValue[32];	// ��¼��׽ֵ
unsigned char xdata nCollectOverV[32];	// ��¼�жϴ���
float		  xdata fFreq[32];			// ���Ƶ��
unsigned int  xdata nFreqN = 0;			// ɨƵʱÿ��Ƶ����Ҫ����ĸ���
unsigned char xdata nTestStatus = 0;	// ����״̬
													 
void PCA0Int() interrupt 11
{
	//���ж�
	OSIntEnter();

	//������*******************************************************************************
	if((INT_TYPE_WAKEN == nIntType) && CCF1)	// CEX1������
	{
		nFreq16Counter += nFreq16Span;

		// ���¼��ص�ƽ�任��Ҫ��������
		PCA0CPL1 = LOBYTE(nFreq16Counter);
		PCA0CPH1 = HIBYTE(nFreq16Counter);

		//�ж��Ƿ������		
		nWakenIndex++;
		if(nWakenIndex >= nWakenMax)
		{
			OSSemPost(pVWSEvent);
		} 	
	}

	//ʰ����*******************************************************************************
	if(INT_TYPE_COLLECT == nIntType) 
	{
		//��׽��Ч
		if(CCF0 == 1)									
		{
			//�ж�ʰ���Ƿ���Ч�������Ч����ǰ����ʰ��
			if(P0&0x08 != 0x08)
			{
				DisablePCA0();						// ��CEX0�ж�
				DisableCFInt();						// ��ֹPCA0����ж�
				DisableCCF0Int();					// ��ֹCEX0�ж�

				//��Ч��־
				nCollectOK = FALSE;

			   	//�����ź���
				OSSemPost(pVWSEvent);
			}
			else
			{ 				
				//��¼ÿһ�εĲ�׽ֵ
				nCollectValue[nCollectIntCount]  = PCA0CPL0;
				nCollectValue[nCollectIntCount] += (PCA0CPH0 << 8);
				nCollectOverV[nCollectIntCount]  = nCollectOverCount;
		
				if(nCollectIntCount >= VWS_COLLECT_COUNT)	// ����20����Ϊ����
				{
					// ʵ�ʲ���Ƶ�ʵ�ʱ���ڲ⵽Ƶ��֮����Ҫ�ر�PCA0������
					DisablePCA0();						// ��CEX0�ж�
					DisableCFInt();						// ��ֹPCA0����ж�
					DisableCCF0Int();					// ��ֹCEX0�ж�
	
					nCollectIntCount = 0;				// �жϴ�������
					nSensorStatus = SENSOR_IDLE;		// �������״̬
					nCollectOK = 1;						// Ƶ�ʲ��Խ���
	
					//�����ź���
					OSSemPost(pVWSEvent);
				}

				nCollectIntCount++;								//��׽������1
			}
		}

		// PCA0����ж�
		if(CF == 1)											
		{	
			// ʰ��ʱ��������������м���
			if(SENSOR_COLLECT == nSensorStatus)				// �����ʰ��״̬��
			{
				nCollectOverCount++;
			}
		}
	}


	// ��������жϱ�־λ
	PCA0CN &= 0x40;										

	//���ж�
	OSIntExit();
}


