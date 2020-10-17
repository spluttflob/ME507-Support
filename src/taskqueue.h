//*****************************************************************************
/** @file taskqueue.h
 *    This file contains a very simple wrapper class for the FreeRTOS queue. 
 *    It makes using the queue just a little bit easier in C++ than it is in C. 
 *
 *  @date 2012-Oct-21 JRR Original file
 *  @date 2014-Aug-26 JRR Changed file names and queue class name to Queue
 *  @date 2020-Oct-10 JRR Made compatible with Arduino/FreeRTOS environment
 *
 *  License:
 *    This file is copyright 2012-2020 by JR Ridgely and released under the 
 *    Lesser GNU Public License, version 2. It intended for educational use 
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
//*****************************************************************************

// This define prevents this .h file from being included more than once
#ifndef _TASKQUEUE_H_
#define _TASKQUEUE_H_

#include <Arduino.h>
#include "FreeRTOS.h"                       // Main header for FreeRTOS
#include "baseshare.h"


//-----------------------------------------------------------------------------
/** @brief   Implements a queue to transmit data from one RTOS task to another. 
 *  @details Since multithreaded tasks must not use unprotected shared data 
 *           items for communication, queues are a primary means of intertask 
 *           communication. Other means include shared data items (see 
 *           @c taskshare.h) and carrier pigeons. The use of a C++ class
 *           template allows the compiler to check that you're putting the 
 *           correct type of data into each queue and getting the correct type
 *           of data out, thus helping to prevent programming mistakes that can
 *           corrupt your data. 
 * 
 *           As a template class, @c Queue<dataType> can be used to make 
 *           queues which hold data of many types. "Plain Old Data" types such
 *           as @c bool or @c uint16_t are supported, of course. But you can 
 *           also use queues which hold compound data types. For example, if
 *           you have @c class @c my_data which holds several measurements 
 *           together in an object, you can make a queue for @c my_data objects
 *           with @c Queue<my_data>.  Each item in the queue will then hold
 *           several measurements. 
 * 
 *           The size of FreeRTOS queues is limited to 255 items in 8-bit 
 *           microcontrollers whose @c portBASE_TYPE is an 8-bit number. This 
 *           is a FreeRTOS feature. 
 * 
 *           Normal writing and reading are done with methods @c put() and 
 *           @c get(). Normal writing means that the sending task must wait 
 *           until there is empty space in the queue, and then it puts a data
 *           item into the "back" of the queue, where "back" means that the 
 *           item in the back of the queue will be read after all items that 
 *           were previously put into the queue have been read. Normal reading
 *           means that when an item is read from the front of the queue, it 
 *           will then be removed, making space for more items at the back. 
 *           This process is often used to synchronize tasks, as the reading 
 *           task's @c get() method blocks, meaning that the reading task gets
 *           stuck waiting for an item to arrive in the queue; it won't do 
 *           anything useful until new data has been read. Note that this is 
 *           acceptable behavior in an RTOS because the RTOS scheduler will
 *           ensure that other tasks get to run even while the reading task 
 *           is blocking itself waiting for data. 
 * 
 *           In some cases, one may need to use less normal reading and writing
 *           methods. Methods whose name begins with @c ISR_ are to be used 
 *           only within a hardware interrupt service routine. If there is a
 *           need to put data at the front of the queue instead of the back, 
 *           use @c butt_in() instead of @c put(). If one needs to read data 
 *           from the queue without removing that data, the @c look_at() method
 *           allows this to be done. If something particularly unusual needs to
 *           be done with the queue, one can use the method @c get_handle() to
 *           retrieve the handle used by the C language functions in FreeRTOS
 *           to access the Queue object's underlying data structure directly. 
 * 
 *           @section queue_usage Usage
 *           The following bits of code show how to set up and use a queue to
 *           transfer data of type @c int16_t from one hypothetical task 
 *           called @c task_A to another called @c task_B.
 *  
 *           Near the top of the file which contains @c setup() we create a 
 *           queue. The constructor of the @c Queue<int16_t> class is given the
 *           number of items in the queue (10 in this example) and an optional 
 *           name for the queue: 
 *           @code
 *           #include "taskqueue.h"
 *           ...
 *           /// This queue holds hockey puck accelerations
 *           Queue<int16_t> hockey_queue (10, "Puckey");
 *           @endcode
 *           In a location which is before we use the queue in any other file
 *           than the one in which the queue was created, we re-declare the
 *           queue with the keyword @c extern to make it accessible to any task
 *           within that file:
 *           @code
 *           extern Queue<int16_t> hockey_queue;
 *           @endcode
 *           In the sending task, data is put into the queue:
 *           @code
 *           int16_t an_item = -3;                 ///< Local acceleration data
 *           ...
 *           an_item = stick_sensor.get_data (2);  // Read data from sensor 
 *           hockey_queue.put (a_data_item);       // Put data into queue
 *           @endcode
 *           In the receiving task, data is read from the queue. In typical 
 *           usage, the call to @c get() will block the receiving task until 
 *           data has been put into the queue by the sending task:
 *           @code
 *           int16_t data_we_got;                  ///< Holds received data
 *           ...
 *           hockey_queue.get (data_we_got);       // Get data from the queue
 *           @endcode
 */

