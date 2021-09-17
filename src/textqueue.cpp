/** @file textqueue.cpp
 */

#include "textqueue.h"



/** @brief   Construct a queue object, allocating memory for the buffer.
 *  @details This constructor creates the FreeRTOS queue which is wrapped by 
 *           the @c Queue class. 
 *  @param   queue_size The number of chars which can be stored in the queue
 *  @param   p_name A name to be shown in the list of task shares (default 
 *           empty String)
 *  @param   wait_time How long, in RTOS ticks, to wait for a queue to become
 *           empty before a character can be sent. (Default: @c portMAX_DELAY,
 *           which causes the sending task to block until sending occurs.)
 */
TextQueue::TextQueue (BaseType_t queue_size, const char* p_name, 
                      TickType_t wait_time)
    : Print (), Queue<char> (queue_size, p_name, wait_time)
{
    // p_queue = new Queue<char> (queue_size, p_name, wait_time);
}




// /** @brief   Print the queue's status to a serial device.
//  *  @details This method makes a printout of the queue's status on the given
//  *           serial device, then calls this same method for the next item of 
//  *           thread-safe data in the linked list of items. 
//  *  @param   print_dev Reference to the serial device on which to print
//  */
// void TextQueue::print_in_list (Print& print_dev)
// {
//     p_queue->print_in_list (print_dev);
// }

