/** @file    taskshare.h
 *  @brief   Data which can be shared between tasks in a thread-safe manner.
 *  @details This file contains a template class for data which is to be shared
 *           between tasks. The data must be protected against damage due to 
 *           context switches, so it is protected by a mutex or by causing 
 *           transfers to take place inside critical sections of code which are
 *           not interrupted.
 *
 *  @date 2012-Oct-29 JRR Original file
 *  @date 2014-Aug-26 JRR Changed file names, class name to @c TaskShare, 
 *        removed unused version that uses semaphores, renamed @c put() and 
 *        @c get()
 *  @date 2014-Oct-18 JRR Added linked list of all shares for tracking and 
 *        debugging
 *  @date 2020-Oct-10 JRR Made compatible with Arduino, class name to @c Share
 *  @date 2020-Nov-14 JRR Added new-ESP32 compatible @c SHARE_..._CRITICAL(x)
 *
 *  @copyright This file is copyright 2014 -- 2019 by JR Ridgely and released 
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


// Macros that allow critical sections on ESP32's to use a mutex, while those
// on some other processors do not. This is being debugged as of 2020-Nov-14
#ifdef ESP32
    #define SHARE_ENTER_CRITICAL(x) portENTER_CRITICAL(x)
    #define SHARE_EXIT_CRITICAL(x)  portEXIT_CRITICAL(x)
    // #define SHARE_ENTER_CRITICAL_FROM_ISR(x) portENTER_CRITICAL_FROM_ISR(x)
    // #define SHARE_EXIT_CRITICAL_FROM_ISR(x)  portEXIT_CRITICAL_FROM_ISR(x)
#else
    #define SHARE_ENTER_CRITICAL(x) portENTER_CRITICAL()
    #define SHARE_EXIT_CRITICAL(x)  portEXIT_CRITICAL()
    // #define SHARE_ENTER_CRITICAL_FROM_ISR(x) portENTER_CRITICAL_FROM_ISR()
    // #define SHARE_EXIT_CRITICAL_FROM_ISR(x)  portEXIT_CRITICAL_FROM_ISR()
#endif


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
 */
template <class DataType> class Share : public BaseShare
{
    protected:
        DataType the_data;                    ///< Holds the data to be shared

    #ifdef ESP32
        /// A mutex used on ESP32's for critical sections
        portMUX_TYPE mutex;
    #endif

    public:
        /** @brief   Construct a shared data item.
         *  @details This default constructor for a shared data item doesn't do
         *           much besides allocate memory because there isn't any 
         *           particular setup required. Note that the data is @b not 
         *           initialized. 
         *  @param   p_name A name to be shown in the list of task shares 
         *           (default @c NULL)
         */
        Share<DataType> (const char* p_name = NULL) : BaseShare (p_name)
        {
        }

        // This method is used to write data into the shared data item
        void put (DataType);

        // This method is used to write data from within an ISR only
        void ISR_put (DataType);

        // This method is used to read data from the shared data item
        void get (DataType&);

        // This method is used to read data from within an ISR only
        void ISR_get (DataType&);

        // Print the share's status within a list of all shares' statuses
        void print_in_list (Print& printer);

        /**   @brief   The prefix increment causes the shared data to increase
         *             by one.
         *    @details This operator just increases by one the variable held by
         *             the shared data item. @b BUG: It should return a 
         *             reference to this shared data item, but for some reason 
         *             the compiler insists it must return a reference to the 
         *             data @e in the shared data object. Why is unknown. 
         */
        DataType& operator ++ (void)
        {
            SHARE_ENTER_CRITICAL (&mutex);
            the_data++;
            SHARE_EXIT_CRITICAL (&mutex);

            return (the_data);
        }

        /**   @brief The postfix increment causes the shared data to increase
         *           by one.
         */
        DataType operator ++ (int)
        {
            DataType result = the_data;
            SHARE_ENTER_CRITICAL (&mutex);
            the_data++;
            SHARE_EXIT_CRITICAL (&mutex);

            return (result);
        }

        /**   @brief   The prefix decrement causes the shared data to decrease
         *             by one.
         *    @details This operator just decreases by one the variable held by
         *             the shared data item. @b BUG: It should return a 
         *             reference to this shared data item, but for some reason 
         *             the compiler insists it must return a reference to the 
         *             data @e in the shared data object. Why is unknown. 
         */
        DataType& operator -- (void)
        {
            SHARE_ENTER_CRITICAL (&mutex);
            the_data--;
            SHARE_EXIT_CRITICAL (&mutex);

            return (the_data); //// *this);  The BUG
        }

        /**   @brief The postfix decrement causes the shared data to decrease
         *           by one.
         */
        DataType operator -- (int)
        {
            DataType result = the_data;
            SHARE_ENTER_CRITICAL (&mutex);
            the_data--;
            SHARE_EXIT_CRITICAL (&mutex);

            return (result);
        }
}; // class TaskShare<DataType>


/** @brief   Put data into the shared data item.
 *  @details This method is used to write data into the shared data item. It's
 *           declared @c inline so that instead of a regular function call at 
 *           the assembly language level, <tt>an_object.put (x);</tt> will 
 *           result in the code within this function being inserted directly 
 *           into the calling function. This is faster than doing a regular 
 *           function call, which involves pushing the program counter on the 
 *           stack, pushing parameters, jumping, making space for local 
 *           variables, jumping back and popping the program counter, @e etc.
 *  @param   new_data The data which is to be written
 */
template <class DataType>
inline void Share<DataType>::put (DataType new_data)
{
    SHARE_ENTER_CRITICAL (&mutex);
    the_data = new_data;
    SHARE_EXIT_CRITICAL (&mutex);
}


/** @brief   Put data into the shared data item from within an ISR.
 *  @details This method writes data from an ISR into the shared data item. It
 *           must only be called from within an interrupt, not a normal task. 
 *  @param   new_data The data which is to be written into the shared data item
 */
template <class DataType>
void Share<DataType>::ISR_put (DataType new_data)
{
    #ifndef ESP32
        // taskENTER_CRITICAL_FROM_ISR ();
    #endif
    the_data = new_data;
    #ifndef ESP32
        // taskEXIT_CRITICAL_FROM_ISR ();
    #endif
}


/** @brief   Read data from the shared data item.
 *  @details This method is used to read data from the shared data item with 
 *           critical section protection to ensure that the data cannot be 
 *           corrupted by a task switch. The shared data is copied into the
 *           variable which is given as this parameter's function, replacing
 *           the previous contents of that variable. 
 *  @param   recv_data A reference to the variable in which to put received
 *           data
 */
template <class DataType>
void Share<DataType>::get (DataType& recv_data)
{
    // Copy the data from the queue into the receiving variable
    SHARE_ENTER_CRITICAL (&mutex);
    recv_data = the_data;
    SHARE_EXIT_CRITICAL (&mutex);
}


/** @brief   Read data from the shared data item, from within an ISR.
 *  @details This method is used to enable code within an ISR to read data from
 *           the shared data item. It must only be called from within an 
 *           interrupt service routine, not a normal task. 
 *  @param   recv_data A reference to the variable in which to put received
 *           data
 */
template <class DataType>
void Share<DataType>::ISR_get (DataType& recv_data)
{
    #ifndef ESP32
        // taskENTER_CRITICAL_FROM_ISR ();
    #endif
    recv_data = the_data;
    #ifndef ESP32
        // taskEXIT_CRITICAL_FROM_ISR ();
    #endif
}


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
