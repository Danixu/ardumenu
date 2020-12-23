#pragma once

#include "Arduino.h"

struct MENU_ITEM
{
  byte type;
  const char *text;
  bool (*function)(MENU_ITEM *);
  MENU_ITEM *subItems;
  int16_t (*rangeManage)(int16_t);
  bool (*toggleManage)(bool);
  bool disabled;
  const char *disabledText;
};

const char _disabledSTR [] PROGMEM = { "Disabled" };

// Menu items types definitions
#define AM_ITEM_TYPE_HEADER 0
#define AM_ITEM_TYPE_MENU 1
#define AM_ITEM_TYPE_COMMAND 2
#define AM_ITEM_TYPE_RANGE 3
#define AM_ITEM_TYPE_TOGGLE 4
#define AM_ITEM_TYPE_EOM 5

// Selection mode
#define AM_SELECTION_MODE_ICON 0
#define AM_SELECTION_MODE_INVERTED 1

// Letters size and box area
#define SCREEN_LETTER_W 6
#define SCREEN_LETTER_H 8
#define SCREEN_BOX_AREA 0.9

// If colors are not defined
#ifndef BLACK
#define BLACK 0x0000
#endif
#ifndef WHITE
#define WHITE 0xFFFF
#endif

template <class T>
class ArduMenu {
  public:
    // Init function
    ArduMenu(MENU_ITEM * menu, T & display);

    // Public variables
    bool inRange = false;
    uint8_t currentMenuItemIdx = 0;
    MENU_ITEM *currentMenuTable;

    // Public functions
    void drawMenu();
    void down(int16_t = NULL, int16_t = NULL, uint16_t = 1);
    void up(int16_t = NULL, int16_t = NULL, uint16_t = 1);
    void enter(int16_t = NULL, int16_t = NULL);
    void setSelectionMode(uint8_t, char icon = 16);
    void setTextSize(uint8_t);

  protected:
    // Protected variables
    uint8_t _itemsOffset = 0;
    uint8_t _baseOffset = 0;
    uint8_t _oldMenuItemIdx = NULL;
    uint8_t _oldItemsOffset = 0;
    uint8_t _oldBaseOffset = 0;
    MENU_ITEM *_oldMenuTable = NULL;
    bool _inDisabled = false;
    uint8_t _boxWidth;
    uint8_t _boxHeight;
    uint8_t _boxXMargin;
    uint8_t _boxYMargin;
    uint8_t _boxLines;
    uint8_t _boxLinesYMargin;
    uint8_t _boxColumns;
    uint8_t _boxColumnsXMargin;
    uint8_t _textSize = 1;
    uint8_t _letterW;
    uint8_t _letterH;
    uint8_t _screen_columns;
    uint8_t _screen_lines;
    char _selectionIcon = 16;
    uint8_t _selectionMode = AM_SELECTION_MODE_ICON;
    int8_t _rangeStatusLevel = -1;
    T _display;

    // Protected functions
    char * _centerText(int, uint8_t);
    char * _centerText(const char *, uint8_t);
    void _cleanUp();
    void _drawMenuItem(uint8_t);
    void _reDraw();
    void _setRangeCurrent(uint16_t);
    void _setRangeMetter(uint8_t);
    void _setBoxSize();
};

#include "ArduMenu.hpp"