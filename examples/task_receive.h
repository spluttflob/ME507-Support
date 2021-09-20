/** @file task_receive.h
 *    This file contains the header for a task function which receives data 
 *    sent by a sending task and checks for corruption in that data. 
 *
 *  @author JR Ridgely
 * 
 *  @date 11 Oct 2020  Original file
 */

#ifndef _TASK_RECEIVE_H_
#define _TASK_RECEIVE_H_

#include <Arduino.h>
#if (defined STM32L4xx || defined STM32F4xx)
    #include <STM32FreeRTOS.h>
#endif
#include <PrintStream.h>
#include "taskqueue.h"
#include "taskshare.h"


// This is the function that implements the shared data receiving task
void task_receive (void* p_params);

#endif // _TASK_RECEIVE_H_
