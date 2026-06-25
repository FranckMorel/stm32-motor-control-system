/******************************************************************************
 * @file    timer.c
 * @author  Morel
 * @brief   TIM2-based millisecond system tick.
 *
 * This module configures TIM2 to generate an interrupt every 1 ms.
 * The interrupt handler increments a global millisecond counter used for
 * non-blocking timing and simple blocking delays.
 *
 * Clock assumption:
 *  - HSI     = 16 MHz
 *  - SYSCLK  = 16 MHz
 *  - APB1    = 16 MHz
 ******************************************************************************/

#include <stm32f401xe.h>
#include <stdint.h>

#define TIM2_ENABLE_BIT        (1U << 0)
#define TIM_UPDATE_INTERRUPT   (1U << 0)
#define TIM_COUNTER_ENABLE     (1U << 0)

#define TIM2_PRESCALER_VALUE   (1600U - 1U)
#define TIM2_AUTO_RELOAD_VALUE (10U - 1U)

static volatile uint32_t tick_ms = 0U;

/******************************************************************************
 * @brief Initializes TIM2 to generate a 1 ms periodic interrupt.
 *
 * TIM2 clock calculation:
 *  - 16 MHz / 1600 = 10 kHz
 *  - 10 kHz / 10   = 1 kHz
 *  - 1 kHz period  = 1 ms
 ******************************************************************************/
void tim2_init(void)
{
    RCC->APB1ENR |= TIM2_ENABLE_BIT;

    TIM2->PSC = TIM2_PRESCALER_VALUE;
    TIM2->ARR = TIM2_AUTO_RELOAD_VALUE;

    TIM2->CNT = 0U;
    TIM2->SR  = 0U;

    TIM2->DIER |= TIM_UPDATE_INTERRUPT;

    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->CR1 |= TIM_COUNTER_ENABLE;
}

/******************************************************************************
 * @brief TIM2 interrupt service routine.
 *
 * Increments the millisecond tick counter whenever an update event occurs.
 ******************************************************************************/
void TIM2_IRQHandler(void)
{
    if ((TIM2->SR & TIM_UPDATE_INTERRUPT) != 0U)
    {
        TIM2->SR &= ~TIM_UPDATE_INTERRUPT;
        tick_ms++;
    }
}

/******************************************************************************
 * @brief Returns the current millisecond tick count.
 *
 * @return Millisecond counter value since TIM2 initialization.
 ******************************************************************************/
uint32_t timer_ms(void)
{
    return tick_ms;
}

/******************************************************************************
 * @brief Blocking millisecond delay.
 *
 * This function waits until the requested number of milliseconds has elapsed.
 * The subtraction-based comparison works correctly across uint32_t overflow.
 *
 * @param ms Delay time in milliseconds.
 ******************************************************************************/
void delay_ms(uint32_t ms)
{
    uint32_t start = timer_ms();

    while ((timer_ms() - start) < ms)
    {
    }
}
