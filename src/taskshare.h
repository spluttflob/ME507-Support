/** @file    taskshare.h
 *  @brief   Data which can be shared between tasks in a thread-safe manner.
 *  @details This file contains a template class for data which is to be shared
 *           between tasks. The data must be protected against damage due to 
 *           context switches, so it is protected by a mutex or by causing 
 *           transfers to take place inside critical sections of code which are
 *           not interrupted. This version is known to work on STM32's and 
 *           ESP32's only because the insertion and extraction operators @c <<
 *           and @c >> need specific functions to determine if they're running 
 *           in ISR's or not. 
 *
 *  @date 2012-Oct-29 JRR Original file
 *  @date 2014-Aug-26 JRR Changed file names, class name to @c TaskShare, 
 *        removed unused version that uses semaphores, renamed @c put() and 
 *        @c get()
 *  @date 2014-Oct-18 JRR Added linked list of all shares for tracking and 
 *        debugging
 *  @date 2020-Oct-10 JRR Made compatible with Arduino, class name to @c Share
 *  @date 2020-Nov-14 JRR Added new-ESP32 compatible @c SHARE_..._CRITICAL(x);
 *                        added @c << and @c >> operators
 *  @date 2020-Nov-18 JRR Critical sections not reliable; changed to a queue
 *  @date 2021-Sep-17 JRR Changed some @c put params from references to copies
 *  @date 2021-Sep-19 JRR Added overloads for @c get() which return values
 *
 *  @copyright This file is copyright 2014 -- 2021 by JR Ridgely and released 
 *    under the Lesser GNU Public License, version 2. It intended for 
 *    educational use only, but its use is not limited thereto. */
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
#ifndef _TASKSHARE_H_
#define _TASKSHARE_H_

#include "baseshare.h"                      // Base class for shared data items
#include "FreeRTOS.h"                       // Main header for FreeRTOS

/** @brief   Class for data to be shared in a thread-safe manner between tasks.
 *  @details This class implements an item of data which can be shared between
 *           tasks without the risk of data corruption associated with global 
 *           variables. Unlike queues, shares do not use a buffer for many data
 *           items; there is only a one-item buffer in which the most recent
 *           value of the data is kept. Shares therefore do not provide the 
 *           task synchronization or incur the overhead associated with queues. 
 * 
 *           The data is protected by using critical code sections (see the 
 *           FreeRTOS documentation of @c portENTER_CRITICAL() ) so that tasks 
 *           can't interrupt each other when reading or writing the data is 
 *           taking place. This prevents data corruption due to thread 
 *           switching. The C++ template mechanism is used to ensure that only
 *           data of the correct type is put into or taken from a shared data
 *           item. A @c TaskShare<DataType> object keeps its own separate copy
 *           of the data. This uses some memory, but it is necessary to 
 *           reliably prevent data corruption; it prevents possible side 
 *           effects from causing the sender's copy of the data from being
 *           inadvertently changed. 
 * 
 *           @section usage_share Usage
 *           The following bits of code show how to set up and use a share to
 *           transfer data of type @c uint16_t from one hypothetical task 
 *           called @c task_A to another called @c task_B.
 *  
 *           In the file which contains @c setup() we create a shared data
 *           object. The constructor of the @c Share<uint16_t> class is given
 *           a name for the share; the name will be shown on system diagnostic
 *           printouts. If it is desired to print diagnostic information to a
 *           serial monitor, a pointer to an output stream may also be given:
 *           @code{.cpp}
 *           #include "taskshare.h"
 *           ...
 *           /// Data from sensor number 3 on the moose's right antler
 *           Share<uint16_t> my_share ("Data_3", &Serial);
 *           @endcode
 *           If there are any tasks which use this share in other source files,
 *           we must re-declare this share with the keyword @c extern near the
 *           top of those files to make it accessible to those task(s). This
 *           copy of the share does not need a Doxygen comment:
 *           @code
 *           // Sensor 3 (right antler) data
 *           extern Share<uint16_t> my_share;
 *           @endcode
 *           In the sending task, data is put into the share:
 *           @code
 *           uint16_t a_data_item = 42;     ///< Holds antler data
 *           ...
 *           a_data_item = antler3 ();      // Get the data
 *           my_share.put (a_data_item);    // Put data into share
 *           @endcode
 *           In the receiving task, data is read from the share:
 *           @code
 *           uint16_t got_data;             ///< Holds received data
 *           ...
 *           my_share.get (got_data);       // Get local copy of shared data
 *           @endcode
 * 
 *           @b The @b Easy @b Way
 *           It's also possible to use overloaded stream insertion and 
 *           extraction operators to interact with the share. These operators
 *           contain code which checks if they're running in an interrupt
 *           service routine (ISR) or not and use the appropriate method to
 *           interact with the internal queue which holds the data, so they're
 *           a little slower than @c put() and @c get(). Usage is as follows,
 *           assuming that the share and local variable have been set up above:
 *           @code
 *           my_share << a_data_item;       // In sending task
 *           ...
 *           my_share >> got_data;          // In receiving task
 *           @endcode
 */
