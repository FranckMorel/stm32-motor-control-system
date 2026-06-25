/******************************************************************************
 * @file    tft.c
 * @author  Morel
 * @brief   ST7735S TFT display driver.
 *
 * This module provides a blocking SPI-based driver for the ST7735S TFT display.
 * It supports display initialization, window selection, pixel color transfer,
 * character rendering and string rendering using an 8x16 bitmap font.
 *
 * Pin mapping:
 *  - PA8: TFT reset pin
 *  - PA6: TFT data/command pin
 *  - PA9: TFT chip select pin
 ******************************************************************************/

#include "stm32f401xe.h"
#include "tft.h"
#include "spi.h"
#include "timer.h"
#include "font.h"

#define TFT_DC_PIN              6U
#define TFT_CS_PIN              9U
#define TFT_RST_PIN             8U

#define TFT_CHAR_WIDTH          8U
#define TFT_CHAR_HEIGHT         16U

#define TFT_X_START             0U
#define TFT_Y_START             0U
#define TFT_X_END               (TFT_WIDTH - 1U)
#define TFT_Y_END               (TFT_HEIGHT - 1U)

#define GPIO_MODE_OUTPUT        1U

#define DC_HIGH()               (GPIOA->BSRR = (1U << TFT_DC_PIN))
#define DC_LOW()                (GPIOA->BSRR = (1U << (TFT_DC_PIN + 16U)))

#define RST_HIGH()              (GPIOA->BSRR = (1U << TFT_RST_PIN))
#define RST_LOW()               (GPIOA->BSRR = (1U << (TFT_RST_PIN + 16U)))

#define CS_HIGH()               (GPIOA->BSRR = (1U << TFT_CS_PIN))
#define CS_LOW()                (GPIOA->BSRR = (1U << (TFT_CS_PIN + 16U)))

/******************************************************************************
 * @brief Configures GPIO pins used by the TFT display.
 *
 * The reset, data/command and chip-select pins are configured as push-pull
 * outputs. The GPIOA clock is enabled at board level before this function is
 * called.
 ******************************************************************************/
void tft_gpio_init(void)
{
    /* PA8: Reset output */
    GPIOA->MODER &= ~(3U << (TFT_RST_PIN * 2U));
    GPIOA->MODER |=  (GPIO_MODE_OUTPUT << (TFT_RST_PIN * 2U));

    /* PA6: Data/Command output */
    GPIOA->MODER &= ~(3U << (TFT_DC_PIN * 2U));
    GPIOA->MODER |=  (GPIO_MODE_OUTPUT << (TFT_DC_PIN * 2U));

    /* PA9: Chip Select output */
    GPIOA->MODER &= ~(3U << (TFT_CS_PIN * 2U));
    GPIOA->MODER |=  (GPIO_MODE_OUTPUT << (TFT_CS_PIN * 2U));

    CS_HIGH();
    DC_HIGH();
    RST_HIGH();
}

/******************************************************************************
 * @brief Enables TFT chip select.
 ******************************************************************************/
void tft_cs_enable(void)
{
    CS_LOW();
}

/******************************************************************************
 * @brief Disables TFT chip select.
 ******************************************************************************/
void tft_cs_disable(void)
{
    CS_HIGH();
}

/******************************************************************************
 * @brief Sends a command byte to the TFT controller.
 *
 * The ST7735S uses DC = 0 for command bytes.
 *
 * @param cmd Command byte.
 ******************************************************************************/
void tft_write_cmd(uint8_t cmd)
{
    DC_LOW();
    tft_cs_enable();
    spi1_transmit(&cmd, 1U);
    tft_cs_disable();
}

/******************************************************************************
 * @brief Sends a data byte to the TFT controller.
 *
 * The ST7735S uses DC = 1 for display data.
 *
 * @param data Data byte.
 ******************************************************************************/
void tft_write_data(uint8_t data)
{
    DC_HIGH();
    tft_cs_enable();
    spi1_transmit(&data, 1U);
    tft_cs_disable();
}

/******************************************************************************
 * @brief Sends one 16-bit RGB565 color value to the TFT controller.
 *
 * The ST7735S receives 16-bit color data as two consecutive bytes,
 * high byte first.
 *
 * @param data RGB565 color value.
 ******************************************************************************/
void tft_write_data16(uint16_t data)
{
    uint8_t buffer[2];

    buffer[0] = (uint8_t)(data >> 8U);
    buffer[1] = (uint8_t)(data & 0xFFU);

    DC_HIGH();
    tft_cs_enable();
    spi1_transmit(buffer, 2U);
    tft_cs_disable();
}

/******************************************************************************
 * @brief Performs a hardware reset of the TFT display.
 *
 * According to the ST7735S reset timing requirements, the reset line must be
 * held low before returning to the high state.
 ******************************************************************************/
void tft_reset(void)
{
    RST_LOW();
    delay_ms(10U);

    RST_HIGH();
    delay_ms(120U);
}

/******************************************************************************
 * @brief Initializes the ST7735S display controller.
 ******************************************************************************/