template <class dataType> class Queue : public BaseShare
{
    // This protected data can only be accessed from this class or its 
    // descendents
    protected:
        QueueHandle_t handle;             ///< Hhandle for the FreeTOS queue
        TickType_t ticks_to_wait;         ///< RTOS ticks to wait for empty
        uint16_t buf_size;                ///< Size of queue buffer in bytes
        uint16_t max_full;                ///< Maximum number of bytes in queue

    // Public methods can be called from anywhere in the program where there is
    // a pointer or reference to an object of this class
    public:
        // The constructor creates a FreeRTOS queue
        Queue (BaseType_t queue_size, const char* p_name = NULL, 
               TickType_t = portMAX_DELAY);

        // Put an item into the queue behind other items.
        bool put (const dataType& item);

        // This method puts an item of data into the back of the queue from 
        // within an interrupt service routine. It must not be used within 
        // non-ISR code. 
        bool ISR_put (const dataType& item);

        /** @brief   Put an item into the front of the queue to be retrieved 
         *           first.
         *  @details This method puts an item into the front of the queue so
         *           that it will be received first as long as nothing else is
         *           put in front of it. This is not the normal way to put 
         *           things into a queue; using @c put() to put items into the
         *           back of the queue is. If you always use this method, 
         *           you're making a stack rather than a queue, you weirdo. 
         *           This method must @b not be used within an interrupt 
         *           service routine. 
         *  @param   item Reference to the item which is going to be (rudely) 
         *           put into the front of the queue
         *  @return  @c True if the item was successfully queued, false if not
         */
        bool butt_in (const dataType& item)
        {
            return ((bool)(xQueueSendToFront (handle, &item, ticks_to_wait)));
        }

        // This method puts an item into the front of the queue from within 
        // an ISR. It must not be used within normal, non-ISR code. 
        bool ISR_butt_in (const dataType& item);

        /** @brief   Return true if the queue is empty.
         *  @details This method checks if the queue is empty. It returns 
         *           @c true if there are no items in the queue and @c false if
         *           there are items.
         *  @return  @c true if the queue is empty, @c false if it's not empty
         */
        bool is_empty (void)
        {
            return (uxQueueMessagesWaiting (handle) == 0);
        }

        /** @brief   Return true if the queue is empty, from within an ISR.
         *  @details This method checks if the queue is empty from within an 
         *           interrupt service routine. It must not be used in normal
         *           non-ISR code. 
         *  @return  @c true if the queue is empty, @c false if it's not empty
         */
        bool ISR_is_empty (void)
        {
            return (uxQueueMessagesWaitingFromISR (handle) == 0);
        }

        // Get an item from the queue
        void get (dataType& recv_item);

        // Get an item from the queue from within an interrupt service routine
        void ISR_get (dataType& recv_item);

        // Look at the first available item in the queue but don't remove it
        void peek (dataType& recv_item);

        // Look at the first item in the queue from within an interrupt 
        // service routine
        void ISR_peek (dataType& recv_item);

        /** @brief   Return true if the queue has contents which can be read.
         *  @details This method allows one to check if the queue has any 
         *           contents. It must @b not be called from within an 
         *           interrupt service routine.
         *  @return  @c true if there's something in the queue, @c false if not
         */
        bool any (void)
        {
            return (uxQueueMessagesWaiting (handle) != 0);
        }

        /** @brief   Return true if the queue has items in it, from within an 
         *           ISR.
         *  @details This method allows one to check if the queue has any 
         *           contents from within an interrupt service routine. It must
         *           @b not be called from within normal, non-ISR code. 
         *  @return  @c true if there's something in the queue, @c false if not
         */
        bool ISR_any (void)
        {
            return (uxQueueMessagesWaitingFromISR (handle) != 0);
        }

        /** @brief   Return the number of items in the queue.
         *  @details This method returns the number of items waiting in the 
         *           queue. It must @b not be called from within an interrupt 
         *           service routine; the method @c ISR_num_items_in() can be 
         *           called from within an ISR. 
         *  @return  The number of items in the queue
         */
        unsigned portBASE_TYPE available (void)
        {
            return (uxQueueMessagesWaiting (handle));
        }

        /** @brief   Return the number of items in the queue, to an ISR.
         *  @details This method returns the number of items waiting in the 
         *           queue; it must be called only from within an interrupt 
         *           service routine.
         *  @return  The number of items in the queue
         */
        unsigned portBASE_TYPE ISR_available (void)
        {
            return (uxQueueMessagesWaitingFromISR (handle));
        }

        /** @brief   Print the queue's status to a serial device.
         *  @details This method makes a printout of the queue's status on 
         *           the given serial device, then calls this same method 
         *           for the next item of thread-safe data in the linked list
         *           of items. 
         *  @param   print_dev Reference to the serial device on which to print
         */
        void print_in_list (Print& print_dev);

        /** @brief   Indicates whether this queue is usable.
         *  @details This method returns a value which is @c true if this queue
         *           has been successfully set up and can be used. 
         *  @returns @c true if this queue is usable, @c false if not
         */
        bool usable (void)
        {
            return (bool)handle;
        }

        /** @brief   Return a handle to the FreeRTOS structure which runs this
         *           queue.
         *  @details If somebody wants to do something which FreeRTOS queues 
         *           can do but this class doesn't support, a handle for the 
         *           queue wrapped by this class can be used to access the 
         *           queue directly. This isn't commonly done.
         *  @return  The handle of the FreeRTOS queue which is wrapped within 
         *           this C++ class
         */
        QueueHandle_t get_handle (void)
        {
            return handle;
        }
}; // class Queue 


