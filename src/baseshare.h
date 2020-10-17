//*****************************************************************************
/** @file    baseshare.h
 *  @brief   Headers for a base class for type-safe, thread-safe task data 
 *           exchange classes.
 *  @details This file contains a base class for classes which exchange data 
 *           between tasks. Inter-task data must be exchanged in a thread-safe 
 *           manner, so the classes which share the data use mutexes or mutual 
 *           exclusion mechanisms to prevent corruption of data. A linked list
 *           of all inter-task data items is kept by this system, and this base
 *           class contains members that handle that linked list. 
 *
 *  @date 2014-Oct-18 JRR Created file
 *  @date 2020-Oct-19 JRR Modified for use with Arduino/FreeRTOS platform
 *
 *  License:
 *    This file is copyright 2014 - 2020 by JR Ridgely and released under the
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
#ifndef _BASESHARE_H_
#define _BASESHARE_H_

#include <Arduino.h>


/** @brief   Base class for classes that share data in a thread-safe manner 
 *           between tasks.
 *  @details This is a base class for classes which share data between tasks
 *           without the risk of data corruption associated with global 
 *           variables. Queues and task shares are two examples of such shared
 *           data classes. 
 */
class BaseShare
{
    protected:
        /** @brief   The name of the shared item.
         *  @details This string holds the shared item's name. The name is only
         *           used for identification on debugging printouts or logs.
         */
        char name[16];

        /** @brief   Pointer to the next item in the linked list of shares.
         *  @details This pointer points to the next item in the system's list
         *           of shared data items (shares, queues, @e etc.) If this 
         *           share is the most recently created one, the pointer will 
         *           be @c NULL. The list goes backwards; the next item is the
         *           previously created one.
         */
        BaseShare* p_next;

        /** @brief   Pointer to the most recently created shared data item.
         *  @details This @c static variable, one copy of which is shared by 
         *           all shared data items, is a pointer to the most recently 
         *           created one. It is used as the beginning of a linked list
         *           of all shared data items in the system. 
         */
        static BaseShare* p_newest;

    public:
        // Construct a base shared data item
        BaseShare (const char* p_name = NULL);

        /** @brief   Print one shared data item within a list.
         *  @details Make a printout showing the condition of this shared data
         *           item, such as the value of a shared variable or how full a
         *           queue's buffer is. This method must be overridden in each
         *           descendent class with a method that actually @e does 
         *           something. 
         *  @param   printer Reference to a serial device on which to print 
         */
        virtual void print_in_list (Print& printer) = 0;

        // }
        friend void print_all_shares (Print& printer);
};


// Function that prints a list of shares and queues
void print_all_shares (Print& printer);

#endif // _BASESHARE_H_
