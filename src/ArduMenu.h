#pragma once

#include "Arduino.h"

struct MENU_ITEM
{
  byte Type;
  const char *Text;
  void (*Function)(MENU_ITEM *);
  MENU_ITEM *SubItems;
  int rangeMin;
  int rangeMax;
  int rangeSteps;
  int (*getRangeStatus)();
  void (*setRangeStatus)(int);
  int (*getToggleStatus)();
  void (*setToggleStatus)(int);
};

// Menu items types definitions
#define AM_ITEM_TYPE_HEADER 0
#define AM_ITEM_TYPE_MENU 1
#define AM_ITEM_TYPE_COMMAND 2
#define AM_ITEM_TYPE_RANGE 3
#define AM_ITEM_TYPE_TOGGLE 4
#define AM_ITEM_TYPE_EOM 5

// Screen types and values defintions
#ifdef _ADAFRUIT_PCD8544_H
#define SCREEN_WIDTH 84
#define SCREEN_HEIGHT 48
#define SCREEN_LINES_BELOW_TEXT 4
#define SCREEN_LINES_TOTAL 6
#define SCREEN_COLUMNS 14
// Those are for avoid calculations
#define SCREEN_LETTER_W 6
#define SCREEN_LETTER_H 8
#endif

class ArduMenu {
  public:
    // Init function
    #ifdef _ADAFRUIT_PCD8544_H
    ArduMenu(MENU_ITEM *menu, Adafruit_PCD8544 display);
    #endif

    void drawMenu();
    void begin();
    void down();
    void up();
    void enter();
  
  protected:
    // Variables
    byte _currentMenuItemIdx;
    byte _itemsOffset;
    byte _lines;
    bool _inRange;
    MENU_ITEM *_currentMenuTable;
    MENU_ITEM *_oldMenuTable;
    #ifdef _ADAFRUIT_PCD8544_H
    Adafruit_PCD8544 _display;
    #endif

    // Functions
    char * centerText(const char *text, unsigned int length);
    char * centerText(const char *text);
};