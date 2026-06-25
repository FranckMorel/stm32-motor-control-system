/******************************************************************************
 * @file    ui.h
 * @author  Morel
 * @brief   Public interface for the TFT-based user interface.
 ******************************************************************************/

#ifndef UI_H_
#define UI_H_

#include <stdint.h>

#define MAIN_MENU_COUNT    4U
#define MODE_MENU_COUNT    4U

typedef enum
{
    UI_MENU_MAIN = 0,
    UI_MENU_MODE,
    UI_MENU_STATUS
} UiMenu_t;

void UI_DrawMenu(void);
void UI_DrawStatus(void);
void UI_UpdateSelection(void);

void UI_NextItem(void);
void UI_PrevItem(void);
void UI_SelectItem(void);

#endif /* UI_H_ */
