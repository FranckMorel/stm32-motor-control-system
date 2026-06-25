/******************************************************************************
 * @file    ui.c
 * @author  Morel
 * @brief   TFT-based user interface for the motor control system.
 *
 * This module implements a simple menu-driven user interface controlled by a
 * rotary encoder. It allows the user to start/stop the motor, select motor
 * modes and display the current motor status.
 ******************************************************************************/

#include "ui.h"
#include "tft.h"
#include "motor_control.h"
#include <stdint.h>

#define UI_TITLE_X          20U
#define UI_TITLE_Y          10U

#define UI_CURSOR_X         10U
#define UI_ITEM_X           25U
#define UI_FIRST_ITEM_Y     40U
#define UI_ITEM_SPACING_Y   20U

#define UI_STATUS_LABEL_X   10U
#define UI_STATUS_VALUE_X   60U

static UiMenu_t currentMenu = UI_MENU_MAIN;
static uint8_t currentItem = 0U;
static uint8_t previousItem = 0U;

static const char* mainMenu_items[] =
{
    "Start",
    "Stop",
    "Status",
    "Mode"
};

static const char* modeMenu_items[] =
{
    "ECO",
    "NORMAL",
    "FAST",
    "Back"
};

/******************************************************************************
 * @brief Returns the item list and item count of the active menu.
 *
 * @param items Pointer to the active menu item list.
 * @param count Pointer to the number of items in the active menu.
 ******************************************************************************/
static void UI_getCurrentMenuData(const char*** items, uint8_t* count)
{
    if (currentMenu == UI_MENU_MAIN)
    {
        *items = mainMenu_items;
        *count = MAIN_MENU_COUNT;
    }
    else if (currentMenu == UI_MENU_MODE)
    {
        *items = modeMenu_items;
        *count = MODE_MENU_COUNT;
    }
    else
    {
        *items = modeMenu_items;
        *count = 1U;
    }
}

/******************************************************************************
 * @brief Draws the currently active menu.
 ******************************************************************************/
void UI_DrawMenu(void)
{
    const char** items = 0;
    uint8_t count = 0U;
    const char* title = "";

    tft_fill_screen(TFT_BLACK);

    if (currentMenu == UI_MENU_MAIN)
    {
        items = mainMenu_items;
        count = MAIN_MENU_COUNT;
        title = "Motor Control";
    }
    else if (currentMenu == UI_MENU_MODE)
    {
        items = modeMenu_items;
        count = MODE_MENU_COUNT;
        title = "Select Mode";
    }

    tft_draw_string(UI_TITLE_X, UI_TITLE_Y, title, TFT_WHITE, TFT_BLACK);

    for (uint8_t i = 0U; i < count; i++)
    {
        uint16_t color = TFT_WHITE;
        uint16_t y = UI_FIRST_ITEM_Y + (i * UI_ITEM_SPACING_Y);

        if (i == currentItem)
        {
            tft_draw_char(UI_CURSOR_X, y, '>', TFT_YELLOW, TFT_BLACK);
            color = TFT_YELLOW;
        }
        else
        {
            tft_draw_char(UI_CURSOR_X, y, ' ', TFT_WHITE, TFT_BLACK);
        }

        tft_draw_string(UI_ITEM_X, y, items[i], color, TFT_BLACK);
    }
}

/******************************************************************************
 * @brief Draws the motor status screen.
 ******************************************************************************/
void UI_DrawStatus(void)
{
    tft_fill_screen(TFT_BLACK);

    tft_draw_string(UI_TITLE_X, UI_TITLE_Y, "Status", TFT_WHITE, TFT_BLACK);

    tft_draw_string(UI_STATUS_LABEL_X, 40U, "Mode:", TFT_WHITE, TFT_BLACK);
    tft_draw_string(UI_STATUS_VALUE_X, 40U, MotorControl_GetModeString(), TFT_YELLOW, TFT_BLACK);

    tft_draw_string(UI_STATUS_LABEL_X, 60U, "State:", TFT_WHITE, TFT_BLACK);
    tft_draw_string(UI_STATUS_VALUE_X, 60U, MotorControl_GetStateString(), TFT_YELLOW, TFT_BLACK);

    tft_draw_string(UI_STATUS_LABEL_X, 80U, "Dir:", TFT_WHITE, TFT_BLACK);
    tft_draw_string(UI_STATUS_VALUE_X, 80U, MotorControl_GetDirectionString(), TFT_YELLOW, TFT_BLACK);

    tft_draw_char(5U, 110U, '>', TFT_YELLOW, TFT_BLACK);
    tft_draw_string(20U, 110U, "Back", TFT_YELLOW, TFT_BLACK);
}

/******************************************************************************
 * @brief Updates only the menu selection highlight.
 *
 * This avoids redrawing the whole menu when the encoder moves.
 ******************************************************************************/
