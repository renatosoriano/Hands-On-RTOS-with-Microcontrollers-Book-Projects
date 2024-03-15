/*
 Overview:
 * The present program is a fixed version of Nucleo_F767ZI_Init.c.
 * The original Nucleo_F767ZI_Init.c is distributed with the book:
   "Hands-On RTOS with Microcontrollers", at
   https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers/blob/master/BSP/Nucleo_F767ZI_Init.c
 * Nucleo_F767ZI_Init.c is used with the book's example programs.

 The bug that was fixed:
 * The bug is in the use of SEGGER_SYSVIEW_PrintfHost().
 * That function-use is described in the book's Chapter 7.
 * The bug is described on my web-page:
   * Study Guide for Hands-On RTOS with Microcontrollers (in the section for Chapter 7)
   * https://jimyuill.com/embedded-systems/study-guide-freertos-book

 An easy way to build and use this program:
 * Make a back-up of your "Nucleo_F767ZI_Init.c".
   * Don't put the backup in the IDE workspace for the book's code, to avoid it being added to the build scripts.
 * Replace the contents of your "Nucleo_F767ZI_Init.c", with the contents of this file.

 Copyright for the original Nucleo_F767ZI_Init.c:
 * The file itself does not have a copyright notice.
 * The repo's copyright notice is:
MIT License
Copyright (c) 2019 Packt
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

  Copyright (c) for the present program: MIT License, 2021 Jim Yuill

 */

#include <FreeRTOS.h>
#include "Nucleo_F767ZI_Init.h"
#include <main.h>
#include <SEGGER_SYSVIEW.h>

// ##################################################
// Code added for bug-fix:  START
#include <stdio.h>
#include <string.h>
/*
The following four strings are used in assert_failed().

* The strings are declared here so they won't take-up stack space.
  (I don't know how much stack-space is available.)

* outputString size calculation:
  * It's the maximum string-length needed for the snprintf() call:
    snprintf(outputString, sizeof(outputString),
      "Assertion Failed.  File: %s%s  Line: %d", pFileNamePrefix, pFileName, line);
  25 bytes: "Assertion Failed.  File: "
  3 bytes: pfileNamePrefix
  FILE_PATH_DISPLAY_LEN bytes: pFileName
  8 bytes: "  Line: "
  6 bytes: line
  1 byte: null-terminator
*/
#define FILE_PATH_DISPLAY_LEN 30  // Display the last 30 bytes
char outputString[25+3+ FILE_PATH_DISPLAY_LEN +8+6+1];
char dots[] = "...";
char emptyString[] = "";
// Code added for bug-fix:  END
// ##################################################


// declarations for 'private' functions not exposed via header file
void SystemClock_Config(void);
static void gpioPinsInit(void);
static void rngInit(void);

UART_HandleTypeDef huart4;
UART_HandleTypeDef uartInitStruct;
/************************************* PUBLIC FUNCTIONS **************************/

/**
 * Initialize the minimum amount of hardware that will be used in all examples
 * Other initialization might also be necessary (PWM for LED's, USART's, USB, etc)
 * and should be performed after the initial call to HWInit
 */
void HWInit( void )
{
	HAL_Init();
	SystemClock_Config();
	gpioPinsInit();			//initialize GPIO lines for LED's
	rngInit();
}

/**
 * NOTE:this function doesn't guarantee a new number every call
 * @param Min smallest number to generate
 * @param Max largest number to generate
 * @returns a pseudo random number from the dedicated RNG peripheral
 */
uint32_t StmRand( uint32_t Min, uint32_t Max )
{
	return RNG->DR %Max + Min;
}


void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	/** Configure LSE Drive Capability
	*/
	HAL_PWR_EnableBkUpAccess();
	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 216;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 2;	//NOTE: this line was not supplied by HAL - it simply
									//sets the struct to match MCU defaults
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Activate the Over-Drive mode
	*/
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInitStruct.PeriphClockSelection =	RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_CLK48|
												RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_UART4;
	PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
	PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_SYSCLK;
	PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
}

/************************************* PRIVATE FUNCTIONS **************************/
//only visible within this compilation unit

/**
  * Initialize all relevant GPIO lines for LED's used in examples, as well as
  * USB pins
  */
static void gpioPinsInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin|LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

}

/**
 * Init the random number generator (RNG) peripheral
 */
static void rngInit( void )
{
	//start the peripheral clock
	__HAL_RCC_RNG_CLK_ENABLE();

	//enable the random number generator
	RNG->CR |= RNG_CR_RNGEN;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  // ##################################################
  // Code added for bug-fix:  START
  char * pFileNamePrefix;
  char * pFileName;
  int length;

  // If the file-name is too long, just display the end of it,
  // and prepend it with "..."
  length = strlen((const char *)file);  // strlen does not include the null-terminator
  if (length > FILE_PATH_DISPLAY_LEN) {
    pFileNamePrefix = dots;
    pFileName = (char *)file + (length - FILE_PATH_DISPLAY_LEN);
  }
  else {
    pFileNamePrefix = emptyString;
    pFileName = (char *)file;
  }

  // Format the output-string
  snprintf(outputString, sizeof(outputString),
    "Assertion Failed.  File: %s%s  Line: %u",
	pFileNamePrefix, pFileName, (unsigned int) line);

  SEGGER_SYSVIEW_PrintfHost(outputString);
  // Code added for bug-fix:  END
  // ##################################################

  //Bug:  SEGGER_SYSVIEW_PrintfHost("Assertion Failed:file %s on line %d\r\n", file, line);

  while(1);
}
#endif /* USE_FULL_ASSERT */