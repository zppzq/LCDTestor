#include "includes.h"
#include "DS18B20\DS18B20.h"
/*
指令                    代码
Read ROM(读ROM)         [33H]
Match ROM(匹配ROM)      [55H]
Skip ROM(跳过ROM]       [CCH]
Search ROM(搜索ROM)     [F0H]
Alarm search(告警搜索)  [ECH]

  指令 							代码
  Write Scratchpad(写暂存存储器) 	[4EH]
  Read Scratchpad(读暂存存储器) 	[BEH]
  Copy Scratchpad(复制暂存存储器) [48H]
  Convert Temperature(温度变换) 	[44H]
  Recall EPROM(重新调出) 			[B8H]
  Read Power supply(读电源) 		[B4H]
*/  
// global search state
int LastDiscrepancy; 
int LastFamilyDiscrepancy; 
int LastDeviceFlag; 
unsigned char crc8;
unsigned char ROM_NO[8]; 
 
 
//-------------------------------------------------------------------------- 
// Find the 'first' devices on the 1-Wire bus 
// Return TRUE  : device found, ROM number in ROM_NO buffer 
//        FALSE : no device present 
// 
int OWFirst() 
{ 
   // reset the search state 
   LastDiscrepancy = 0; 
   LastDeviceFlag = FALSE; 
   LastFamilyDiscrepancy = 0; 
 
   return OWSearch(); 
}
//-------------------------------------------------------------------------- 
// Find the 'next' devices on the 1-Wire bus 
// Return TRUE  : device found, ROM number in ROM_NO buffer 
//        FALSE : device not found, end of search 
// 
int OWNext() 
{ 
   // leave the search state alone 
   return OWSearch(); 
} 
 
//-------------------------------------------------------------------------- 
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing 
// search state. 
// Return TRUE  : device found, ROM number in ROM_NO buffer 
//        FALSE : device not found, end of search 
// 
int OWSearch() 
{ 
   int id_bit_number; 
   int last_zero, rom_byte_number, search_result; 
   int id_bit, cmp_id_bit; 
   unsigned char rom_byte_mask, search_direction; 
 
   // initialize for search 
   id_bit_number = 1; 
   last_zero = 0; 
   rom_byte_number = 0; 
   rom_byte_mask = 1; 
   search_result = 0; 
   crc8 = 0; 
 
   // if the last call was not the last one 
   if (!LastDeviceFlag) 
   { 
      // 1-Wire reset 
      if (!OWReset()) 
      { 
         // reset the search 
         LastDiscrepancy = 0; 
         LastDeviceFlag = FALSE; 
         LastFamilyDiscrepancy = 0; 
         return FALSE; 
      } 
 
      // issue the search command  
      OWWriteByte(0xF0);
      // loop to do the search 
      do 
      { 
         // read a bit and its complement 
         id_bit = OWReadBit(); 
		 OWDelay(120);
         cmp_id_bit = OWReadBit(); 
 
         // check for no devices on 1-Wire 
         if ((id_bit == 1) && (cmp_id_bit == 1)) 
            break; 
         else 
         { 
            // all devices coupled have 0 or 1 
            if (id_bit != cmp_id_bit) 
               search_direction = id_bit;  // bit write value for search 
            else 
            { 
               // if this discrepancy if before the Last Discrepancy 
               // on a previous next then pick the same as last time 
               if (id_bit_number < LastDiscrepancy) 
                  search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0); 
               else 
                  // if equal to last pick 1, if not then pick 0 
                  search_direction = (id_bit_number == LastDiscrepancy); 
 
               // if 0 was picked then record its position in LastZero 
               if (search_direction == 0) 
               { 
                  last_zero = id_bit_number; 
 
                  // check for Last discrepancy in family 
                  if (last_zero < 9) 
                     LastFamilyDiscrepancy = last_zero; 
               } 
            } 
 
            // set or clear the bit in the ROM byte rom_byte_number 
            // with mask rom_byte_mask 
            if (search_direction == 1) 
              ROM_NO[rom_byte_number] |= rom_byte_mask; 
            else 
              ROM_NO[rom_byte_number] &= ~rom_byte_mask; 
 
            // serial number search direction write bit 
            OWWriteBit(search_direction); 
 
            // increment the byte counter id_bit_number 
            // and shift the mask rom_byte_mask 
            id_bit_number++; 
            rom_byte_mask <<= 1; 
 
            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask 
            if (rom_byte_mask == 0) 
            { 
                docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC 
                rom_byte_number++; 
                rom_byte_mask = 1; 
            } 
         } 
      } 
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7 
      // if the search was successful then 
      if (!((id_bit_number < 65) || (crc8 != 0))) 
      { 
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result 
         LastDiscrepancy = last_zero; 
 
         // check for last device 
         if (LastDiscrepancy == 0) 
            LastDeviceFlag = TRUE; 
          
         // check for last family group 
         if (LastFamilyDiscrepancy == LastDiscrepancy) 
            LastFamilyDiscrepancy = 0; 
 
         search_result = TRUE; 
      } 
   } 
 
   // if no device found then reset counters so next 'search' will be like a first 
   if (!search_result || !ROM_NO[0]) 
   { 
      LastDiscrepancy = 0; 
      LastDeviceFlag = FALSE; 
      LastFamilyDiscrepancy = 0; 
      search_result = FALSE; 
   } 
 
   return search_result; 
} 
 