void UI_UpdateSelection(void)
{
    const char** items = 0;
    uint8_t count = 0U;

    UI_getCurrentMenuData(&items, &count);

    if ((previousItem >= count) || (currentItem >= count))
    {
        return;
    }

    uint16_t oldY = UI_FIRST_ITEM_Y + (previousItem * UI_ITEM_SPACING_Y);
    uint16_t newY = UI_FIRST_ITEM_Y + (currentItem * UI_ITEM_SPACING_Y);

    tft_draw_char(UI_CURSOR_X, oldY, ' ', TFT_WHITE, TFT_BLACK);
    tft_draw_string(UI_ITEM_X, oldY, items[previousItem], TFT_WHITE, TFT_BLACK);

    tft_draw_char(UI_CURSOR_X, newY, '>', TFT_YELLOW, TFT_BLACK);
    tft_draw_string(UI_ITEM_X, newY, items[currentItem], TFT_YELLOW, TFT_BLACK);
}

/******************************************************************************
 * @brief Moves the menu cursor to the next item.
 ******************************************************************************/
void UI_NextItem(void)
{
    const char** items = 0;
    uint8_t count = 0U;

    UI_getCurrentMenuData(&items, &count);

    previousItem = currentItem;
    currentItem++;

    if (currentItem >= count)
    {
        currentItem = 0U;
    }

    UI_UpdateSelection();
}

/******************************************************************************
 * @brief Moves the menu cursor to the previous item.
 ******************************************************************************/
void UI_PrevItem(void)
{
    const char** items = 0;
    uint8_t count = 0U;

    UI_getCurrentMenuData(&items, &count);

    previousItem = currentItem;

    if (currentItem == 0U)
    {
        currentItem = count - 1U;
    }
    else
    {
        currentItem--;
    }

    UI_UpdateSelection();
}

/******************************************************************************
 * @brief Executes the action associated with the currently selected item.
 ******************************************************************************/
void UI_SelectItem(void)
{
    if (currentMenu == UI_MENU_MAIN)
    {
        if (currentItem == 0U)
        {
            MotorControl_RunUserMode();
        }
        else if (currentItem == 1U)
        {
            MotorControl_Stop();
        }
        else if (currentItem == 2U)
        {
            currentMenu = UI_MENU_STATUS;
            currentItem = 0U;
            UI_DrawStatus();
        }
        else if (currentItem == 3U)
        {
            currentMenu = UI_MENU_MODE;
            currentItem = 0U;
            UI_DrawMenu();
        }
    }
    else if (currentMenu == UI_MENU_MODE)
    {
        if (currentItem == 0U)
        {
            MotorControl_SetMode(MODE_ECO);
        }
        else if (currentItem == 1U)
        {
            MotorControl_SetMode(MODE_NORMAL);
        }
        else if (currentItem == 2U)
        {
            MotorControl_SetMode(MODE_FAST);
        }
        else if (currentItem == 3U)
        {
            currentMenu = UI_MENU_MAIN;
            currentItem = 0U;
        }

        UI_DrawMenu();
    }
    else if (currentMenu == UI_MENU_STATUS)
    {
        currentMenu = UI_MENU_MAIN;
        currentItem = 0U;
        UI_DrawMenu();
    }
}




