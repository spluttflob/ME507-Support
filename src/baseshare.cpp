//*****************************************************************************
/** @file    baseshare.cpp
 *  @brief   Source code of a base class for type-safe, thread-safe task data 
 *           exchange classes.
 *  @details This file contains a base class for classes which exchange data 
 *           between tasks. Inter-task data must be exchanged in a thread-safe
 *           manner, so the classes which share the data use mutexes or mutual 
 *           exclusion mechanisms to prevent corruption of data. A linked list
 *           of all inter-task data items is kept by the system, and this base
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

#include "baseshare.h"                      // Header for the base share class


// Set pointer to most recently created shared data item to initially be NULL
BaseShare* BaseShare::p_newest = NULL;


/** @brief   Construct a base shared data item.
 *  @details This default constructor saves the name of the shared data item. 
 *           It is not to be called by application code (nobody has any reason 
 *           to create a base class object which can't do anything!) but 
 *           instead by the constructors of descendent classes. 
 *  @param   p_name The name for the shared data item, in a character string
 */
BaseShare::BaseShare (const char* p_name)
{
    // Allocate some memory and save the share's name; trim it to 12 characters
    if (p_name != NULL)
    {
        uint8_t namelength = strlen (p_name);
        namelength = (namelength <= 15) ? namelength : 15;
        strncpy (name, p_name, namelength);
    }
    else
    {
        strcpy (name, "(No Name)");
    }

    // Install this share in the linked list of shares
    p_next = p_newest;
    p_newest = this;
}


/** @brief   Start the printout showing the status of all shared data items.
 *  @details This method begins printing out the status of all items in the 
 *           system's linked list of shared data items (queues, task shares, 
 *           and so on). The most recently created share's status is printed
 *           first, followed by the status of other shares in reverse order of
 *           creation. 
 *  @param   printer Pointer to a serial device on which to print
 */
void print_all_shares (Print& printer)
{
    printer.println ("Share/Queue     Type    Max. Full");
    printer.println ("-----------     ----    ---------");

    BaseShare::p_newest->print_in_list (printer);
}