//-------------------------------------------------------------------------- 
// Verify the device with the ROM number in ROM_NO buffer is present. 
// Return TRUE  : device verified present 
//        FALSE : device not present 
// 
int OWVerify() 
{ 
   unsigned char rom_backup[8]; 
   int i,rslt,ld_backup,ldf_backup,lfd_backup; 
 
   // keep a backup copy of the current state 
   for (i = 0; i < 8; i++) 
      rom_backup[i] = ROM_NO[i]; 
   ld_backup = LastDiscrepancy; 
   ldf_backup = LastDeviceFlag; 
   lfd_backup = LastFamilyDiscrepancy; 
 
   // set search to find the same device 
   LastDiscrepancy = 64; 
   LastDeviceFlag = FALSE; 
 
   if (OWSearch()) 
   { 
      // check if same device found 
      rslt = TRUE; 
      for (i = 0; i < 8; i++) 
      { 
         if (rom_backup[i] != ROM_NO[i]) 
         { 
            rslt = FALSE; 
            break; 
         } 
      } 
   } 
   else 
     rslt = FALSE; 
   // restore the search state  
   for (i = 0; i < 8; i++) 
      ROM_NO[i] = rom_backup[i]; 
   LastDiscrepancy = ld_backup; 
   LastDeviceFlag = ldf_backup; 
   LastFamilyDiscrepancy = lfd_backup; 
 
   // return the result of the verify 
   return rslt; 
} 
 
//-------------------------------------------------------------------------- 
// Setup the search to find the device type 'family_code' on the next call 
// to OWNext() if it is present. 
// 
void OWTargetSetup(unsigned char family_code) 
{ 
   int i; 
 
   // set the search state to find SearchFamily type devices 
   ROM_NO[0] = family_code; 
   for (i = 1; i < 8; i++) 
      ROM_NO[i] = 0; 
   LastDiscrepancy = 64; 
   LastFamilyDiscrepancy = 0; 
   LastDeviceFlag = FALSE; 
} 
 
//-------------------------------------------------------------------------- 
// Setup the search to skip the current device type on the next call 
// to OWNext(). 
// 
void OWFamilySkipSetup() 
{ 
   // set the Last discrepancy to last family discrepancy 
   LastDiscrepancy = LastFamilyDiscrepancy; 
 
   // check for end of list 
   if (LastDiscrepancy == 0) 
      LastDeviceFlag = TRUE; 
} 
 
//-------------------------------------------------------------------------- 
// 1-Wire Functions to be implemented for a particular platform 
//-------------------------------------------------------------------------- 
 
