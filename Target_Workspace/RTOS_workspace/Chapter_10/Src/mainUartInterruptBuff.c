/**
 Description:
 * Fix for a program distributed with the book:  "Hands-On RTOS with Microcontrollers"
 * The present program is a fixed version of "mainUartInterruptBuff.c", from Chapter 10.
 * The bugs in mainUartInterruptBuff.c are described in my web-page:
   * Study Guide for "Hands-On RTOS with Microcontrollers"
   * https://jimyuill.com/embedded-systems/study-guide-freertos-book

 An easy way to build and use this program:
 * Make a back-up of your "mainUartInterruptBuff.c".
   * Don't put the backup in the IDE workspace for the book's code, to avoid it being added to the build scripts.
 * Replace the contents of your "mainUartInterruptBuff.c", with the contents of this file.
 * The fixed version of Chapter-10's Uart4Setup.c should also be used, but it might not be essential.

  The original program is mainUartInterruptBuff.c:
 * https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers/blob/master/Chapter_10/Src/mainUartInterruptBuff.c
 * Copyright for mainUartInterruptBuff.c:
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
#include <semphr.h>
#include <timers.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"
#include <stdbool.h>
#include <string.h>

/*********************************************
 * A demonstration of a simple receive-only block
 * based interrupt driven UART driver
 *********************************************/


// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

void wastefulTask( void* NotUsed);
void uartPrintOutTask( void* NotUsed);
void startUart4Traffic( TimerHandle_t xTimer );

static SemaphoreHandle_t rxOkToFill = NULL;
static SemaphoreHandle_t rxOkToOutput = NULL;

static uint_fast16_t rxLen = 0;
static uint8_t* rxBuff = NULL;

static uint_fast16_t rxFillIndex = 0;

static uint32_t rxTestTakeOkNothingReceived = 0;
static uint32_t rxTestTakeFailed = 0;
static uint32_t rxTestTakeFailedAndError = 0;
static uint32_t rxTestTakeFailedAndByteReceived = 0;
static uint32_t rxTestIHcounter = 0;


