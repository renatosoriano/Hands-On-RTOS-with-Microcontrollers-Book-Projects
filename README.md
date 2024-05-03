# Hands-On RTOS with Microcontrollers Book Projects

This repo contains projects, updates code, notes, fixes and results for the book "[Hands-On RTOS with Microcontrollers](https://www.packtpub.com/product/hands-on-rtos-with-microcontrollers/9781838826734)" by Brian Amos and published by Packt.

Date: March, 2024.

- The [**SystemView Recordings .SVDat and Image Outputs**](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results) are available. 

## Description from the author

This microcontrollers book starts by introducing you to the concept of RTOS and compares some other alternative methods for achieving real-time performance. Once you've understood the fundamentals, such as tasks, queues, mutexes, and semaphores, you'll learn what to look for when selecting a microcontroller and development environment. 
By working through examples that use an STM32F7 Nucleo board, the STM32CubeIDE, and SEGGER debug tools, including SEGGER J-Link, Ozone, and SystemView, you'll gain an understanding of preemptive scheduling policies and task communication. 
The book will then help you develop highly efficient low-level drivers and analyze their real-time performance and CPU utilization.

## Hardware and Software Requirements

**[STM32 Nucleo-F767ZI Development Board](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html#overview)** - Board used in this course. \
**[Eclipse-based STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)** - C/C++ development platform with peripheral configuration, code generation, code compilation, and debug features for STM32 microcontrollers and microprocessors. Works on Windows/Linux/Mac and is free. \
**[SEGGER SystemView](https://www.segger.com/products/development-tools/systemview/)** - Real-time recording and visualization tool for embedded systems. It reveals the true runtime behavior of an application, going far deeper than the system insights provided by debuggers. This is particularly effective when developing and working with complex embedded systems comprising multiple threads and interrupts. Works on Windows/Linux/Mac and is free. \
**[SEGGER Ozone](https://www.segger.com/products/development-tools/ozone-j-link-debugger/)** - Full-featured graphical debugger for embedded applications. It is possible to debug any embedded application on C/C++ source and assembly level, as well as loading applications built with any tool chain / IDE or debug the target's resident application without any source. Ozone includes all well-known debug controls and information windows and makes use of the best performance of J-Link and J-Trace debug probes. Works on Windows/Linux/Mac and is free. \

## Notes

As with any other software engineering book, it is inevitably to find few specific errors like software tools that were used at the moment of publishing that are no longer supported, or some coding bugs that can be improved in order to get the expected output.

Shout out to Jim Yuill who took the lead on this and in 2022 decided to document with detail these findings in his website [**An Unofficial Study-Guide for the Book: Hands-On RTOS with Microcontrollers**](https://jimyuill.com/embedded-systems/study-guide-freertos-book/).
Whenever I reached an area of confussion or where I thought could be an error, his study guide had the bugs found and fixes that needed to be applied to the source code or the configuration files to run the exercises as expected.

You can find the [**GitHub Issue**](https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers/issues/9) where they share and comment some of this bugs with the author.

Here is his [**GitHub Repo**](https://github.com/jimyuill/embedded-systems-projects-01/tree/main/book--Hands-On-RTOS) with some of the fixes he applied, and from where I based to continue fixing and improving more the the source code and configuration files.

## Fixes
You can find here the most important fixes that need to be applied for running /debugging the applications the most simple way.
For full details and alternatives please refere to the links above in the Notes section.

* #### jdebug files fixes for running correctly Ozone debugger and SystemView tool.
    After building the project and source files with STM32Cube IDE, we need to make use of the .jdebug files provided by the author in order to be capable or flashing the board with Ozone as well as for debugging. However the author left some lines of codes that doesn't have the right attribute/settings or that contain paths making reference to IDE tools no longer available that need to be replaced.
  * The line of code with `Project.SetTargetIF ("JTAG");` needs to be changed for `Project.SetTargetIF ("SWD");`.
  * For the AddSvdFile settings, the author used an IDE that is not used in the book, instead we use STM32CubeIDE and thus the .svd file for the STM32F767 microcontroller is found in other path. Replace `Project.AddSvdFile ("C:\Program Files (x86)\Atollic\TrueSTUDIO for STM32 9.3.0\ide\plugins\com.atollic.truestudio.tsp.stm32_1.0.0.20190212-0734\tsp\sfr\STM32F7x7.svd");` with the coresponding path for STM32CubeIDE, depending if you are using Windows or MacOS might look something like this:
    * For Windows: `Project.AddSvdFile ("C:\ST\STM32CubeIDE_1.5.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.productdb.debug_1.5.0.202011051456\resources\cmsis\STMicroelectronics_CMSIS_SVD\STM32F767.svd");`.
    * For MacOS: `Project.AddSvdFile ("/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.productdb.debug_2.1.100.202311191741/resources/cmsis/STMicroelectronics_CMSIS_SVD/STM32F767.svd");`.
  * For the File.Open settings, the default .elf file is this: `freeRTOS_Nucleo767.elf");` which doesn't match with any of the exercises projects name and needs to be specific for each exercise. Also the path will change according to where each user decided to place its workspace. An example of .elf generated correctly for first exercise 
    * For Windows: `File.Open ("C:\projects\packtBookRTOS\Chapters5_6\Debug\Chapter5_main.elf");`.
    * For MacOS: `File.Open ("/Users/renatosoriano/RTOS_Workspace/Chapter5_6/Debug/Chapter5_main.elf");`.

    Below is the snippet of the .jdebug file for Chapter5 exercise. That would be the only area where those changes are needed, the rest fo the code is fine.

```c
/*********************************************************************
*                                                                     
*       OnProjectLoad                                                 
*                                                                     
* Function description                                                
*   Project load routine. Required.                                   
*                                                                     
**********************************************************************
*/                                                                    
void OnProjectLoad (void) {
  //
  // Dialog-generated settings
  //
  Project.SetTraceSource ("SWO");
  Project.SetDevice ("STM32F767ZI");
  Project.SetHostIF ("USB", "");
  Project.SetTargetIF ("SWD");
  Project.SetTIFSpeed ("50 MHz");
  Project.AddSvdFile ("/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.productdb.debug_2.1.100.202311191741/resources/cmsis/STMicroelectronics_CMSIS_SVD/STM32F767.svd");
  //Project.AddSvdFile ("C:\ST\STM32CubeIDE_1.5.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.productdb.debug_1.5.0.202011051456\resources\cmsis\STMicroelectronics_CMSIS_SVD\STM32F767.svd");  
Project.SetOSPlugin("FreeRTOSPlugin_CM7");
  //
  // User settings
  //
  File.Open ("/Users/renatosoriano/RTOS_Workspace/Chapter5_6/Debug/Chapter5_main.elf");
  //File.Open ("C:\projects\packtBookRTOS\Chapters5_6\Debug\Chapter5_main.elf");
}
```

* #### Compatibility while running Ozone and SystemView

    SystemView hangs when run in conjunction with an Ozone debug-session. There are 2 workarounds:
  * Don't run an Ozone debug-session when SystemView is recording. If it is running, it can be stopped by clicking on: `Debug: Stop Debug Session`. Ozone itself can then be closed or left open.
  * In each .jdebug file for Ozone, disable RTT in code settings by adding this line to the *.jdebug file: `Project.SetRTT(0);`.

## Results

## Chapter 5 'Selecting an IDE' & Chapter 6 'Debbuging Tools for Real-Time Systems':

![Chapter5_6.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter5_6/Outputs/Chapter5_6.png)

## Chapter 7 'The FreeRTOS Scheduler':

`Chapter7_main_taskCreation`:

![Chapter7_main_taskCreation.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter7/Outputs/Chapter7_main_taskCreation.png)

## Chapter 8 'Protecting Data and Synchronizing Tasks':

`Chapter8_mainSemExample_1`:

![Chapter8_mainSemExample_1.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter8/Outputs/Chapter8_mainSemExample_1.png)

`Chapter8_mainSemExample_2`:

![Chapter8_mainSemExample_2.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter8/Outputs/Chapter8_mainSemExample_2.png)

`Chapter8_mainPolledExample`:

![Chapter8_mainPolledExample.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter8/Outputs/Chapter8_mainPolledExample.png)

`Chapter8_mainSemTimeBound_1`:

![Chapter8_mainSemTimeBound_1.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter8/Outputs/Chapter8_mainSemTimeBound_1.png)

`Chapter8_mainSemTimeBound_2`:

![Chapter8_mainSemTimeBound_2.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter8/Outputs/Chapter8_mainSemTimeBound_2.png)

`Chapter8_mainSemPriorityInversion`:

![Chapter8_mainSemPriorityInversion.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter8/Outputs/Chapter8_mainSemPriorityInversion.png)

`Chapter8_mainMutexExample`:

![Chapter8_mainMutexExample.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter8/Outputs/Chapter8_mainMutexExample.png)

`Chapter8_mainSoftwareTimers`:

![Chapter8_mainSoftwareTimers.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter8/Outputs/Chapter8_mainSoftwareTimers.png)

## Chapter 9 'Intertask Communication':

`Chapter9_mainQueueSimplePassByValue`:

![Chapter9_mainQueueSimplePassByValue.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter9/Outputs/Chapter9_mainQueueSimplePassByValue.png)

`Chapter9_mainQueueCompositePassByValue_Debug`:

![Chapter9_mainQueueCompositePassByValue_Debug.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter9/Outputs/Chapter9_mainQueueCompositePassByValue_Debug.png)

`Chapter9_mainQueueCompositePassByValue`:

![Chapter9_mainQueueCompositePassByValue.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter9/Outputs/Chapter9_mainQueueCompositePassByValue.png)

`Chapter9_mainQueueCompositePassByReference`:

![Chapter9_mainQueueCompositePassByReference.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter9/Outputs/Chapter9_mainQueueCompositePassByReference.png)

`Chapter9_mainQueueCompositePassByReference_Debug`:

![Chapter9_mainQueueCompositePassByReference_Debug.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter9/Outputs/Chapter9_mainQueueCompositePassByReference_Debug.png)

`Chapter9_mainTaskNotifications`:

![Chapter9_mainTaskNotifications.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter9/Outputs/Chapter9_mainTaskNotifications.png)


## Chapter 10 'Drivers and ISRs':

`Chapter10_mainUartDMABuff_1`:

![Chapter10_mainUartDMABuff_1.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter10/Outputs/Chapter10_mainUartDMABuff_1.png)

`Chapter10_mainUartDMABuff_2`:

![Chapter10_mainUartDMABuff_2.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter10/Outputs/Chapter10_mainUartDMABuff_2.png)

`Chapter10_mainUartDMABuff_3`:

![Chapter10_mainUartDMABuff_3.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter10/Outputs/Chapter10_mainUartDMABuff_3.png)

`Chapter10_mainUartDMAStreamBufferCont`:

![Chapter10_mainUartDMAStreamBufferCont.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter10/Outputs/Chapter10_mainUartDMAStreamBufferCont.png)

`Chapter10_mainUartInterruptBuff`:

![Chapter10_mainUartInterruptBuff.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter10/Outputs/Chapter10_mainUartInterruptBuff.png)

`Chapter10_mainUartInterruptQueue`:

![Chapter10_mainUartInterruptQueue.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter10/Outputs/Chapter10_mainUartInterruptQueue.png)

`Chapter10_mainUartPolled`:

![Chapter10_mainUartPolled.png](https://github.com/renatosoriano/Hands-On-RTOS-with-Microcontrollers-Book-Projects/blob/main/Target_Workspace/Results/Chapter10/Outputs/Chapter10_mainUartPolled.png)

