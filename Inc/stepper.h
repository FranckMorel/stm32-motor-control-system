/******************************************************************************
 * @file    stepper.h
 * @author  Morel
 * @brief   Public interface for the stepper motor driver.
 ******************************************************************************/

#ifndef STEPPER_H_
#define STEPPER_H_

#include <stdint.h>

void stepper_gpio_init(void);
void stepper_init(void);
void stepper_stop(void);

void stepper_step_forward(void);
void stepper_step_backward(void);

void stepper_move_forward(uint16_t steps, uint32_t delay_ms_per_step);
void stepper_move_backward(uint16_t steps, uint32_t delay_ms_per_step);

#endif /* STEPPER_H_ */
