/** @file task_receive.cpp
 *    This file contains a task which receives data sent by a sending task and
 *    checks for corruption in that data. 
 *
 *  @author JR Ridgely
 * 
 *  @date 11 Oct 2020  Original file
 */

#include "task_receive.h"


// This shared data item allows thread-safe transfer of data between tasks
extern Share<uint32_t> test_share_0;

// This queue transmits data from one task to another and has a buffer to
// hold data in case it piles up when transmitted more quickly than received
// for a short time
extern Queue<uint32_t> test_queue_0;

// This unprotected global variable (ugh) is used to send data from one task
// to another, but it isn't thread protected and may cause errors
extern uint32_t bad_global_0;


/** @brief   Task which receives data from the sender.
 *  @details This task receives random data with redundancy from the sending
 *           task and checks if the data was correctly received. If there
 *           has been an error, the corresponding error counter is incremented.
 *  @param   p_params A pointer to function parameters which we don't use.
 */
void task_receive (void* p_params)
{
    (void)p_params;            // Does nothing but shut up a compiler warning

    uint32_t queue_data;            // Data received via the task queue
    uint32_t share_data;            // Data received via the task share
    uint32_t global_data;           // Data received via the global variable

    uint32_t received = 0;          // The number of sets of data received
    uint16_t mismatches = 0;        // The number of sets which don't agree

    // The number of errors detected during transmission of the data using the
    // queue, share, and global variable
    uint16_t queue_errors = 0;
    uint16_t share_errors = 0;
    uint16_t global_errors = 0;

    for (;;)
    {
        // First get data from the queue. This call will block this task from
        // running until some data arrives here.
        test_queue_0 >> queue_data;                 // .get (queue_data);

        // Now get data from the share and global variable. These should be
        // the same as what arrived in the queue
        test_share_0 >> share_data;
        global_data = bad_global_0;

        // Check the data
        if (share_data != queue_data || global_data != queue_data)
        {
            mismatches++;
        }
        if ((queue_data >> 16) != (queue_data & 0xFFFF))
        {
            queue_errors++;
        }
        if ((share_data >> 16) != (share_data & 0xFFFF))
        {
            share_errors++;
        }
        if ((global_data >> 16) != (global_data & 0xFFFF))
        {
            global_errors++;
        }

        // Increment the counter and periodically print statistics
        if ((received++ % 10000) == 0)
        {
            Serial << received << "  M: " << mismatches << "  S: " 
                   << share_errors << "  Q: "
                   << queue_errors << "  G: " << global_errors 
                   << "  #: 0x" << hex << share_data << dec 
                   << endl;
        }
    }
}

