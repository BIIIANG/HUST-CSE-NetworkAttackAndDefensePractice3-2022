/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher for MPS2
 * Note(s): possible defines set in "options for target - C/C++ - Define"
 *            __USE_LCD    - enable Output on GLCD
 *            __USE_TIMER0 - use Timer0  to generate timer interrupt
 *                         - use SysTick to generate timer interrupt (default)
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2015 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

/* Standard includes. */
#include <stdio.h>
#include <string.h>

typedef unsigned int            uint32_t; 
#include "task2.h"

/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/


uint32_t *MPU_REG_CTRL = (uint32_t *)0XE000ED94;
uint32_t *MPU_REG_RNR  = (uint32_t *)0XE000ED98;
uint32_t *MPU_REG_RBAR = (uint32_t *)0XE000ED9C;
uint32_t *MPU_REG_RASR = (uint32_t *)0XE000EDA0;

void showMPU() {
	printf(" RNR   CTRL    RBAR         RASR\n");
	for (int i = 0; i < 8; i++) {
		*MPU_REG_RNR = i;
		printf( "%3d%7d  0x%08x   0x%08x\n", *MPU_REG_RNR, *MPU_REG_CTRL, *MPU_REG_RBAR, *MPU_REG_RASR );
	}
}


void vTaskStart( void *pvParameters )
{
/* write your MPU re-configuration code here */

	showMPU();
	
	*MPU_REG_RNR  = 6;
	*MPU_REG_RBAR = 0x40040000;
	//     | XN |   | AP  |    | TEX | SCB | SRD       |    | REGION SIZE | ENABLE
	// 000 | 0  | 0 | 001 | 00 | 000 | 010 | 00000000  | 00 | 00100       | 1
	*MPU_REG_RASR = 0x1020009;
	
	printf( "Add MPU done\n" );
	showMPU();
	
  printf( "Attack test begin\n" );
  AttackTest();
  for(;;);
}



int main (void) {

	uint32_t a = 1803; // replace with your last 4-digital id 
    prvSetupHardware();
	xTaskCreate( vTaskStart, "Test2", 100, NULL, ( 1 | ( 0x80000000UL ) ), NULL );
    StartFreeRTOS(a);

	/* Will only get here if there was insufficient memory to create the idle
	task. */
	for(;;);

}