/*Draft

#include "ui.h"
#include "tft.h"
#include "motor_control.h"
#include <stdint.h>


static UiMenu_t currentMenu = UI_MENU_MAIN;
static uint8_t currentItem = 0;
static uint8_t previousItem = 0;


const char* mainMenu_items[] = {
        "Start",
        "Stop",
        "Status",
        "Mode"
  };

const char* modeMenu_items[] = {
        "ECO",
        "NORMAL",
        "FAST",
		"Back"
  };

const char* statusMenu_items[] = {
        "Mode: ",
        "State: ",
        "Direction: ",
		"Back"
  };


static void UI_getCurrentMenuData(const char*** items, uint8_t* count){

	if(currentMenu == UI_MENU_MAIN){
		*items = mainMenu_items;
		*count = MAIN_MENU_COUNT;
	}
	else if(currentMenu == UI_MENU_MODE){
		*items = modeMenu_items;
		*count = MODE_MENU_COUNT;

	}
	else if(currentMenu == UI_MENU_STATUS){
		*items = statusMenu_items;
		*count = STATUS_MENU_COUNT;

	}

}


//void UI_init(void){}

void UI_DrawMenu(void)
{
    tft_testFullScreenColor(TFT_BLACK);

    const char** items = 0;
    uint8_t count = 0;
    const char* title = "";

    if(currentMenu == UI_MENU_MAIN)
    {
        items = mainMenu_items;
        count = MAIN_MENU_COUNT;
        title = "Motor Control";
    }
    else if(currentMenu == UI_MENU_MODE)
    {
        items = modeMenu_items;
        count = MODE_MENU_COUNT;
        title = "Select Mode";
    }
    else if(currentMenu == UI_MENU_STATUS)
    {
        items = statusMenu_items;
        count = STATUS_MENU_COUNT;
        title = "Status";
    }

    tft_draw_string(20, 10, title, TFT_WHITE, TFT_BLACK);

    for(uint8_t i = 0; i < count; i++)
    {
        uint16_t color = TFT_WHITE;

        if(i == currentItem)
        {
            tft_draw_char(10, 40 + i * 20, '>', TFT_YELLOW, TFT_BLACK);
            color = TFT_YELLOW;
        }
        else
        {
            tft_draw_char(10, 40 + i * 20, ' ', TFT_WHITE, TFT_BLACK);
        }

        tft_draw_string(25, 40 + i * 20, items[i], color, TFT_BLACK);
    }
}


void UI_DrawStatus(void)
{
    tft_testFullScreenColor(TFT_BLACK);

    tft_draw_string(20, 10, "Status", TFT_WHITE, TFT_BLACK);

    tft_draw_string(10, 40, "Mode:", TFT_WHITE, TFT_BLACK);
    tft_draw_string(60, 40, MotorControl_GetModeString(), TFT_YELLOW, TFT_BLACK);

    tft_draw_string(10, 60, "State:", TFT_WHITE, TFT_BLACK);
    tft_draw_string(60, 60, MotorControl_GetStateString(), TFT_YELLOW, TFT_BLACK);

    tft_draw_string(10, 80, "Dir:", TFT_WHITE, TFT_BLACK);
    tft_draw_string(60, 80, MotorControl_GetDirectionString(), TFT_YELLOW, TFT_BLACK);


    tft_draw_char(5, 110, '>', TFT_YELLOW, TFT_BLACK);
    tft_draw_string(20, 110, "Back", TFT_YELLOW, TFT_BLACK);

}

void UI_UpdateSelection(void)
{
    const char** items = 0;
    uint8_t count = 0;

    UI_getCurrentMenuData(&items, &count);

    if(previousItem >= count || currentItem >= count)
        return;

    uint16_t oldY = 40 + previousItem * 20;
    uint16_t newY = 40 + currentItem * 20;

    // alte Zeile wird normal gezichnet
    tft_draw_char(10, oldY, ' ', TFT_WHITE, TFT_BLACK);
    tft_draw_string(25, oldY,items[previousItem],TFT_WHITE,TFT_BLACK);

    // neue Zeile wird markiert
    tft_draw_char(10, newY, '>', TFT_YELLOW, TFT_BLACK);
    tft_draw_string(25, newY,items[currentItem], TFT_YELLOW,TFT_BLACK);

}


void UI_NextItem(void){
    const char** items = 0;
    uint8_t count = 0;

    UI_getCurrentMenuData(&items, &count);

    previousItem = currentItem;
    currentItem++;

    if(currentItem >= count)
        currentItem = 0;

    UI_UpdateSelection();
}


void UI_PrevItem(void){
    const char** items = 0;
    uint8_t count = 0;

    UI_getCurrentMenuData(&items, &count);

    previousItem = currentItem;

	    if(currentItem == 0)
	 	    currentItem = count - 1;
	    else
	    	currentItem--;

	UI_UpdateSelection();

}

void UI_SelectItem(void){

	 if(currentMenu == UI_MENU_MAIN){
		 if(currentItem == 0){
			 MotorControl_RunUserMode();

		 }
		 else if(currentItem == 1){
			 MotorControl_Stop();

		 }
		 else if(currentItem == 2){
			 currentMenu = UI_MENU_STATUS;
			 currentItem = 0;
			 UI_DrawStatus();
		 }
		 else if(currentItem == 3){
			 currentMenu = UI_MENU_MODE;
			 currentItem = 0;
			 UI_DrawMenu();
		 }

	 }

	 else if(currentMenu == UI_MENU_MODE){
		 if(currentItem == 0){
			 MotorControl_SetMode(MODE_ECO);
		 }
		 else if(currentItem == 1) {
			 MotorControl_SetMode(MODE_NORMAL);
		 }
		 else if (currentItem == 2){
			 MotorControl_SetMode(MODE_FAST);
		 }
		 else if(currentItem == 3) {  // "Back"
		        currentMenu = UI_MENU_MAIN;
		        currentItem = 0;
		    }

		 UI_DrawMenu();

	 }

	 else if(currentMenu == UI_MENU_STATUS){

		 currentMenu = UI_MENU_MAIN;
		 currentItem = 0;
		 UI_DrawMenu();

	 }
}

*/
