/**
 Description:
 * Fix for a program distributed with the book:  "Hands-On RTOS with Microcontrollers"
 * The present program is a fixed version of "mainUartPolled.c", from Chapter 10.
 * The bugs in mainUartPolled.c are described in my web-page:
   * Study Guide for "Hands-On RTOS with Microcontrollers"
   * https://jimyuill.com/embedded-systems/study-guide-freertos-book

 An easy way to build and use this program:
 * Make a back-up of your "mainUartPolled.c".
   * Don't put the backup in the IDE workspace for the book's code, to avoid it being added to the build scripts.
 * Replace the contents of your "mainUartPolled.c", with the contents of this file.
 * The fixed version of Uart4Setup.c must also be used.

 How to run the present program:
 * Start SystemView after the red LED turns on, but before pushing the push-button.

  The original program is mainUartPolled.c:
 * https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers/blob/master/Chapter_10/Src/mainUartPolled.c
 * Copyright for mainUartPolled.c:
    * MIT License, Copyright (c) 2019 Brian Amos
	* Permission is hereby granted, free of charge, to any person obtaining a copy
	* of this software and associated documentation files (the "Software"), to deal
	* in the Software without restriction, including without limitation the rights
	* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	* copies of the Software, and to permit persons to whom the Software is
	* furnished to do so, subject to the following conditions:
	* The above copyright notice and this permission notice shall be included in all
	* copies or substantial portions of the Software.
    * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    * SOFTWARE.

 Copyright (c) for the present program: MIT License, 2021 Jim Yuill

 */

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"

/*********************************************
 * A demonstration of a polled UART driver for
 * sending and receiving
 *********************************************/


// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

void polledUartReceive ( void* NotUsed );
void uartPrintOutTask( void* NotUsed);

static QueueHandle_t uart2_BytesReceived = NULL;

// #####################################################################
// Code for bug-fix:  START
#include <stdbool.h>
// These two variables are defined in Uart4Setup.c:
extern const char uart4Msg[];
extern const size_t uart4MsgSize;
// Code for bug-fix:  END
// #####################################################################


int main(void)
{
	RedLed.Off();  // Code for bug-fix

	HWInit();
	SetupUart4ExternalSim(9600);
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	//setup tasks, making sure they have been properly created before moving on
	assert_param(xTaskCreate(polledUartReceive, "polledUartRx", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);
	assert_param(xTaskCreate(uartPrintOutTask, "uartPrintTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);

	uart2_BytesReceived = xQueueCreate(10, sizeof(char));

//	for(int i = 0; i < 10; i++)
//	{
//		UART4->TDR = i;
//		while(!(UART4->ISR & USART_ISR_TXE));
//	}

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

void uartPrintOutTask( void* NotUsed)
{
	char nextByte;
	// #####################################################################
	// Code for bug-fix:  START

	unsigned int bytesReceived = 0;
	unsigned int stringsFound = 0;
	unsigned int charsNotRecognized = 0;
	bool stringTrackingMode = false;
	unsigned int expectedCharIndex = 0;

	//Indicate task is waiting for button to be pushed
	RedLed.On();
	//wait for the push button
	while(!ReadPushButton());
	//Indicate button-push was detected
	RedLed.Off();

	// Code for bug-fix:  END
	// #####################################################################


	while(1)
	{
		xQueueReceive(uart2_BytesReceived, &nextByte, portMAX_DELAY);

		// #####################################################################
		// Code for bug-fix:  START

		bytesReceived++;

		// The string being sent is specified in uart4Msg[].
		// * It includes a null-pointer at the end.
		// * The string is sent serially and continuously.

		// This code records:
		// * The number of properly-received strings. Such strings have all expected chars, in order.
		// * The number of chars received that are not among the chars in uart4Msg[]

		// If this is the string's first char, start tracking the received bytes
		if (nextByte == uart4Msg[0]) {
			stringTrackingMode = true;  // Turn on the string-tracking-mode
			expectedCharIndex = 1;
		}
		// * If the string-tracking-mode is on, test if the byte received is the
		//   expected char
		else if ( (stringTrackingMode == true) && (nextByte == uart4Msg[expectedCharIndex]) ) {
			expectedCharIndex++;
			// Test if this byte is the string's last char
			if (expectedCharIndex == uart4MsgSize){
				stringsFound++;
				stringTrackingMode = false;
				expectedCharIndex = 0;
			}
		}
		else {
 			// Test if the received byte is not among the chars in uart4Msg[]
			if ( (nextByte != 0) &&
				 ( (nextByte < uart4Msg[0]) || (nextByte > uart4Msg[uart4MsgSize-1]) ) ) {
				charsNotRecognized++;
			}
			// If string-tracking-mode is on, turn it off
			stringTrackingMode = false;
			expectedCharIndex = 0;
		}

		// Display the stats after every 1000th byte is received
		/*
		if ((bytesReceived % 1000) == 0){
			SEGGER_SYSVIEW_PrintfHost(
"uartPrintOutTask(). Received:: total_bytes:%u  found_strings:%u(%u bytes)  chars_not_recognized:%u",
bytesReceived, stringsFound, (stringsFound * uart4MsgSize), charsNotRecognized);
		}
		*/
		// Code for bug-fix:  END
		// #####################################################################
		SEGGER_SYSVIEW_PrintfHost("%c", nextByte);

	}
}

/**
 * This receive task uses a queue to directly monitor
 * the USART2 peripheral.
 */
void polledUartReceive( void* NotUsed )
{
	uint8_t nextByte;
	//setup UART
	STM_UartInit(USART2, 9600, NULL, NULL);
	while(1)
	{
		while(!(USART2->ISR & USART_ISR_RXNE_Msk));
		nextByte = USART2->RDR;
		xQueueSend(uart2_BytesReceived, &nextByte, 0);
	}
}