template <class DataType> class Share : public BaseShare
{
protected:
    /// A queue is used to hold the data, as it's portable to different CPU's
    QueueHandle_t queue;

public:
    /** @brief   Construct a shared data item.
     *  @details This constructor for a shared data item creates a queue in 
     *           which to hold one item of data. Note that the data is @b not 
     *           initialized. 
     *  @param   p_name A name to be shown in the list of task shares 
     *           (default @c NULL)
     */
    Share<DataType> (const char* p_name = NULL) : BaseShare (p_name)
    {
        queue = xQueueCreate (1, sizeof (DataType));
    }

    /** @brief   Put data into the shared data item.
     *  @details This method is used to write data into the shared data item. 
     *  @param   new_data The data which is to be written
     */
    void put (DataType new_data)
    {
        xQueueOverwrite (queue, &new_data);
    }

    /** @brief   Put data into the shared data item from within an ISR.
     *  @details This method writes data from an ISR into the shared data item. 
     *           It must only be called from within an interrupt service 
     *           routine, not a normal task. 
     *  @param   new_data The data to be written into the shared data item
     */
    void ISR_put (DataType new_data)
    {
        BaseType_t wake_up;
        xQueueOverwriteFromISR (queue, &new_data, &wake_up);
    }

    /** @brief   Operator which inserts data into the share.
     *  @details This convenient operator puts data into the share, protecting
     *           the data from corruption by thread switching. It checks if the
     *           processor is currently in an interupt service routine (ISR);
     *           if so, it calls ISR specific functions to prevent corruption,
     *           so this function may be used within an ISR or outside one. It
     *           runs a little more slowly than the @c put() method. 
     *  @param   new_data The data which is to be put into the share
     */
    void operator << (DataType new_data)
    {
        if (CHECK_IF_IN_ISR ())
        {
            BaseType_t wake_up;
            xQueueOverwriteFromISR (queue, &new_data, &wake_up);
        }
        else
        {
            xQueueOverwrite (queue, &new_data);
        }
    }

    /** @brief   Read data from the shared data item.
     *  @details This method is used to read data from the shared data item 
     *           with critical section protection to ensure that the data 
     *           cannot be corrupted by a task switch. The shared data is 
     *           copied into the variable which is given as this method's 
     *           parameter, replacing the previous contents. This method checks
     *           if the processor is currently in an interupt service routine 
     *           (ISR) and if so, it calls ISR specific functions to prevent 
     *           corruption, so this function may be used within an ISR or 
     *           outside one. It runs a little more slowly than the @c get() 
     *           method because of the run-time ISR check. 
     *  @param   put_here A reference to the variable in which to put received
     *           data
     */
    void operator >> (DataType put_here)
    {
        if (CHECK_IF_IN_ISR ())
        {
            // Copy the data from the queue into the receiving variable
            xQueuePeekFromISR (queue, &put_here);
        }
        else
        {
            xQueuePeek (queue, &put_here, portMAX_DELAY);
        }
    }

    /** @brief   Read data from the shared data item into a variable.
     *  @details This method is used to read data from the shared data item 
     *           with protection to ensure that the data cannot be corrupted by
     *           a task switch. The shared data is copied into the variable 
     *           which is given as this method's parameter, replacing the 
     *           previous contents of that variable. 
     *  @param   recv_data A reference to the variable in which to put received
     *           data
     */
    void get (DataType& recv_data)
    {
        // Copy the data from the queue into the receiving variable
        xQueuePeek (queue, &recv_data, portMAX_DELAY);
    }

    /** @brief   Read and return data from the shared data item.
     *  @details This method is used to read data from the shared data item 
     *           with protection to ensure that the data cannot be corrupted by
     *           a task switch. A copy of the shared data is returned. 
     *  @returns A copy of the data found in the queue, or a default value of
     *           the given data type if no data was found in the queue
     */
    DataType get (void)
    {
        DataType return_this;
    
        // Copy the data from the queue into the receiving variable
        xQueuePeek (queue, &return_this, portMAX_DELAY);

        return return_this;
    }

    /** @brief   Read data from the shared data item, from within an ISR.
     *  @details This method is used to enable code within an ISR to read data 
     *           from the shared data item. It must only be called from within 
     *           an interrupt service routine, not a normal task. 
     *  @param   recv_data A reference to the variable in which to put received
     *           data
     */
    void ISR_get (DataType& recv_data)
    {
        xQueuePeekFromISR (queue, &recv_data);
    }

    /** @brief   Read and return data from the shared data item, from within an
     *           ISR.
     *  @details This method is used to enable code within an ISR to read data 
     *           from the shared data item. It must only be called from within 
     *           an interrupt service routine, not a normal task. 
     *  @returns A copy of the data found in the queue, or a default value of
     *           the given data type if no data was found in the queue
     */
    DataType ISR_get (void)
    {
        DataType return_this;
        xQueuePeekFromISR (queue, &return_this);
        return return_this;
    }

    // Print the share's status within a list of all shares' statuses
    void print_in_list (Print& printer);

}; // class TaskShare<DataType>


/** @brief   Print the name and type (share) of this data item.
 *  @details This method prints the share's name and a word indicating that it
 *           is a shared data item, as opposed to a queue, formatted to match
 *           similar printouts from other task shares such as queues. After
 *           printing this share's information, it looks in the linked list of
 *           shares for the next one and asks it to print its information too.
 *  @param   printer Reference to a serial device on which to print the status
 */
template <class DataType>
void Share<DataType>::print_in_list (Print& printer)
{
    // Print this task's name and pad it to 16 characters
    printer.printf ("%-16sshare\t", name);

    // End the line
    printer << endl;

    // Call the next item
    if (p_next != NULL)
    {
        p_next->print_in_list (printer);
    }
}

#endif  // _TASKSHARE_H_