/** @brief   Construct a queue object, allocating memory for the buffer.
 *  @details This constructor creates the FreeRTOS queue which is wrapped by 
 *           the @c Queue class. 
 *  @param   queue_size The number of characters which can be stored in the 
 *           queue
 *  @param   p_name A name to be shown in the list of task shares (default 
 *           empty String)
 *  @param   wait_time How long, in RTOS ticks, to wait for a queue to become
 *           empty before a character can be sent. (Default: @c portMAX_DELAY,
 *           which causes the sending task to block until sending occurs.)
 */
template <class dataType>
Queue<dataType>::Queue (BaseType_t queue_size, const char* p_name, 
                        TickType_t wait_time)
    : BaseShare (p_name)
{
    // Create a FreeRTOS queue object with space for the data items
    handle = xQueueCreate (queue_size, sizeof (dataType));

    // Store the wait time; it will be used when writing to the queue
    ticks_to_wait = wait_time;

    // Save the buffer size
    buf_size = queue_size;

    // We haven't stored any items in the queue yet
    max_full = 0;
}


/** @brief   Remove the item at the head of the queue.
 *  @details This method gets the item at the head of the queue and removes
 *           that item from the queue. If there's nothing in the queue, this 
 *           method waits, blocking the calling task, for the number of RTOS 
 *           ticks specified in the @c wait_time parameter to the queue 
 *           constructor (the default is forever) or until something shows up. 
 *  @param   recv_item A reference to the item to be filled with data from the
 *           queue
 */
template <class dataType>
inline void Queue<dataType>::get (dataType& recv_item)
{
    // If xQueueReceive doesn't return pdTrue, nothing was found in the queue, 
    // so no changes are made to the item
    xQueueReceive (handle, &recv_item, ticks_to_wait);
}


/** @brief   Remove the item at the head of the queue from within an ISR.
 *  @details This method gets and returns the item at the head of the queue 
 *           from within an interrupt service routine. This method must @b not 
 *           be called from within normal non-ISR code. 
 *  @param   recv_item A reference to the item to be filled with data from the
 *           queue
 */
template <class dataType>
inline void Queue<dataType>::ISR_get (dataType& recv_item)
{
    portBASE_TYPE task_awakened;            // Checks if context switch needed

    // If xQueueReceive doesn't return pdTrue, nothing was found in the queue,
    // so we'll return the item as created by its default constructor
    xQueueReceiveFromISR (handle, &recv_item, &task_awakened);
}


/** @brief   Return the item at the queue head without removing it.
 *  @details This method returns the item at the head of the queue without 
 *           removing that item from the queue. If there's nothing in the queue
 *           this method waits, blocking the calling task, for for the number
 *           of RTOS ticks specified in the @c wait_time parameter to the queue
 *           constructor (the default is forever) or until something shows up. 
 *           This method must \b not be called from within an interrupt service
 *           routine. 
 *  @param   recv_item A reference to the item to be filled with data from the
 *           queue
 */
template <class dataType>
inline void Queue<dataType>::peek (dataType& recv_item)
{
    // If xQueueReceive doesn't return pdTrue, nothing was found in the queue,
    // so don't change the item
    xQueuePeek (handle, &recv_item, ticks_to_wait);
}


