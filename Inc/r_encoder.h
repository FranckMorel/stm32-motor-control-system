/******************************************************************************
 * @file    r_encoder.h
 * @author  Morel
 * @brief   Public interface for the rotary encoder driver.
 ******************************************************************************/

#ifndef R_ENCODER_H_
#define R_ENCODER_H_

/******************************************************************************
 * @brief Configures the GPIO pins used by the rotary encoder.
 ******************************************************************************/
void encoder_gpio_init(void);

/******************************************************************************
 * @brief Initializes the encoder software state.
 ******************************************************************************/
void encoder_init(void);

/******************************************************************************
 * @brief Processes rotary encoder rotation events.
 *
 * This function should be called periodically from the main loop.
 ******************************************************************************/
void encoder_task(void);

/******************************************************************************
 * @brief Processes the rotary encoder push button.
 *
 * This function should be called periodically from the main loop.
 ******************************************************************************/
void encoder_button_task(void);

#endif /* R_ENCODER_H_ */
