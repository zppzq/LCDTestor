#ifndef     SEARCH_H_H_H
#define     SEARCH_H_H_H
/* Private define ------------------------------------------------------------*/

#define DS18B20_PORT  GPIOB
#define DS18B20_BIT   GPIO_Pin_11
#define DS18B20_DPU   GPIO_Pin_14
#define DS18B20_DRIVE   GPIO_Pin_15
#define DS18B20_BITSET  GPIO_SetBits(DS18B20_PORT, DS18B20_BIT)
#define DS18B20_BITRESET  GPIO_ResetBits(DS18B20_PORT, DS18B20_BIT)
#define DS18B20_BITCHECK  GPIO_ReadInputDataBit(DS18B20_PORT, DS18B20_BIT)


//#define SysTick_CLKSource_HCLK_Div8    ((u32)0xFFFFFFFB)
//#define SysTick_CLKSource_HCLK         ((u32)0x00000004)
/* SysTick counter state */
#define SysTick_Counter_Disable        ((u32)0xFFFFFFFE)
#define SysTick_Counter_Enable         ((u32)0x00000001)
#define SysTick_Counter_Clear          ((u32)0x00000000)
/* CTRL TICKINT Mask */
#define CTRL_TICKINT_Set      ((u32)0x00000002)
#define CTRL_TICKINT_Reset    ((u32)0xFFFFFFFD)

//ROM²Ù×÷Ö¸Áî
//¶ÁROM
#define     READ_ROM            0x33
//Æ¥ÅäROM
#define     MATCH_ROM            0x55
//Ìø¹ýROM
#define        SKIP_ROM            0xcc
//ËÑË÷ROM
#define        SEARCH_ROM            0xf0
//¸æ¾¯ËÑË÷
#define        ALARM_SEARCH        0xec
//´æ´¢Æ÷²Ù×÷Ö¸Áî
//Ð´ÔÝ´æ´æ´¢Æ÷
#define        WRITE_SCRATCHPAD    0x4e
//¶ÁÔÝ´æ´æ´¢Æ÷
#define        READ_SCRATCHPAD        0xbe
//¸´ÖÆÔÝ´æ´æ´¢Æ÷
#define        COPY_SCRATCHPAD        0x48
//ÎÂ¶È±ä»»
#define        CONVERT_TEMPERATURE    0x44
//ÖØÐÂµ÷³ö
#define        RECALL_EPROM        0xb8
//¶ÁµçÔ´
#define        READ_POWER_SUPPLY    0xb4
// method declarations
u32 SysTick_GetCounter(void);
void SysTick_Configuration(void);
void RCC_Config(void);
void GPIO_Configuration(void);
void OWDelay(unsigned int t);
void DS18B20_DirectPort(u16 GPIO_Pin,BitAction  BitVal);
void OWWriteByte(unsigned char Data);
unsigned char OWReadByte(void);
unsigned char *DS18B20_ReadROM(unsigned char *u8Rom);
int OWReset(void);
void DS18B20_Convert(unsigned int n);
unsigned int DS18B20_ReadTemperature(void);
int  OWFirst(void);
int  OWNext(void);
int  OWVerify(void);
void OWTargetSetup(unsigned char family_code);
void OWFamilySkipSetup(void);
void OWWriteByte(unsigned char byte_value);
void OWWriteBit(char bitval);
unsigned char OWReadBit(void);
int  OWSearch(void);
unsigned char docrc8(unsigned char value);

void FindDevices(void);
unsigned char ow_crc( unsigned char x);
unsigned char First(void);
unsigned char Next(void);


extern unsigned char ROM_NO[8];
extern unsigned char FoundROM[10][8]; // table of found ROM codes
extern unsigned char numROMs;

#endif
