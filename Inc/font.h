#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>

#define FONT_WIDTH        8U
#define FONT_HEIGHT       16U
#define FONT_ASCII_FIRST  32U
#define FONT_ASCII_LAST   126U
#define FONT_ASCII_COUNT  96U

extern const uint8_t Font8x16[FONT_ASCII_COUNT][FONT_HEIGHT];

#endif /* FONT_H_ */
