/** @file   mutex.h
 *    This file contains C++ code which makes it easier to use semaphores such
 *    as mutexes (mutices?) to coordinate some actions between tasks.
 * 
 *  @author JR Ridgely
 *  @date   2020-Nov-16 Original file
 */

#include <Arduino.h>
#if (defined STM32F4xx || defined STM32L4xx)
    #include <FreeRTOS.h>
#endif


/** @brief   Class which implements a mutex which can guard a resource. 
 *  @details A mutex (short for MUTual EXclusion) is used to ensure that two
 *           tasks don't use a resource at the same time. This is one method
 *           which can be used to prevent data corruption due to task 
 *           switching. This class doesn't add functionality to the FreeRTOS
 *           mutex; it just simplifies the programming interface. 
 */
class Mutex
{
protected:
    SemaphoreHandle_t handle;    ///< Handle to the FreeRTOS mutex being used
    TickType_t timeout;          ///< How many RTOS ticks to wait for the mutex

public:
    /** @brief   Create a mutex and save a handle to it.
     *  @details A mutex @b must @b not @b be @b used within an interrupt
     *           service routine; there are ways to use queues to accomplish
     *           the same goal. See the FreeRTOS documentation for details. 
     *  @param   timeout The number of RTOS ticks to wait for the mutex to
     *           become available if another task has it (default 
     *           @c portMAX_DELAY which means wait forever)
     */
    Mutex (TickType_t timeout = portMAX_DELAY)
    {
        handle = xSemaphoreCreateMutex ();
        this->timeout = timeout;
    }

    /** @brief   Take the mutex, preventing other tasks from using whatever
     *           resource the mutex protects.
     *  @returns @c true if the mutex was taken or @c false if we timed out
     */
    bool take (void)
    {
        portBASE_TYPE result = xSemaphoreTake (handle, timeout);

        return (result == pdTRUE);
    }

    /** @brief   Give back the mutex, allowing other tasks to access the
     *           resource protected by the mutex.
     */
    void give (void)
    {
        xSemaphoreGive (handle);
    }
};