void tft_init(void)
{
    spi1_config();

    tft_reset();

    tft_write_cmd(SWRESET);
    delay_ms(150U);

    tft_write_cmd(SLPOUT);
    delay_ms(120U);

    tft_write_cmd(COLMOD);
    tft_write_data(0x05U);      /* 16-bit RGB565 color mode */

    tft_write_cmd(MADCTL);
    tft_write_data(0xC0U);      /* Display orientation / RGB order */

    tft_write_cmd(DISPON);
    delay_ms(100U);
}

/******************************************************************************
 * @brief Defines the active drawing window.
 *
 * After setting the column and row address range, the display controller enters
 * RAM write mode. Subsequent data bytes are written as pixel data into this
 * window.
 *
 * @param x1 Start column.
 * @param y1 Start row.
 * @param x2 End column.
 * @param y2 End row.
 ******************************************************************************/
void tft_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    tft_write_cmd(CASET);
    tft_write_data((uint8_t)(x1 >> 8U));
    tft_write_data((uint8_t)(x1 & 0xFFU));
    tft_write_data((uint8_t)(x2 >> 8U));
    tft_write_data((uint8_t)(x2 & 0xFFU));

    tft_write_cmd(RASET);
    tft_write_data((uint8_t)(y1 >> 8U));
    tft_write_data((uint8_t)(y1 & 0xFFU));
    tft_write_data((uint8_t)(y2 >> 8U));
    tft_write_data((uint8_t)(y2 & 0xFFU));

    tft_write_cmd(RAMWR);
}

/******************************************************************************
 * @brief Draws one ASCII character using the 8x16 bitmap font.
 *
 * @param x  X position of the upper-left corner.
 * @param y  Y position of the upper-left corner.
 * @param ch ASCII character from 32 to 126.
 * @param fg Foreground color in RGB565 format.
 * @param bg Background color in RGB565 format.
 ******************************************************************************/
void tft_draw_char(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg)
{
    if ((ch < FONT_ASCII_FIRST) || (ch > FONT_ASCII_LAST))
    {
        return;
    }

    const uint8_t *data = &Font8x16[(uint8_t)ch - FONT_ASCII_FIRST][0];

    tft_set_window(x,
                   y,
                   x + TFT_CHAR_WIDTH - 1U,
                   y + TFT_CHAR_HEIGHT - 1U);

    for (uint8_t row = 0U; row < TFT_CHAR_HEIGHT; row++)
    {
        uint8_t row_data = data[row];

        for (uint8_t col = 0U; col < TFT_CHAR_WIDTH; col++)
        {
            if ((row_data & 0x80U) != 0U)
            {
                tft_write_data16(fg);
            }
            else
            {
                tft_write_data16(bg);
            }

            row_data <<= 1U;
        }
    }
}

/******************************************************************************
 * @brief Draws a null-terminated string on the TFT display.
 *
 * Text automatically wraps to the next line when the right display edge is
 * reached.
 *
 * @param x   X start position.
 * @param y   Y start position.
 * @param str Pointer to a null-terminated string.
 * @param fg  Foreground color in RGB565 format.
 * @param bg  Background color in RGB565 format.
 ******************************************************************************/
void tft_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg)
{
    while (*str != '\0')
    {
        if (x > (TFT_WIDTH - TFT_CHAR_WIDTH))
        {
            x = 0U;
            y += TFT_CHAR_HEIGHT;
        }

        tft_draw_char(x, y, *str, fg, bg);

        x += TFT_CHAR_WIDTH;
        str++;
    }
}

/******************************************************************************
 * @brief Fills the entire display with one color.
 *
 * @param color Fill color in RGB565 format.
 ******************************************************************************/
void tft_fill_screen(uint16_t color)
{
    tft_set_window(TFT_X_START, TFT_Y_START, TFT_X_END, TFT_Y_END);

    for (uint32_t i = 0U; i < (TFT_WIDTH * TFT_HEIGHT); i++)
    {
        tft_write_data16(color);
    }
}