int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();

	//ensure proper priority grouping for freeRTO
	NVIC_SetPriorityGrouping(0);

	//setup a timer to kick off UART traffic (flowing out of UART4 TX line
	//and into USART2 RX line) 1 second after the scheduler starts
	//this delay is only present since we're using a simple
	//block-based buffer for receiving data - the transmission
	//needs to start after the receiver is ready for data for the
	//strings to start in the correct position in this simple setup
	TimerHandle_t oneShotHandle =
	xTimerCreate(	"startUart4Traffic",
					5000 /portTICK_PERIOD_MS,
					pdFALSE,
					NULL,
					startUart4Traffic);
	assert_param(oneShotHandle != NULL);
	xTimerStart(oneShotHandle, 0);

	// * Use two semaphores to coordinate interactions between the interrupt-handler
	//   and the task uartPrintOutTask().
	// * Access to the following data is serialized by the semaphores.
	//   That data will be referred to as "the serialized-data":
	//   * rxData[20]
	//   * rxFillIndex
	// * Semaphores:
	//   * rxOkToFill : indicates the interrupt-handler can access and modify the
	//                  serialized-data (and uartPrintOutTask() cannot).
	//   * rxOkToOutput : indicates uartPrintOutTask() can access and modify the
	//                    serialized-data (and the interrupt-handler cannot).
	rxOkToFill = xSemaphoreCreateBinary();
	rxOkToOutput = xSemaphoreCreateBinary();
	assert_param(rxOkToFill != NULL);
	assert_param(rxOkToOutput != NULL);

	//setup tasks, making sure they have been properly created before moving on
	assert_param(xTaskCreate(uartPrintOutTask, "uartPrint", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);
	assert_param(xTaskCreate(wastefulTask, "wastefulTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

void startUart4Traffic( TimerHandle_t xTimer )
{
	SetupUart4ExternalSim(9600);
}

void wastefulTask( void* NotUsed)
{
	while(1)
	{
		volatile int i, j;
		i = 10;
		j = i;
		i = j;
	}
}
void uartPrintOutTask( void* NotUsed)
{
#define BUFFER_ARRAY_LEN 20
	uint8_t rxData[BUFFER_ARRAY_LEN];
	uint8_t expectedLen = 16;

    // Set-up the buffer data
	memset((void*)rxData, 0, BUFFER_ARRAY_LEN);
	rxLen = expectedLen;
	rxBuff = rxData;
	rxFillIndex = 0;

	// Allow interrupt-handler to fill the buffer
	xSemaphoreGive(rxOkToFill);

	// Start USART-2
	STM_UartInit(USART2, 9600, NULL, NULL);
	USART2->CR3 |= USART_CR3_EIE;	//enable error interrupts
	USART2->CR1 |= (USART_CR1_UE | USART_CR1_RXNEIE);
	//all 4 bits are for preemption priority -
	NVIC_SetPriority(USART2_IRQn, 6);
	NVIC_EnableIRQ(USART2_IRQn);

	while(1)
	{
		// Wait for the semaphore needed to output the buffer
		if(xSemaphoreTake(rxOkToOutput, 100) == pdPASS)
		{
			if(expectedLen == rxFillIndex)
			{
				SEGGER_SYSVIEW_PrintfHost("received: ");
				SEGGER_SYSVIEW_Print((char*)rxData);
			}
			else
			{
				SEGGER_SYSVIEW_PrintfHost("expected %i bytes received %i", expectedLen, rxFillIndex);
			}
			// Clear the buffer
			memset((void*)rxData, 0, BUFFER_ARRAY_LEN);
			rxFillIndex = 0;

			// Allow the interrupt-handler to fill the buffer
			xSemaphoreGive(rxOkToFill);
		}
		else
		{
			SEGGER_SYSVIEW_PrintfHost("timeout");
		}
	}
}

void USART2_IRQHandler( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken_take = pdFALSE;
	portBASE_TYPE xHigherPriorityTaskWoken_give = pdFALSE;
	portBASE_TYPE xHigherPriorityTaskWoken;
	SEGGER_SYSVIEW_RecordEnterISR();

	bool errorOccurred = false;
	bool byteReceived = false;

	uint8_t tempVal;

	// * For use in testing, e.g., to check the values of the variables used for testing.
	// * A breakpoint can be set in the If-body.
	rxTestIHcounter++;
	if ( (rxTestIHcounter % 5000) == 0 ){
		rxTestIHcounter++; // Just a placeholder for setting a breakpoint
	}

	//first check for errors
	if(	USART2->ISR & (	USART_ISR_ORE_Msk |
						USART_ISR_NE_Msk |
						USART_ISR_FE_Msk |
						USART_ISR_PE_Msk ))
	{
		//clear error flags
		USART2->ICR |= (USART_ICR_FECF |
						USART_ICR_PECF |
						USART_ICR_NCF |
						USART_ICR_ORECF);

		// If a transfer was in progress, it will be cancelled
		errorOccurred = true;
	}

	if(	USART2->ISR & USART_ISR_RXNE_Msk)
	{
		//read the data register unconditionally to clear
		//the receive not empty interrupt if no reception is
		//in progress
		tempVal = (uint8_t) USART2->RDR;
		byteReceived = true;
	}

	// * Check if the semaphore rxOkToFill is available.
	// * If the semaphore is not available, the interrupt-handler will not block.
	// * The second parameter is required, but probably not relevant given the program-design.
	if ( xSemaphoreTakeFromISR(rxOkToFill, &xHigherPriorityTaskWoken_take) == pdPASS){

		// * If an error occurred, give the semaphore rxOkToOutput, so the
		//   output can be processed.
		if (errorOccurred){
			xSemaphoreGiveFromISR(rxOkToOutput, &xHigherPriorityTaskWoken_give);
		}
		else if(byteReceived)
		{
			// * If the received-byte is a null-terminator, replace it with a "!"
			// * The buffer is displayed by SEGGER_SYSVIEW_Print(), and it's display
			//   stops at the first null-terminator.
			// * rxBuff[] is initialized to zeroes, so the string put in it will have a
			//   null-terminator
			if (tempVal == 0){
				tempVal = '!';
			}
			rxBuff[rxFillIndex++] = tempVal;
			if(rxFillIndex >= rxLen)
			{
				// * The buffer is full, give the semaphore rxOkToOutput, so the
				//   output can be processed.
				xSemaphoreGiveFromISR(rxOkToOutput, &xHigherPriorityTaskWoken_give);
			}
			else{
				// * The buffer is not full, give the semaphore rxOkToFill, so another
				//   byte can be added to the buffer.
				xSemaphoreGiveFromISR(rxOkToFill, &xHigherPriorityTaskWoken_give);
			}
		}
		// * This case is for when no error-ocurred, but a byte was not available from USART2
		else  {
			// * rxTestTakeOkNothingReceived is for testing, to record how often this
			//   case occurs
			rxTestTakeOkNothingReceived++;
			// * The buffer is not full, give the semaphore rxOkToFill, so another
			//   byte can be added to the buffer.
			xSemaphoreGiveFromISR(rxOkToFill, &xHigherPriorityTaskWoken_give);
		}

	}
	else {
		// * This case occurs when the semaphore rxOkToFill is not available.
		// * The variables are used for testing, to record how often these cases occur.
		rxTestTakeFailed++;
		if (errorOccurred){
			rxTestTakeFailedAndError++;
		}
		if(byteReceived){
			rxTestTakeFailedAndByteReceived++;
		}
	}

	if ( (xHigherPriorityTaskWoken_give == true) || (xHigherPriorityTaskWoken_take == true) ){
		xHigherPriorityTaskWoken = true;
	}
	else{
		xHigherPriorityTaskWoken = false;
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	SEGGER_SYSVIEW_RecordExitISR();
}
