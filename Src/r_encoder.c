/******************************************************************************
 * @file    r_encoder.c
 * @author  Morel
 * @brief   Rotary encoder driver for menu navigation.
 *
 * This module reads a mechanical rotary encoder with an integrated push button.
 * The encoder is used to navigate through the UI menu and to select menu items.
 *
 * Pin mapping:
 *  - CLK: PB3
 *  - DT : PA10
 *  - SW : PB5
 ******************************************************************************/

#include <stm32f401xe.h>
#include <ui.h>
#include <timer.h>

#define ENC_DT_PORT    GPIOA
#define ENC_DT_PIN     10U

#define ENC_CLK_PORT   GPIOB
#define ENC_CLK_PIN    3U

#define ENC_SW_PORT    GPIOB
#define ENC_SW_PIN     5U

#define ENCODER_DEBOUNCE_MS  200U

static uint8_t last_clk = 1U;
static uint8_t last_sw  = 1U;

/******************************************************************************
 * @brief Reads the logical state of a GPIO input pin.
 *
 * @param port GPIO port base address.
 * @param pin  GPIO pin number.
 *
 * @return 1 if the pin is high, 0 if the pin is low.
 ******************************************************************************/
static uint8_t read_pin(GPIO_TypeDef *port, uint8_t pin)
{
    return (uint8_t)((port->IDR >> pin) & 1U);
}

/******************************************************************************
 * @brief Configures the GPIO pins used by the rotary encoder.
 *
 * DT, CLK and SW are configured as input pins with internal pull-up resistors.
 * With this configuration, the idle state is high and an active button press
 * or encoder transition pulls the signal low.
 ******************************************************************************/
void encoder_gpio_init(void)
{
    /* DT: PA10 input with pull-up */
    ENC_DT_PORT->MODER &= ~(3U << (ENC_DT_PIN * 2U));
    ENC_DT_PORT->PUPDR &= ~(3U << (ENC_DT_PIN * 2U));
    ENC_DT_PORT->PUPDR |=  (1U << (ENC_DT_PIN * 2U));

    /* CLK: PB3 input with pull-up */
    ENC_CLK_PORT->MODER &= ~(3U << (ENC_CLK_PIN * 2U));
    ENC_CLK_PORT->PUPDR &= ~(3U << (ENC_CLK_PIN * 2U));
    ENC_CLK_PORT->PUPDR |=  (1U << (ENC_CLK_PIN * 2U));

    /* SW: PB5 input with pull-up */
    ENC_SW_PORT->MODER &= ~(3U << (ENC_SW_PIN * 2U));
    ENC_SW_PORT->PUPDR &= ~(3U << (ENC_SW_PIN * 2U));
    ENC_SW_PORT->PUPDR |=  (1U << (ENC_SW_PIN * 2U));
}

/******************************************************************************
 * @brief Initializes the encoder software state.
 *
 * The current pin states are stored to avoid detecting a false transition
 * immediately after startup.
 ******************************************************************************/
void encoder_init(void)
{
    last_clk = read_pin(ENC_CLK_PORT, ENC_CLK_PIN);
    last_sw  = read_pin(ENC_SW_PORT, ENC_SW_PIN);
}

/******************************************************************************
 * @brief Processes rotary encoder rotation events.
 *
 * A falling edge on CLK is used as the evaluation point. The DT signal is then
 * read to determine the rotation direction.
 *
 * Depending on the detected direction, the UI moves to the next or previous
 * menu item.
 ******************************************************************************/
void encoder_task(void)
{
    uint8_t clk_now = read_pin(ENC_CLK_PORT, ENC_CLK_PIN);
    uint8_t dt_now  = read_pin(ENC_DT_PORT, ENC_DT_PIN);

    if ((last_clk == 1U) && (clk_now == 0U))
    {
        if (dt_now == 1U)
        {
            UI_NextItem();
        }
        else
        {
            UI_PrevItem();
        }
    }

    last_clk = clk_now;
}

/******************************************************************************
 * @brief Processes the rotary encoder push button.
 *
 * The button is active-low because the input uses an internal pull-up resistor.
 * A simple time-based debounce filter prevents multiple detections caused by
 * mechanical bouncing.
 ******************************************************************************/
void encoder_button_task(void)
{
    static uint32_t last_press_time = 0U;

    uint8_t sw_now = read_pin(ENC_SW_PORT, ENC_SW_PIN);
    uint32_t now   = timer_ms();

    if ((last_sw == 1U) && (sw_now == 0U))
    {
        if ((now - last_press_time) > ENCODER_DEBOUNCE_MS)
        {
            last_press_time = now;
            UI_SelectItem();
        }
    }

    last_sw = sw_now;
}