/** @brief   Return the item at the front of the queue without deleting it, 
 *           from within an ISR.
 *  @details This method returns the item at the head of the queue without 
 *           removing that item from the queue. If there's nothing in the 
 *           queue, this method returns the result of the default constructor 
 *           for the data item, usually zero in the given data type. This 
 *           method must \b not be called from within an interrupt service 
 *           routine. 
 *  @param   recv_item A reference to the item to be filled with data from the
 *           queue
 */
template <class dataType>
inline void Queue<dataType>::ISR_peek (dataType& recv_item)
{
    portBASE_TYPE task_awakened;             // Checks if a task will wake up

    // If xQueueReceive doesn't return pdTrue, nothing was found in the queue,
    // so the value of recv_item is not changed
    xQueuePeekFromISR (handle, &recv_item, &task_awakened);
}


/** @brief   Put an item into the queue behind other items.
 *  @details This method puts an item of data into the back of the queue, which
 *           is the normal way to put something into a queue. If you want to be
 *           rude and put an item into the front of the queue so it will be 
 *           retrieved first, use @c butt_in() instead. <b>This method must not
 *           be used within an Interrupt Service Routine.</b>
 *  @param   item Reference to the item which is going to be put into the queue
 *  @return  True if the item was successfully queued, false if not
 */
template <class dataType>
bool Queue<dataType>::put (const dataType& item)
{
    bool return_value = (bool)(xQueueSendToBack (handle, &item, 
                                                 ticks_to_wait));

    // Keep track of the maximum fillage of the queue
    uint16_t fillage = uxQueueMessagesWaiting (handle);
    if (fillage > max_full)
    {
        max_full = fillage;
    }

    return (return_value);
}


/** @brief   Put an item into the queue from within an ISR.
 *  @details This method puts an item of data into the back of the queue from
 *           within an interrupt service routine. It must \b not be used within
 *           non-ISR code. 
 *  @param   item Reference to the item which is going to be put into the queue
 *  @return  True if the item was successfully queued, false if not
 */
template <class dataType>
inline bool Queue<dataType>::ISR_put (const dataType& item)
{
    // This value is set true if a context switch should occur due to this data
    signed portBASE_TYPE shouldSwitch = pdFALSE;

    bool return_value;                      // Value returned from this method

    // Call the FreeRTOS function and save its return value
    return_value = (bool)(xQueueSendToBackFromISR (handle, &item, 
                                                   &shouldSwitch));

    // Keep track of the maximum fillage of the queue. BUG: max_full isn't
    // thread safe (but getting max_full corrupted shouldn't cause a calamity)
    uint16_t fillage = uxQueueMessagesWaitingFromISR (handle);
    if (fillage > max_full)
    {
        max_full = fillage;
    }

    // Return the return value saved from the call to xQueueSendToBackFromISR()
    return (return_value);
}


/** @brief   Put an item into the front of the queue from within an ISR.
 *  @details This method puts an item into the front of the queue from within
 *           an ISR. It must \b not be used within normal, non-ISR code. 
 *  @param   item The item which is going to be (rudely) put into the front of
 *           the queue
 *  @return  True if the item was successfully queued, false if not
 */
template <class dataType>
bool Queue<dataType>::ISR_butt_in (const dataType& item)
{
    // This value is set true if a context switch should occur due to this data
    signed portBASE_TYPE shouldSwitch = pdFALSE;

    bool return_value;                        // Value returned from this method

    // Call the FreeRTOS function and save its return value
    return_value = (bool)(xQueueSendToFrontFromISR (handle, &item, 
                                                    &shouldSwitch));

    // Return the return value saved from the call to xQueueSendToBackFromISR()
    return (return_value);
}


/** @brief   Print the queue's status to a serial device.
 *  @details This method makes a printout of the queue's status on the given
 *           serial device, then calls this same method for the next item of 
 *           thread-safe data in the linked list of items. 
 *  @param   print_dev Reference to the serial device on which to print
 */
template <class dataType>
void Queue<dataType>::print_in_list (Print& print_dev)
{
    // Print this task's name and pad it to 16 characters
    print_dev.printf ("%-16squeue\t", name);

    // Print the free and total number of spaces in the queue or an error
    // message if this queue can't be used (probably due to a memory error)
    if (usable ())
    {
        print_dev << max_full << '/' << buf_size << endl;
    }
    else
    {
        print_dev << "UNUSABLE" << endl;
    }

    // Call the next item
    if (p_next != NULL)
    {
        p_next->print_in_list (print_dev);
    }
}


#endif  // _TASKQUEUE_H_
