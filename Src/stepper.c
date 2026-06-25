/******************************************************************************
 * @file    stepper.c
 * @author  Morel
 * @brief   Stepper motor driver for 28BYJ-48 / ULN2003 control.
 *
 * This module controls a unipolar stepper motor using a 4-phase output
 * sequence. The motor is driven in half-step mode to achieve smoother motion
 * and higher angular resolution.
 ******************************************************************************/

#include <stm32f401xe.h>
#include <stdint.h>
#include <stepper.h>
#include <timer.h>

#define IN1_PIN    0U      /* PA0 */
#define IN2_PIN    1U      /* PA1 */
#define IN3_PIN    4U      /* PA4 */
#define IN4_PIN    0U      /* PB0 */

#define IN1_ON      (GPIOA->BSRR = (1U << 0U))
#define IN1_OFF     (GPIOA->BSRR = (1U << 16U))
#define IN2_ON      (GPIOA->BSRR = (1U << 1U))
#define IN2_OFF     (GPIOA->BSRR = (1U << 17U))
#define IN3_ON      (GPIOA->BSRR = (1U << 4U))
#define IN3_OFF     (GPIOA->BSRR = (1U << 20U))
#define IN4_ON      (GPIOB->BSRR = (1U << 0U))
#define IN4_OFF     (GPIOB->BSRR = (1U << 16U))

#define STEPPER_PHASE_COUNT   8U
#define STEPPER_COIL_COUNT    4U

static uint8_t step_index = 0U;

/*
 * Half-step sequence for 28BYJ-48 stepper motor.
 *
 * Each row represents the output state of IN1, IN2, IN3 and IN4.
 * Half-step mode alternates between single-coil and dual-coil activation.
 */
static const uint8_t phase[STEPPER_PHASE_COUNT][STEPPER_COIL_COUNT] =
{
    {1U, 0U, 0U, 0U},
    {1U, 1U, 0U, 0U},
    {0U, 1U, 0U, 0U},
    {0U, 1U, 1U, 0U},
    {0U, 0U, 1U, 0U},
    {0U, 0U, 1U, 1U},
    {0U, 0U, 0U, 1U},
    {1U, 0U, 0U, 1U}
};

/******************************************************************************
 * @brief Applies one phase pattern to the stepper motor driver inputs.
 *
 * @param index Index of the phase pattern to apply.
 ******************************************************************************/
static void stepper_apply_phase(uint8_t index)
{
    if (phase[index][0U] != 0U)
    {
    	IN1_ON;
    }
    else
    {
    	IN1_OFF;
    }

    if (phase[index][1U] != 0U)
    {
    	IN2_ON;
    }
    else
    {
    	IN2_OFF;
    }

    if (phase[index][2U] != 0U)
    {
    	IN3_ON;
    }
    else
    {
    	IN3_OFF;
    }

    if (phase[index][3U] != 0U)
    {
        IN4_ON;
    }
    else
    {
        IN4_OFF;
    }
}

/******************************************************************************
 * @brief Configures the GPIO pins used for stepper motor control.
 ******************************************************************************/
void stepper_gpio_init(void)
{
    /* IN1: PA0 output */
    GPIOA->MODER &= ~(3U << (IN1_PIN * 2U));
    GPIOA->MODER |=  (1U << (IN1_PIN * 2U));

    /* IN2: PA1 output */
    GPIOA->MODER &= ~(3U << (IN2_PIN * 2U));
    GPIOA->MODER |=  (1U << (IN2_PIN * 2U));

    /* IN3: PA4 output */
    GPIOA->MODER &= ~(3U << (IN3_PIN * 2U));
    GPIOA->MODER |=  (1U << (IN3_PIN * 2U));

    /* IN4: PB0 output */
    GPIOB->MODER &= ~(3U << (IN4_PIN * 2U));
    GPIOB->MODER |=  (1U << (IN4_PIN * 2U));
}

/******************************************************************************
 * @brief Initializes the stepper motor driver state.
 ******************************************************************************/
void stepper_init(void)
{
    step_index = 0U;
    stepper_apply_phase(step_index);
}

/******************************************************************************
 * @brief Disables all stepper motor coils.
 ******************************************************************************/
void stepper_stop(void)
{
    IN1_AUS;
    IN2_AUS;
    IN3_AUS;
    IN4_AUS;
}

/******************************************************************************
 * @brief Moves the motor by one half-step in forward direction.
 ******************************************************************************/
void stepper_step_forward(void)
{
    step_index++;

    if (step_index >= STEPPER_PHASE_COUNT)
    {
        step_index = 0U;
    }

    stepper_apply_phase(step_index);
}

/******************************************************************************
 * @brief Moves the motor by one half-step in backward direction.
 ******************************************************************************/
void stepper_step_backward(void)
{
    if (step_index == 0U)
    {
        step_index = STEPPER_PHASE_COUNT - 1U;
    }
    else
    {
        step_index--;
    }

    stepper_apply_phase(step_index);
}

/******************************************************************************
 * @brief Moves the motor forward by a given number of half-steps.
 *
 * @param steps Number of half-steps.
 * @param delay_ms_per_step Delay between two half-steps in milliseconds.
 ******************************************************************************/
void stepper_move_forward(uint16_t steps, uint32_t delay_ms_per_step)
{
    for (uint16_t i = 0U; i < steps; i++)
    {
        stepForward();
        delay_ms(delay_ms_per_step);
    }
}

/******************************************************************************
 * @brief Moves the motor backward by a given number of half-steps.
 *
 * @param steps Number of half-steps.
 * @param delay_ms_per_step Delay between two half-steps in milliseconds.
 ******************************************************************************/
void stepper_move_backward(uint16_t steps, uint32_t delay_ms_per_step)
{
    for (uint16_t i = 0U; i < steps; i++)
    {
        stepBackward();
        delay_ms(delay_ms_per_step);
    }
}
