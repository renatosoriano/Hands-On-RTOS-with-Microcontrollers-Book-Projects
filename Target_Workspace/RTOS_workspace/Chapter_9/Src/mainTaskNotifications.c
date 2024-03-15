/**
 * MIT License
 *
 * Copyright (c) 2019 Brian Amos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <FreeRTOS.h>
#include <task.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>

/*********************************************
 * A simple demonstration of using task
 * notifications in place of queues
 *********************************************/


// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

#define RED_LED_MASK 0x0001
#define BLUE_LED_MASK 0x0002
#define GREEN_LED_MASK 0x0004

void recvTask( void* NotUsed );
void sendingTask( void* NotUsed );

//this task handle will be used to send direct
//task notifications to recvTask
static TaskHandle_t recvTaskHandle = NULL;

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	//setup tasks, making sure they have been properly created before moving on
	assert_param(xTaskCreate(recvTask, "recvTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &recvTaskHandle) == pdPASS);
	assert_param(xTaskCreate(sendingTask, "sendingTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

	assert_param(recvTaskHandle != NULL);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

/**
 * This receive task watches a queue for a new ledCmd to be added to it
 */
void recvTask( void* NotUsed )
{
	while(1)
	{
		//wait for the next notification value, clearing it to 0
		//after receiving
		uint32_t notificationvalue = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		//Other alternative for ulTaskNotifyTake: xTaskNotifyWait()
		//Source (last part of article): https://jimyuill.com/embedded-systems/study-guide-freertos-book/chapter-9.html

		//uint32_t notificationvalue;
		//xTaskNotifyWait(0, 0, &notificationvalue, portMAX_DELAY);

		SEGGER_SYSVIEW_PrintfHost("Notification received");
		if((notificationvalue & RED_LED_MASK) != 0)
		{
			RedLed.On();
			SEGGER_SYSVIEW_PrintfHost("Red LED ON");
		}
		else
		{
			RedLed.Off();
			SEGGER_SYSVIEW_PrintfHost("Red LED OFF");
		}
		if((notificationvalue & BLUE_LED_MASK) != 0)
		{
			BlueLed.On();
			SEGGER_SYSVIEW_PrintfHost("Blue LED ON");
		}
		else
		{
			BlueLed.Off();
			SEGGER_SYSVIEW_PrintfHost("Blue LED OFF");
		}
		if((notificationvalue & GREEN_LED_MASK) != 0)
		{
			GreenLed.On();
			SEGGER_SYSVIEW_PrintfHost("Green LED ON");
		}
		else
		{
			GreenLed.Off();
			SEGGER_SYSVIEW_PrintfHost("Green LED OFF");
		}
	}
}

/**
 * sendingTask pushes a pointer to alternating instances of LedStates_t
 *
 */
void sendingTask( void* NotUsed )
{
	while(1)
	{
		//send the notification to recvTask - turning on the Red LED
		//since we're overwriting the value, even if another notification is pending
		//there is no need to check the return value (see docs for xTaskNofify in header)
		xTaskNotify( recvTaskHandle, RED_LED_MASK, eSetValueWithOverwrite);
		SEGGER_SYSVIEW_PrintfHost("Notification sent: 'Red LED ON only'");
		vTaskDelay(200);

		xTaskNotify( recvTaskHandle, BLUE_LED_MASK, eSetValueWithOverwrite);
		SEGGER_SYSVIEW_PrintfHost("Notification sent: 'Blue LED ON only'");
		vTaskDelay(200);

		xTaskNotify( recvTaskHandle, GREEN_LED_MASK, eSetValueWithOverwrite);
		SEGGER_SYSVIEW_PrintfHost("Notification sent: 'Green LED ON only'");
		vTaskDelay(200);

		xTaskNotify( recvTaskHandle, RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK, eSetValueWithOverwrite);
		SEGGER_SYSVIEW_PrintfHost("Notification sent: 'ALL LEDs ON'");
		vTaskDelay(200);

		xTaskNotify( recvTaskHandle,0, eSetValueWithOverwrite);
		SEGGER_SYSVIEW_PrintfHost("Notification sent: 'ALL LEDs OFF'");
		vTaskDelay(200);
	}
}