//-------------------------------------------------------------------------- 
// Reset the 1-Wire bus and return the presence of any device 
// Return TRUE  : device present 
//        FALSE : no device present 
// 
int OWReset(void)
{
	u8 temp = 0;
  DS18B20_DirectPort(DS18B20_BIT,Bit_SET);
  DS18B20_BITSET;  
  DS18B20_BITRESET;
  OWDelay(700);//480us以上
  DS18B20_BITSET;
  DS18B20_DirectPort(DS18B20_BIT,Bit_RESET); 
  OWDelay(110);//15~60US
  
  temp = DS18B20_BITCHECK;
  DS18B20_DirectPort(DS18B20_BIT,Bit_SET);
  DS18B20_BITSET;
  OWDelay(100);//60~240US
  return !temp;
}
 
//-------------------------------------------------------------------------- 
// Send 8 bits of data to the 1-Wire bus 
// 
void OWWriteByte(unsigned char Data)
{
	unsigned char i;
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	DS18B20_DirectPort(DS18B20_BIT,Bit_SET);
	OS_ENTER_CRITICAL();
	for(i = 0;i < 8;i++)
	{
		DS18B20_BITRESET;
		OWDelay(1);
		if(Data & 0x01)
		{
			DS18B20_BITSET;
		}
		else
		{
			DS18B20_BITRESET;
		}
		Data >>= 1;
		OWDelay(30);
		DS18B20_BITSET;
		OWDelay(1);
	}
	DS18B20_BITSET;
	OS_EXIT_CRITICAL();
}

//-------------------------------------------------------------------------- 
// Read 1 byte of data to the 1-Wire bus 
// 
unsigned char OWReadByte(void)
{
	unsigned char i,Data = 0;
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	DS18B20_DirectPort(DS18B20_BIT,Bit_SET);
	OS_ENTER_CRITICAL();
	for(i = 0;i < 8;i++)
	{
		DS18B20_BITRESET;
		Data >>= 1;
		DS18B20_BITSET;
		DS18B20_DirectPort(DS18B20_BIT,Bit_RESET);
		if(DS18B20_BITCHECK)
		{
			Data |= 0x80;
		}
		OWDelay(100);
		DS18B20_DirectPort(DS18B20_BIT,Bit_SET);
	}
	OS_EXIT_CRITICAL();
	return Data;
}

//-------------------------------------------------------------------------- 
// Send 1 bit of data to the 1-Wire bus 
// 
void OWWriteBit(char bitval)
{
	DS18B20_DirectPort(DS18B20_BIT,Bit_SET);
	DS18B20_BITRESET; // pull DQ low to start timeslot
	OWDelay(1);	
	if(bitval==1) DS18B20_BITSET; // return DQ high if write 1
	OWDelay(100); // hold value for remainder of timeslot
	DS18B20_BITSET;
	OWDelay(1);
}// Delay provides 16us per loop, plus 24us. Therefore delay(5) = 104us

//--------------------------------------------------------------------------
// Read 1 bit of data from the 1-Wire bus
// Return 1 : bit read is 1
//        0 : bit read is 0
//
unsigned char OWReadBit()
{
	u8 dat = 0;
	DS18B20_DirectPort(DS18B20_BIT,Bit_SET);
	DS18B20_BITRESET;
	OWDelay(1);
	DS18B20_BITSET;
	DS18B20_DirectPort(DS18B20_BIT,Bit_RESET);
	OWDelay(10);
	if ( DS18B20_BITCHECK )
	{
		dat = 1;
	}
	DS18B20_DirectPort(DS18B20_BIT,Bit_SET);
	OWDelay(100);
	return dat;
}

// TEST BUILD 
static unsigned char dscrc_table[] = { 
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65, 
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220, 
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98, 
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255, 
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7, 
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154, 
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36, 
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185, 
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205, 
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80, 
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238, 
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115, 
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139, 
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22, 
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168, 
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53}; 
 
//-------------------------------------------------------------------------- 
// Calculate the CRC8 of the byte value provided with the current  
// global 'crc8' value.  
// Returns current global crc8 value 
// 
unsigned char docrc8(unsigned char value) 
{ 
   // See Application Note 27 
    
   // TEST BUILD 
   crc8 = dscrc_table[crc8 ^ value]; 
   return crc8; 
}
 
