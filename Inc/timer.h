/******************************************************************************
 * @file    timer.h
 * @author  Morel
 * @brief   Public interface for the TIM2 system timer.
 ******************************************************************************/

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

/******************************************************************************
 * @brief Initializes TIM2 as a 1 ms system timer.
 *
 * Configures TIM2 to generate an interrupt every millisecond. The interrupt
 * updates the internal system tick used for timing functions.
 ******************************************************************************/
void tim2_init(void);

/******************************************************************************
 * @brief Returns the current system time in milliseconds.
 *
 * @return Millisecond counter since timer initialization.
 ******************************************************************************/
uint32_t timer_ms(void);

/******************************************************************************
 * @brief Creates a blocking delay.
 *
 * This function blocks the CPU until the specified delay time has elapsed.
 *
 * @param ms Delay time in milliseconds.
 ******************************************************************************/
void delay_ms(uint32_t ms);

#endif /* TIMER_H_ */
