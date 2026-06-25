/******************************************************************************
 * @file    board_gpio.c
 * @author  Morel
 * @brief   Board-level GPIO initialization.
 *
 * This module enables the GPIO port clocks used by the application and
 * initializes the GPIO configuration of all connected peripherals.
 ******************************************************************************/

#include <r_encoder.h>
#include <spi.h>
#include <tft.h>
#include <stepper.h>

/******************************************************************************
 * @brief Enables GPIO port clocks used by the board.
 *
 * GPIO clocks must be enabled before configuring any GPIO registers.
 * Ports A, B and C are used by the SPI interface, TFT display,
 * stepper motor driver and rotary encoder.
 ******************************************************************************/
static void enable_gpio_clocks(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN;

    /*
     * Ensure that the clock enable writes are completed before accessing
     * GPIO peripheral registers.
     */
    __DSB();
}

/******************************************************************************
 * @brief Initializes all GPIO peripherals used by the board.
 *
 * This function provides the central board-level GPIO initialization sequence.
 * It first enables the required GPIO port clocks and then delegates the
 * individual pin configuration to the corresponding peripheral modules.
 ******************************************************************************/
void board_gpio_init(void)
{
    enable_gpio_clocks();

    spi_gpio_init();
    tft_gpio_init();
    stepper_gpio_init();
    encoder_gpio_init();
}