/*******************************************************************************
* Function Name  : SysTick_GetCounter
* Description    : Gets SysTick counter value.
* Input          : None
* Output         : None
* Return         : SysTick current value
*******************************************************************************/
u32 SysTick_GetCounter(void)
{
	return(SysTick->VAL);
}

void DS18B20PowerOpen(void)
{
	GPIO_SetBits(GPIOG, GPIO_Pin_9);
}

void DS18B20PowerClose(void)
{
	GPIO_ResetBits(GPIOG, GPIO_Pin_9);
}

void DS18B20_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(DS18B20_PORTCLK,ENABLE);  	  		//打开端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);  	  	//打开端口时钟

	GPIO_InitStructure.GPIO_Pin = DS18B20_BIT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);	  //配置1Wire端口
	GPIO_SetBits(DS18B20_PORT, DS18B20_BIT);

	//传感器电源开关端口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOG, &GPIO_InitStructure);	  //配置传感器电源开关端口

	DS18B20PowerOpen();//打开电源
}
/*******************************************************************************
* Function Name  : DS18B20_Delay
* Description    : 精确延时us,参数范围0~1000.利用SysTick延时
* Input          : 延时时间
* Output         : None
* Return         : None
*******************************************************************************/
void OWDelay(unsigned int t)
{
	unsigned int start,total,diff = 0;
	unsigned int count = 0;
	unsigned int temp;
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	temp = 10000*SYSCLK_1US;
	OS_ENTER_CRITICAL();
	start = SysTick_GetCounter();
	total = t*SYSCLK_1US;
	while(diff < total)
	{
		count = SysTick_GetCounter();
		if(start < count)
		{
			diff = temp - (count - start);
		}
		else
		{
			diff = start -count;
		}
	}
	OS_EXIT_CRITICAL();
}
/*
端口设置为推挽输出，浮空输入。
用库函数设置的时候温度读取失败，原因不明，不知是否是时间问题。
*/
void DS18B20_DirectPort(u16 GPIO_Pin,BitAction  BitVal)
{
	if(1 == BitVal)//Set OutPut
	{	
		DS18B20_PORT->CRH &= 0xFFFF0FFF;DS18B20_PORT->CRH |= 0x00003000;	
	}
	else //Set Input
	{
		DS18B20_PORT->CRH &= 0xFFFF0FFF;DS18B20_PORT->CRH |= 0x00004000;			
	}
}



unsigned char *DS18B20_ReadID(unsigned char *u8Rom)
{
	int i;
	OWReset(); //复位
	OWWriteByte(0x33);
	for(i = 0; i < 8; i++)
	{
		u8Rom[i] = OWReadByte();
	}
	return u8Rom;
}

void DS18B20_SendID(unsigned char *u8Rom)
{
	int i;
	OWWriteByte(0x55);
	for(i = 0; i < 8; i++)
	{
		OWWriteByte(u8Rom[i]);
	}
}

void DS18B20_Convert(uint8 *pID)//温度转换
{
	OWReset(); //复位
//	OWWriteByte(0xcc);
	DS18B20_SendID(pID);

	OWWriteByte(0x44);//发出转换命令
}

unsigned int DS18B20_ReadTemperature(uint8 *pID)
{
	unsigned int Data;
	OWReset(); //复位
	DS18B20_SendID(pID);
	OWWriteByte(0xbe);	//读温度
	Data = OWReadByte();
	Data += (unsigned int)(OWReadByte() << 8);
	return Data;
}

unsigned int GetTemperature(uint8 *pID)
{
	unsigned int Data;
	DS18B20_Convert(pID);
	OWDelay(500);
	Data = DS18B20_ReadTemperature(pID); 
	Data = Data*10/16;
	return Data;
}


