/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: F34xUsbRegister.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年04月10日
**最后修改日期: 2008年04月10日
**描        述: F34x_MSD_USB_Register头文件
*****************************************************************************************************************/
#ifndef  _USB_REGS_H_
#define  _USB_REGS_H_

//USB寄存器定义===================================================================================================
#define  USB_BASE   	0x00
#define  FADDR    		USB_BASE
#define  POWER    		USB_BASE + 0x01
#define  IN1INT   		USB_BASE + 0x02
#define  OUT1INT 		USB_BASE + 0x04
#define  CMINT   		USB_BASE + 0x06
#define  IN1IE   		USB_BASE + 0x07
#define  OUT1IE   		USB_BASE + 0x09
#define  CMIE   		USB_BASE + 0x0B
#define  FRAMEL   		USB_BASE + 0x0C
#define  FRAMEH   		USB_BASE + 0x0D
#define  INDEX    		USB_BASE + 0x0E
#define  CLKREC  		USB_BASE + 0x0F
#define  E0CSR   		USB_BASE + 0x11
#define  EINCSR1  		USB_BASE + 0x11
#define  EINCSR2  		USB_BASE + 0x12
#define  EOUTCSR1 		USB_BASE + 0x14
#define  EOUTCSR2 		USB_BASE + 0x15
#define  E0CNT    		USB_BASE + 0x16
#define  EOUTCNTL 		USB_BASE + 0x16
#define  EOUTCNTH 		USB_BASE + 0x17
#define  FIFO_EP0 		USB_BASE + 0x20
#define  FIFO_EP1 		USB_BASE + 0x21
#define  FIFO_EP2 		USB_BASE + 0x22
#define  FIFO_EP3 		USB_BASE + 0x23

//USB寄存器位定义=================================================================================================
// POWER
#define  rbISOUD        0x80
#define  rbSPEED        0x40
#define  rbUSBRST       0x08
#define  rbRESUME       0x04
#define  rbSUSMD        0x02
#define  rbSUSEN        0x01

// IN1INT
#define  rbIN3          0x08
#define  rbIN2          0x04
#define  rbIN1          0x02
#define  rbEP0          0x01

// OUT1INT
#define  rbOUT3         0x08
#define  rbOUT2         0x04
#define  rbOUT1         0x02

// CMINT
#define  rbSOF          0x08
#define  rbRSTINT       0x04
#define  rbRSUINT       0x02
#define  rbSUSINT       0x01

// IN1IE
#define  rbIN3E         0x08
#define  rbIN2E         0x04
#define  rbIN1E         0x02
#define  rbEP0E         0x01

// OUT1IE
#define  rbOUT3E        0x08
#define  rbOUT2E        0x04
#define  rbOUT1E        0x02

// CMIE
#define  rbSOFE         0x08
#define  rbRSTINTE      0x04
#define  rbRSUINTE      0x02
#define  rbSUSINTE      0x01

// E0CSR
#define  rbSSUEND       0x80
#define  rbSOPRDY       0x40
#define  rbSDSTL        0x20
#define  rbSUEND        0x10
#define  rbDATAEND      0x08
#define  rbSTSTL        0x04
#define  rbINPRDY       0x02
#define  rbOPRDY        0x01

// EINCSR1
#define  rbInCLRDT      0x40
#define  rbInSTSTL      0x20
#define  rbInSDSTL      0x10
#define  rbInFLUSH      0x08
#define  rbInUNDRUN     0x04
#define  rbInFIFONE     0x02
#define  rbInINPRDY     0x01

// EINCSR2
#define  rbInDBIEN      0x80
#define  rbInISO        0x40
#define  rbInDIRSEL     0x20
#define  rbInFCDT       0x08
#define  rbInSPLIT      0x04  

// EOUTCSR1
#define  rbOutCLRDT     0x80
#define  rbOutSTSTL     0x40
#define  rbOutSDSTL     0x20
#define  rbOutFLUSH     0x10
#define  rbOutDATERR    0x08
#define  rbOutOVRUN     0x04
#define  rbOutFIFOFUL   0x02
#define  rbOutOPRDY     0x01

// EOUTCSR2
#define  rbOutDBOEN     0x80
#define  rbOutISO       0x40 

// INDEX IDENTIFIERS
#define  EP0_IDX		0x00
#define  EP1_IN_IDX		0x01
#define  EP2_OUT_IDX	0x02



//操作宏=============================================================================================================
//从USB寄存器读取一个字节
#define UsbReadByte(addr, target) while(USB0ADR&0x80); USB0ADR = (0x80|addr); while(USB0ADR & 0x80); target = USB0DAT
//向USB寄存器写入一个字节
#define UsbWriteByte(addr, data) while(USB0ADR&0x80); USB0ADR = (addr); USB0DAT = data



#endif //_USB_REGS_H_
