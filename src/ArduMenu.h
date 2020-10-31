#pragma once

#include "Arduino.h"

struct MENU_ITEM
{
  byte Type;
  const char *Text;
  void (*Function)(MENU_ITEM *);
  MENU_ITEM *SubItems;
  int16_t (*rangeManage)(int8_t);
  bool (*toggleManage)(bool);
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
#define SCREEN_LINES_BELOW_TEXT 4
#define SCREEN_LINES_TOTAL 6
#define SCREEN_COLUMNS 14
// Those are for avoid calculations
#define SCREEN_LETTER_W 6
#define SCREEN_LETTER_H 8
#define SCREEN_BOX_AREA 0.9
#endif

class ArduMenu {
  public:
    // Init function
    #ifdef _ADAFRUIT_PCD8544_H
    ArduMenu(MENU_ITEM *menu, Adafruit_PCD8544 display);
    #endif

    // Public variables
    bool inRange;

    // Public functions
    void drawMenu();
    void begin();
    void down();
    void down(int16_t, int16_t);
    void up();
    void up(int16_t, int16_t);
    void enter();
  
  protected:
    // Protected variables
    uint8_t _currentMenuItemIdx;
    int8_t _itemsOffset;
    uint8_t _lines;
    MENU_ITEM *_currentMenuTable;
    MENU_ITEM *_oldMenuTable;
    bool _haveBox;
    uint8_t _boxWidth;
    uint8_t _boxHeight;
    uint8_t _boxXMargin;
    uint8_t _boxYMargin;
    uint8_t _boxLines;
    uint8_t _boxLinesYMargin;
    uint8_t _boxColumns;
    uint8_t _boxColumnsXMargin;
    uint8_t _toggleMargin;
    uint16_t _toggleX;
    uint8_t _toggleWH;
    #ifdef _ADAFRUIT_PCD8544_H
    Adafruit_PCD8544 _display;
    #endif

    // Protected functions
    char * _centerText(int);
    char * _centerText(int, uint8_t);
    char * _centerText(const char *);
    char * _centerText(const char *, uint8_t);
    void _setRangeCurrent(uint16_t);
    void _setRangeMetter(uint8_t);
    void _reDraw();
};