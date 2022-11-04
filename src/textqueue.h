/** @file textqueue.h
 *    This file contains a class which implements a FreeRTOS queue specifically
 *    for text, allowing a stream insertion operator to be used to put all
 *    sorts of things (strings, numbers, @a etc.) into the queue. The stream 
 *    extraction operator has been tested on STM32's and ESP32's only; the 
 *    extraction operator @c >> needs somewhat processor specific functions to 
 *    determine if it's running in an ISR or not. 
 *
 *  @date 2021-Sep-17 JRR Original file
 *
 *  License:
 *    This file is copyright 2021 by JR Ridgely and released under the 
 *    Lesser GNU Public License, version 2.1. It intended for educational use 
 *    only, but its use is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIB-
 *    UTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 *    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE. */

// This define prevents this .h file from being included more than once
#ifndef _TEXTQUEUE_H_
#define _TEXTQUEUE_H_

#include <Arduino.h>
#if (defined STM32F4xx || defined STM32L4xx)
    #include "FreeRTOS.h"                       // Main header for FreeRTOS
#endif
#include "taskqueue.h"


/** @brief   Implements a queue to transmit text from one RTOS task to another. 
 *  @details Since multithreaded tasks must not use unprotected shared data 
 *           items for communication, queues are a primary means of intertask 
 *           communication. Other means include shared data items (see 
 *           @c taskshare.h) and carrier pigeons. The use of a C++ class
 *           template allows the compiler to check that you're putting the 
 *           correct type of data into each queue and getting the correct type
 *           of data out, thus helping to prevent programming mistakes that can
 *           corrupt your data. 
 * 
 *           The size of FreeRTOS queues is limited to 255 items in 8-bit 
 *           microcontrollers whose @c portBASE_TYPE is an 8-bit number. This 
 *           is a FreeRTOS feature. 
 * 
 *           @section textqueue_usage Usage
 *           The following bits of code show how to set up and use a queue to
 *           transfer text from one hypothetical task called @c task_A to 
 *           another called @c task_B.
 *
 *           Near the top of the file which contains @c setup() we create a 
 *           queue. The constructor of the @c TextQueue class is given the
 *           maximum number of characters which can be stored in the queue (100
 *           in this example) and an optional name for the queue: 
 *           @code
 *           #include <PrintStream.h>
 *           #include "textqueue.h"
 *           ...
 *           /// This queue holds angry complaints
 *           TextQueue whiny_queue (100, "Complaints");
 *           @endcode
 *           In a location which is before we use the queue in any other file
 *           than the one in which the queue was created, we re-declare the
 *           queue with the keyword @c extern to make it accessible to any task
 *           within that file:
 *           @code
 *           extern TextQueue whiny_queue;
 *           @endcode
 *           In the sending task, @c task_A, text is put into the queue:
 *           @code
 *           int16_t n_fish = -3;                  ///< I guess we owe someone
 *           ...
 *           /// Write into the queue the easy way, with stream insertions
 *           whiny_queue << "I only have " << fish << " fish!" << endl;
 *           @endcode
 *           In the receiving task, data is read from the queue one character at
 *           a time using the @c >> operator. In typical usage, the call to 
 *           @c >> will block the receiving task until data has been put into
 *           the queue by the sending task:
 *           @code
 *           char recv_ch;                         ///< Holds received data
 *           ...
 *           for (;;)
 *           {
 *               whiny_queue.get (recv_ch);        // Get data from the queue
 *               Serial.print (recv_ch);           // Or use << to print
 *           }
 *           @endcode
 */
class TextQueue : public Print, public Queue<char>
{
// No protected data is needed; the parent classes take care of everything

// Public methods can be called from anywhere in the program where there is
// a pointer or reference to an object of this class
public:
    /** @brief   Construct a queue object, allocating memory for the buffer.
     *  @details This constructor creates the FreeRTOS queue which is wrapped 
     *           by the @c Queue class. 
     *  @param   queue_size The number of chars which can be stored in the queue
     *  @param   p_name A name to be shown in the list of task shares (default 
     *           empty String)
     *  @param   wait_time How long, in RTOS ticks, to wait for a queue to 
     *           empty before a character can be sent. 
     *           (Default: @c portMAX_DELAY, which causes the sending task to 
     *           block until sending occurs.)
     */
    TextQueue (BaseType_t queue_size, const char* p_name = NULL, 
               TickType_t wait_time = portMAX_DELAY)
        : Print (), Queue<char> (queue_size, p_name, wait_time)
    {
    }

    /** @brief   Write a character into the queue in the @c Print style.
     *  @details This method is pure virtual in class @c Print, so it must be
     *           specified here. It puts one character into the queue.
     *  @param   a_char The character to be queued
     *  @returns The number of characters written, which is one when returning
     */
    virtual size_t write (uint8_t a_char)
    {
        put (a_char);
        return 1;
    }

}; // class TextQueue 

#endif // _TEXTQUEUE_H_
