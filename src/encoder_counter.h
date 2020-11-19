/** @file encoder_counter.h
 *    This file contains a class which allows timer/counters on an STM32 to be
 *    used to read quadrature encoders within the Arduino environment. This 
 *    code is a bit kludgey, as it was put together quickly in response to a
 *    fairly urgent need, but it has been tested and seems to work. It comes 
 *    with no guarantees; YMMV. Valuable assistance was found at 
 *    @c https://www.edwinfairchild.com/2019/04/interface-rotary-encoder-right-way.html
 * 
 *  @author JR Ridgely
 *  @date   2020-Nov-15 Original file
 */

#include <Arduino.h>
#include <HardwareTimer.h>


/** @brief   Class which operates an STM32 timer in quadrature encoder mode.
 *  @details This class sets up a timer/counter which is capable of reading
 *           quadrature signals from an incremental encoder and provides a
 *           method for retrieving a count which corresponds to the encoder's
 *           position. The position will overflow, and this class doesn't deal
 *           with that -- the user must periodically read the position and use
 *           it to update a position count with a large enough bit width that
 *           the full position reading does not overflow.
 * 
 *           So far this class has been tested with the following timers and 
 *           pins on an STM32L476RG:
 *           * Timer @c TIM2 using pins @c PA0 and @c PA1
 *           * Timer @c TIM3 using pins @c PB4 and @c PB5
 *           * Timer @c TIM3 using pins @c PA6 and @c PA7
 *           * Timer @c TIM4 using pins @c PB6 and @c PB7
 *           * Timer @c TIM8 using pins @c PC6 and @c PC7
 */
class STM32Encoder
{
protected:
    HardwareTimer* p_timer;      ///< Pointer to the timer/counter to be used

public:
    STM32Encoder (TIM_TypeDef* timer, uint8_t pin1, uint8_t pin2);

    /** @brief   Return the current position count from the timer.
     *  @returns The value in the timer's count register
     */
    uint16_t getCount (void)
    {
        return p_timer->getCount ();
    }

    /** @brief   Set the counter reading to zero.
     */
    void zero (void)
    {
        p_timer->setCount (0);
    }

    /** @brief   Pause the counter so it won't update its count until resumed. 
     */
    void pause (void)
    {
        p_timer->pause ();
    }

    /** @brief   Resume the counter so it will update its count when the 
     *           encoder is moved.
     */
    void resume (void)
    {
        p_timer->resume ();
    }
};


