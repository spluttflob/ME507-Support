/** @file encoder_counter.cpp
 *    This file contains a class which allows timer/counters on an STM32 to be
 *    used to read quadrature encoders within the Arduino environment. This 
 *    code is a bit kludgey, as it was put together quickly in response to a
 *    fairly urgent need, but it has been tested and seems to work. It comes 
 *    with no guarantees; YMMV. Valuable assistance was found at 
 *    @c https://www.edwinfairchild.com/2019/04/interface-rotary-encoder-right-way.html
 * 
 *    There are specific pins which must be used for each timer, though some
 *    timers allow one to choose between two sets of pins. 
 * 
 *  @author JR Ridgely
 *  @date   2020-Nov-15 Original file
 */

#include "encoder_counter.h"


/** @brief   Set up an STM32 timer to read a quadrature encoder.
 *  @details This class prepares an STM32 timer in quadrature encoder reading
 *           mode, counting on each transition of either channel. It only works
 *           on timers whose hardware is quadrature compatible; check the data
 *           sheet to see which are. Each compatible timer must have the 
 *           encoder connected to its channel 1 and 2 inputs; these inputs can
 *           only be connected to specific pins. For most timers there are two
 *           sets of pins from which we can choose. The alternate function
 *           tables in the STM32xxx data sheet shows which pins may be used
 *           with each timer. 
 * 
 *           This class has been tested on an STM32L476RG and may or may not
 *           work on other STM32 processors; see the TODO in the constructor. 
 * 
 *           @b Example: 
 *           @code
 *           STM32Encoder timer_X (TIM3, PB4, PB5);      // Set up once
 *           ...
 *           int16_t where_am_I = timer_X.getCount ();   // In a loop
 *           @endcode
 *  @param   timer A pointer to a @c TIM_TypeDef object, such as @c TIM3,
 *           which designates the timer to be used
 *  @param   pin1 The pin to be configured for use with timer channel 1
 *  @param   pin2 The pin to be configured for use with timer channel 2
 */
STM32Encoder::STM32Encoder (TIM_TypeDef* timer, uint8_t pin1, uint8_t pin2)
{
    // Create the timer control object
    p_timer = new HardwareTimer (timer);

    // Temporarily put the timer in a mode where it won't count
    p_timer->pause ();

    // Set channels 1 and 2 in an encoder compatible mode, assigned to the
    // pins which have been selected for those channels. This code might not
    // work for STM32's other than the STM32L476 on which it has been tested
    // because symbolic names aren't used for the bits. TODO: Fix this.
    p_timer->setMode (1, (TimerModes_t)((1UL << 8) | (1UL << 0)), pin1); 
    p_timer->setMode (2, (TimerModes_t)((1UL << 8) | (1UL << 0)), pin2);

    // Set the initial count to zero
    p_timer->setCount (0);

    // Set registers which control the counting mode directly. This has to be
    // done because the HardwareTimer library doesn't handle encoders (lame...)
    // The reference in this function's comment sort of explains how it works
    timer->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;
    timer->CR1 |= TIM_CR1_CEN;
}

