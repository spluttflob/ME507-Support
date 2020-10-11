/** @file main.cpp
 *    This file contains a simple demonstration program for ME507 which uses
 *    FreeRTOS to do multitasking and some custom code to share data between
 *    tasks.
 *
 *  @author JR Ridgely
 * 
 *  @date 28 Sep 2020  Original file
 *  @date  9 Oct 2020  Added another task because I got bored
 */

#include <Arduino.h>
#if (defined STM32L4xx || defined STM32F4xx)
    #include <STM32FreeRTOS.h>
#endif
#include <PrintStream.h>
#include "taskqueue.h"
#include "taskshare.h"

#include "task_receive.h"


/// This shared data item allows thread-safe transfer of data between tasks
Share<uint32_t> test_share_0 ("Share 0");

/// This queue transmits data from one task to another and has a buffer to
/// hold data in case it piles up when transmitted more quickly than received
/// for a short time
Queue<uint32_t> test_queue_0 (10, "Queue 0.1");

/// This unprotected global variable (ugh) is used to send data from one task
/// to another, but it isn't thread protected and may cause errors
uint32_t bad_global_0;


/** @brief   Task which creates data to be sent to other tasks.
 *  @details This task creates some random data to be sent through a queue,
 *           a share, and a global variable. The data is created with some
 *           redundancy so that errors in transmission are easily detected.
 *  @param   p_params A pointer to function parameters which we don't use.
 */
void task_send (void* p_params)
{
    (void)p_params;            // Does nothing but shut up a compiler warning

    // This 32-bit number will be transmitted to other tasks
    uint32_t number;

    // Seed the random number generator with something dumb and unpredictable
    randomSeed (micros ());

    for (;;)
    {
        // Put a random 16-bit number into the lower 16 bits, then copy that
        // number into the upper 16 bits. Most errors in transmission will
        // show up as a mismatch between the two halves of the number. No, 
        // this method isn't as elegant as a CRC but it's simpler to use
        number = random (0xFFFF);
        number |= number << 16;
        bad_global_0 = number;
        test_share_0.put (number);
        test_queue_0.put (number);

        // Delay the given number of RTOS ticks until beginning to run this
        // task loop again. The resulting timing is not accurate, as the time
        // it took to run the task adds to this interval and accumulates
        vTaskDelay (1);
    }
}


/** @brief   Task which occasionally prints a carriage return.
 *  @details Since the serial monitor gets all ugly when we don't end lines at
 *           reasonable intervals, this task prints a carriage return now and
 *           then.
 *  @param   p_params A pointer to function parameters which we don't use.
 */
void task_returns (void* p_params)
{
    (void)p_params;            // Does nothing but shut up a compiler warning

    for (;;)
    {
        Serial << " Hi. ";

        // Delay the given number of RTOS ticks
        vTaskDelay (32768);
    }
}


/** @brief   Arduino setup function which runs once at program startup.
 *  @details This function sets up a serial port for communication and creates the
 *           tasks which will be run.
 */
void setup () 
{
    // Start the serial port, wait a short time, then say hello. Use the
    // non-RTOS delay function because the RTOS hasn't been started yet
    Serial.begin (115200);
    delay (2000);
    Serial << "\033[2JTesting queues and shares and stuff" << endl;

    // Create a task which prints a slightly disagreeable message
    xTaskCreate (task_send,
                 "Send",                          // Name for printouts
                 256,                             // Stack size
                 NULL,                            // Parameters for task fn.
                 3,                               // Priority
                 NULL);                           // Task handle

    // Create a task which prints a more agreeable message
    xTaskCreate (task_receive,
                 "Receive",
                 256,                             // Stack size
                 NULL,
                 4,                               // Priority
                 NULL);

    // // Create a task which prints a more agreeable message
    // xTaskCreate (task_returns,
    //              "Endl",
    //              256,                             // Stack size
    //              NULL,
    //              1,                               // Priority
    //              NULL);

    print_all_shares (Serial);

    // If using an STM32, we need to call the scheduler startup function now;
    // if using an ESP32, it has already been called for us
    #ifdef STM32L4xx
        vTaskStartScheduler ();
    #endif
}


/** @brief   Arduino's low-priority loop function, which we don't use.
 *  @details A non-RTOS Arduino program runs all of its continuously running
 *           code in this function after @c setup() has finished. When using
 *           FreeRTOS, @c loop() implements a low priority task on most
 *           microcontrollers, and crashes on some others, so we'll not use it.
 */
void loop () 
{
}
