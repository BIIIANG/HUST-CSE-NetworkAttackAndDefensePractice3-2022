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

/* ----- xba begin ----- */
#include "RTE_Components.h"
#include CMSIS_device_header
/* ----- xba  end  ----- */

#include <stdio.h>
#include <string.h>
#include "task2a_23.h"

typedef unsigned int uint32_t;

/*----------------------------------------------------------------------------
  Main function
*----------------------------------------------------------------------------*/

void mpu_disable(void) {
    MPU->CTRL = 0;
}

void mpu_enable(uint32_t options) {
    MPU->CTRL = MPU_CTRL_ENABLE_Msk | options;
}

void mpu_region_config(uint32_t region_num, uint32_t addr, uint32_t size, uint32_t attributes) {
    MPU->RNR = region_num;
    MPU->RBAR = addr;
    MPU->RASR = size | attributes;
}

void mpu_region_disable(uint32_t region_num) {
    MPU->RNR = region_num;
    MPU->RBAR = 0;
    MPU->RASR = 0;
}

void vTaskStart(void* pvParameters) {
    /* write your MPU re-configuration code here */

    /* ----- xba begin ----- */
	/* ARM_MPU_Disable() => OK */
	/* ARM_MPU_Disable + ARM_MPU_SetRegionEx => OK */
	/* ARM_MPU_ClrRegion(0ul); => LOSE */
    // ARM_MPU_Disable();		// OK
    // ARM_MPU_SetRegionEx(0UL, 0x08000000UL, ARM_MPU_RASR(0UL, ARM_MPU_AP_FULL, 0UL, 0UL, 1UL, 1UL, 0x00UL, ARM_MPU_REGION_SIZE_1MB));
    // ARM_MPU_Enable(0);
    // mpu_disable();
    // mpu_region_config(0, 0x08000000, 0X13 << 1, MPU_RASR_C_Msk | 0x3 << 24 | MPU_RASR_ENABLE_Msk);
    // mpu_region_disable(1);
    // mpu_region_disable(2);
    // mpu_region_disable(3);
    // mpu_region_disable(4);
    // mpu_region_disable(5);
    // mpu_region_disable(6);
    // mpu_region_disable(7);
    // mpu_enable(0);
    /* ----- xba  end  ----- */

    printf("Attack test begin\n");

    AttackTest();
    for (;;)
        ;
}

int main(void) {
    uint32_t a = 1803;  // replace with your last 4-digital id
    prvSetupHardware();
    xTaskCreate(vTaskStart, "Test2", 100, NULL, (1 | (0x80000000UL)), NULL);
    StartFreeRTOS(a);

    /* Will only get here if there was insufficient memory to create the idle task. */
    for (;;)
        ;
}
