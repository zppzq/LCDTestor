/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------�� �� �� Ϣ--------------------------------------------------------**
**��   ��   ����															 **
**��   ��   �ˣ�															 **
**�� ��  ʱ �䣺2007.6.1													 **
**����޸�ʱ�䣺													 		 **
**��        ����														 	 **
******************************************************************************/

//--------Compile Control------------------------------------------------------
#define DA_INIT
#define DA_OUT_0
//#define DA_OUT_1
//#define CAL_DA_VALUE_ATOM
//#define CAL_DA_VALUE_LIGHT
//#define OUT_CURRENT_ATOM
//#define OUT_CURRENT_LIGHT

//--------Function Declare-----------------------------------------------------
void 		 	DAInit(void);
void 		 	DAOut0(unsigned int DAValue);
void 			DAOut1(unsigned int DAValue);
unsigned int 	CalDAValueAtom(float fValue);
unsigned int 	CalDAValueLight(float fValue);
void 			OutCurrentAtom(float fValue);
void 			OutCurrentLight(float fValue);