/*Draft


// PA8 -> RST , OUTPUT PIN
// PA6 -> DC , OUTPUT PIN
// PA9 -> CS , OUTPUT PIN


#include "stm32f401xe.h"
#include "tft.h"
#include "spi.h"
#include "timer.h"
#include "font.h"


#define DC_PIN     	6
#define CS_PIN     	9
#define RST_PIN  	8

#define DC_HIGH()	(GPIOA -> BSRR = (1U<<6))
#define DC_LOW()	(GPIOA -> BSRR = (1U<<(DC_PIN + 16)))

#define RST_HIGH()	(GPIOA -> BSRR = (1U<<8))
#define RST_LOW()	(GPIOA -> BSRR = (1U<<(RST_PIN + 16)))


void tft_gpio_init(void){

	//set PA8 as an OUTPUT Pin
	GPIOA->MODER &= ~(3U << (RST_PIN*2));
	GPIOA->MODER |= (1U << (RST_PIN*2));

	//set PA6 as an OUTPUT Pin
	GPIOA->MODER &= ~(3U << (DC_PIN*2));
	GPIOA->MODER |= (1U << (DC_PIN*2));

	//set PA9 as an OUTPUT Pin
	GPIOA->MODER &= ~(3U << (CS_PIN*2));
	GPIOA->MODER |= (1U << (CS_PIN*2));

}

void tft_cs_enable(void){

	// CS enable by setting Pin 9 to LOW
	GPIOA -> BSRR = (1U<<25);
}

void tft_cs_disable(void){

	// CS disable by setting pin 9 to HIGH
	GPIOA -> BSRR = (1U<<9);
}

// s24 -> ST7735S Datasheet v1.1: DC LOW for cmd and DC HIGH for data
void tft_write_cmd(uint8_t cmd){

	DC_LOW();
	tft_cs_enable();
	spi1_transmit(&cmd,1);
	tft_cs_disable();

}

void tft_write_data(uint8_t data){

	DC_HIGH();
	tft_cs_enable();
	spi1_transmit(&data,1);
	tft_cs_disable();

}


void tft_write_data16(uint16_t data){
    uint8_t high = (data >> 8); 	 // obere 8 Bits (z.B. 0xF8)
    uint8_t low  = (data & 0xFF);    // untere 8 Bits (z.B. 0x00)
    tft_write_data(high);
    tft_write_data(low);
}

// s93 -> ST7735S Datasheet v1.1: RST muss mindestens 10us LOW sein
void tft_reset(void){

	RST_LOW();
    delay_ms(10);

    RST_HIGH();
    delay_ms(120);
}

void tft_init(){

	spi1_config();

    tft_reset();
    tft_write_cmd(SWRESET);
    delay_ms(150);			 // s108 discription fpr SWRESET
    tft_write_cmd(SLPOUT);
    delay_ms(120);			 // s120 Sleepout cmd
    tft_write_cmd(COLMOD);
    tft_write_data(0x05);    // s150, 16-bit RGB565
    tft_write_cmd(MADCTL);
    tft_write_data(0xC0);    // BGR Modus, alle anderen Bits werden überschrieben aber ok , weil nicht gebraucht werden
    tft_write_cmd(DISPON);
    delay_ms(100);
}


/*
void tft_test1(void) {
     // Daten werden als 16 Bits Werte geschrieben, also in 8Bits Paare aufgespittet
     // s128,s130 : CASET und RASET definieren den Schreibbereich

    tft_write_cmd(CASET);
    tft_write_data(0x00); tft_write_data(0x00); // Start bei 0
    tft_write_data(0x00); tft_write_data(0x83); // Ende bei 131

    tft_write_cmd(RASET);
    tft_write_data(0x00); tft_write_data(0x00); // Start bei 0
    tft_write_data(0x00); tft_write_data(0xA1); // Ende bei 161

    tft_write_cmd(RAMWR); // versetzt Display in den Pixel Schreib Modus

    for(uint32_t i = 0; i < 132 * 162; i++) {
        tft_write_data(0x00);  // Rot High
        tft_write_data(0x1F);  // Rot Low
    }
}
*/

/*
void tft_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    tft_write_cmd(CASET);
    // Koordinaten in 8 Bits Paare aufgesplittet:
    tft_write_data(x1 >> 8); tft_write_data(x1 & 0xFF);
    tft_write_data(x2 >> 8); tft_write_data(x2 & 0xFF);

    tft_write_cmd(RASET);
    tft_write_data(y1 >> 8); tft_write_data(y1 & 0xFF);
    tft_write_data(y2 >> 8); tft_write_data(y2 & 0xFF);

    tft_write_cmd(RAMWR);
}

/* Zeichen zeichnen:
   @param x, y: Position (oben links)
   @param ch: ASCII Zeichen (32-127)
   @param fg: Vordergrundfarbe (das Zeichen selbst)
   @param bg: Hintergrundfarbe (die Fläche um das Zeichen)
 */

/*
void tft_draw_char(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg) {
    if(ch < 32 || ch > 127) return; // ungueltige Zeichen filtern

    const uint8_t *data = &Font8x16[ch - 32][0];
    tft_set_window(x, y, x + 7, y + 15); // Ein Zeichen ist 8Pixel breit und 16Pixel hoch

    for(int row = 0; row < 16; row++) {
        uint8_t row_data = data[row];
        for(int col = 0; col < 8; col++) {
            if(row_data & 0x80) { // 0x80 = 10000000 -> wenn Bit7 ist 1 -> Bedingung erfüllt || MSB fisrt
                tft_write_data16(fg); // Print mit eingestellter Zeichenfarbe
            } else {
                tft_write_data16(bg); // print mit Hintergrundfarbe
            }
            row_data <<= 1;
        }
    }
}

void tft_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg) {
    while(*str) {
        if(x > TFT_WIDTH - 8) {
            x = 0;
            y += 16;
        }
        tft_draw_char(x, y, *str, fg, bg);
        x += 8;
        str++;
    }
}


void tft_testFullScreenColor(uint16_t color){

	tft_set_window(0,0,131,161);

	for(uint32_t i = 0; i < 132 * 162; i++) {

		tft_write_data16(color);

	    }

}
*/