////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//
/*
unsigned char ROM[8]; // ROM Bit
//unsigned char LastDiscrepancy = 0; //位指针，指明下次搜索从哪个值差异位开始。
unsigned char doneFlag = 0; // Done flag
unsigned char FoundROM[10][8]; // table of found ROM codes
unsigned char numROMs;
unsigned char dowcrc;

//////////////////////////////////////////////////////////////////////////////
// ONE WIRE CRC
//
unsigned char ow_crc( unsigned char x)
{
	dowcrc = dscrc_table[dowcrc^x];
	return dowcrc;
}

// FIND DEVICES
void FindDevices(void)
{
	unsigned char m;
	if(OWReset()) //Begins when a presence is detected
	{
		if(First()) //Begins when at least one part is found
		{
			numROMs=0;
			do
			{
				numROMs++;	//number on found device
				for(m=0;m<8;m++)
				{
					FoundROM[numROMs][m]=ROM[m]; //Identifies ROM					
				} 
			}while (Next()&&(numROMs<10)); //Continues until no additional devices are found
		}
		else
		{
			printf("Device not found!\r\n");
		}
	}
	else
	{
		printf("Reset failed!\r\n");
	}
}

// FIRST
// The First function resets the current state of a ROM search and calls
// Next to find the first device on the 1-Wire bus.
//
unsigned char First(void)
{
	LastDiscrepancy = 0; // reset the rom search last discrepancy global
	doneFlag = FALSE;
	return Next(); // call Next and return its return value
}

// NEXT
// The Next function searches for the next device on the 1-Wire bus. If
// there are no more devices on the 1-Wire then false is returned.
//
unsigned char Next(void)
{
	unsigned char romBitIndex = 1; // ROM Bit index
	unsigned char romByteIndex = 0; // ROM Byte index
	unsigned char bitMask = 1; // bit mask
	unsigned char x = 0;
	unsigned char discrepMarker = 0; // discrepancy marker
	unsigned char OutputBit; // Output bit
	unsigned char rlt; // return value
	int flag;
	rlt = FALSE; // set the next flag to false
	dowcrc = 0; // reset the dowcrc
	flag = !OWReset(); // reset the 1-Wire
	if(flag||doneFlag) // no parts -> return false
	{
		LastDiscrepancy = 0; // reset the search
		return FALSE;
	}
	OWWriteByte(0xF0); // send SearchROM command
	do
	// for all eight bytes
	{
		x = 0;
		if(OWReadBit()==1) x = 2;
 		OWDelay(120);
		if(OWReadBit()==1) x |= 1; // and its complement
		if(x ==3) // there are no devices on the 1-Wire
			break;
		
		else
		{
			if(x>0) // all devices coupled have 0 or 1
				OutputBit = x>>1; // bit write value for search
			else
			{
				// if this discrepancy is before the last
				// discrepancy on a previous Next then pick
				// the same as last time
				if(romBitIndex<LastDiscrepancy)
					OutputBit = ((ROM[romByteIndex]&bitMask)>0);
				else // if equal to last pick 1
					OutputBit = (romBitIndex==LastDiscrepancy); // if not then pick 0
				// if 0 was picked then record
				// position with mask k
				if (OutputBit==0) discrepMarker = romBitIndex;
			}
			if(OutputBit==1) // isolate bit in ROM[n] with mask k
				ROM[romByteIndex] |= bitMask;
			else
				ROM[romByteIndex] &= ~bitMask;
			OWWriteBit(OutputBit); // ROM search write
			romBitIndex++; // increment bit counter m
			bitMask = bitMask<<1; // and shift the bit mask k
			if(bitMask==0) // if the mask is 0 then go to new ROM
			{ // byte n and reset mask
				ow_crc(ROM[romByteIndex]); // accumulate the CRC
				romByteIndex++; bitMask++;
			}
		}
	}while(romByteIndex<8); //loop until through all ROM bytes 0-7
	if(romBitIndex<65||dowcrc) // if search was unsuccessful then
		LastDiscrepancy=0; // reset the last discrepancy to 0
	else
	{
		// search was successful, so set lastDiscrep,
		// lastOne, nxt
		LastDiscrepancy = discrepMarker;
		doneFlag = (LastDiscrepancy==0);
		rlt = TRUE; // indicates search is not complete yet, more
		// parts remain
	}
	return rlt;
}*/



