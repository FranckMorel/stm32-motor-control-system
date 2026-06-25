/******************************************************************************
 * @file    tft.h
 * @author  Morel
 * @brief   Public interface for the ST7735S TFT display driver.
 ******************************************************************************/

#ifndef TFT_H_
#define TFT_H_

#include <stdint.h>

/* ST7735S command set */
#define SWRESET     0x01U   /* Software reset */
#define SLPOUT      0x11U   /* Exit sleep mode */
#define COLMOD      0x3AU   /* Set color mode */
#define MADCTL      0x36U   /* Memory access control */
#define DISPON      0x29U   /* Display on */
#define CASET       0x2AU   /* Column address set */
#define RASET       0x2BU   /* Row address set */
#define RAMWR       0x2CU   /* Memory write */

/* Display resolution */
#define TFT_WIDTH   132U
#define TFT_HEIGHT  162U

/* RGB565 color definitions */
#define TFT_BLACK   0x0000U
#define TFT_WHITE   0xFFFFU
#define TFT_RED     0xF800U
#define TFT_GREEN   0x07E0U
#define TFT_BLUE    0x001FU
#define TFT_YELLOW  0xFFE0U

void tft_gpio_init(void);
void tft_cs_enable(void);
void tft_cs_disable(void);

void tft_write_cmd(uint8_t cmd);
void tft_write_data(uint8_t data);
void tft_write_data16(uint16_t data);

void tft_reset(void);
void tft_init(void);

void tft_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void tft_draw_char(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg);
void tft_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg);
void tft_fill_screen(uint16_t color);

#endif /* TFT_H_ */
