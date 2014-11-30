#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#define SYSTEM_CLOCK_8M 1
#define SYSTEM_CLOCK_16M 2
#define SYSTEM_CLOCK_24M 3
#define SYSTEM_CLOCK_32M 4
#define SYSTEM_CLOCK_40M 5
#define SYSTEM_CLOCK_48M 6
#define SYSTEM_CLOCK_56M 7
#define SYSTEM_CLOCK_64M 8
#define SYSTEM_CLOCK_72M 9

#define SYSTEM_CLOCK SYSTEM_CLOCK_8M

#include "stm32f10x.h"
#include <ucos_ii.h>
#include "bsp.h"
#include <stdio.h>
#include "os_cpu.h"
#include "Serial.h"
#include "SPI.h"
#include "fsmc_nand.h"
#include "spi_flash.h"
#include "ColBusSlaveAps.h"
#endif
		    