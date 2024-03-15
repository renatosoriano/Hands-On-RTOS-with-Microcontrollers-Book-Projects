/*
 Description:
 * Fix for a program distributed with the book:  "Hands-On RTOS with Microcontrollers"
 * The present program is a fixed version of "mainSoftwareTimers.c", from Chapter 8.
 * The bugs in mainSoftwareTimers.c are described in my web-page:
   * Study Guide for "Hands-On RTOS with Microcontrollers"
   * https://jimyuill.com/embedded-systems/study-guide-freertos-book

 An easy way to build and use this program:
 * Make a back-up of your "mainSoftwareTimers.c".
   * Don't put the backup in the IDE workspace for the book's code, to avoid it being added to the build scripts.
 * Replace the contents of your "mainSoftwareTimers.c", with the contents of this file.

 How to run the present program:
 * Start SystemView after the red LED turns on, but before pushing the push-button.

  The original program is mainSoftwareTimers.c:
 * https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers/blob/master/Chapter_8/Src/mainSoftwareTimers.c
 * Copyright for mainSoftwareTimers.c:
    * MIT License, Copyright (c) 2019 Brian Amos
	* Permission is hereby granted, free of charge, to any person obtaining a copy
	* of this software and associated documentation files (the "Software"), to deal
	* in the Software without restriction, including without limitation the rights
	* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	* copies of the Software, and to permit persons to whom the Software is
	* furnished to do so, subject to the following conditions:
	* The above copyright notice and this permission notice shall be included in all
	* copies or substantial portions of the Software.

 Copyright (c) for the present program: MIT License, 2021 Jim Yuill
*/

#include <FreeRTOS.h>
#include <Nucleo_F767ZI_GPIO.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <SEGGER_SYSVIEW.h>
#include <Nucleo_F767ZI_Init.h>
#include <stm32f7xx_hal.h>

// some common variables to use for each task
// 128 * 4 = 512 bytes
//(recommended min stack size per task)
#define STACK_SIZE 128

void startTimersTask( void * argument);

void oneShotCallBack( TimerHandle_t xTimer );
void repeatCallBack( TimerHandle_t xTimer );

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for freeRTOS

	// Set the task's priority to be lower than the timer-task: (configTIMER_TASK_PRIORITY - 1)
	assert_param(xTaskCreate(startTimersTask, "startTimersTask", STACK_SIZE, NULL, (configTIMER_TASK_PRIORITY - 1), NULL) == pdPASS);

	//start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	//if you've wound up here, there is likely an issue with overrunning the freeRTOS heap
	while(1)
	{
	}
}

// Task used to start the timers
void startTimersTask( void* argument )
{

	//Indicate task is waiting for button to be pushed
	RedLed.On();
	//wait for the push button
	while(!ReadPushButton());
	//Indicate button-push was detected
	RedLed.Off();

	SEGGER_SYSVIEW_PrintfHost("startTimersTask: button pushed");

	TimerHandle_t repeatHandle =
		xTimerCreate(	"myRepeatTimer",			//name for timer
						500 /portTICK_PERIOD_MS,	//period of timer in ticks
						pdTRUE,						//auto-reload flag
						NULL,						//unique ID for timer
						repeatCallBack);			//callback function
	assert_param(repeatHandle != NULL);
	SEGGER_SYSVIEW_PrintfHost("startTimersTask: repeating timer started (blinks the green LED)");
	xTimerStart(repeatHandle, 0);

	//start with Blue LED on - it will be turned off after one-shot fires
	BlueLed.On();
	SEGGER_SYSVIEW_PrintfHost("startTimersTask: blue LED on");
	TimerHandle_t oneShotHandle =
		xTimerCreate(	"myOneShotTimer",			//name for timer
						2200 /portTICK_PERIOD_MS,	//period of timer in ticks
						pdFALSE,					//auto-reload flag
						NULL,						//unique ID for timer
						oneShotCallBack);			//callback function
	assert_param(oneShotHandle != NULL);

	SEGGER_SYSVIEW_PrintfHost("startTimersTask: one-shot timer started (turns off blue LED)");
	xTimerStart(oneShotHandle, 0);

	SEGGER_SYSVIEW_PrintfHost("startTimersTask: spinning");
	while(1)
	{
	}

}

void oneShotCallBack( TimerHandle_t xTimer )
{
	SEGGER_SYSVIEW_PrintfHost("oneShotCallBack:  blue LED off");
	BlueLed.Off();
}

void repeatCallBack( TimerHandle_t xTimer )
{
	static uint32_t counter = 0;

	SEGGER_SYSVIEW_PrintfHost("repeatCallBack:  toggle Green LED");
	//toggle the green LED
	if(counter++ % 2)
	{
		GreenLed.On();
	}
	else
	{
		GreenLed.Off();
	}
}
