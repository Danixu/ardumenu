#include "Arduino.h"
#include "ArduMenu.h"

ArduMenu::ArduMenu(MENU_ITEM *menu, Adafruit_PCD8544 display):
  _currentMenuItemIdx(0),
  _itemsOffset(0),
  _currentMenuTable(menu),
  _lines(SCREEN_LINES_TOTAL),
  _display(display) {
  //nothing to do
}

void ArduMenu::drawMenu() {
  _display.clearDisplay();
  _display.setCursor(0, 0);
  
  if (_currentMenuTable[0].Type == AM_ITEM_TYPE_HEADER) {
    char * tmpText = centerText(_currentMenuTable[0].Text, 12);
    char text[15];
    snprintf(text, 15, "*%s*", tmpText);
    delete [] tmpText;
    _display.setTextColor(BLACK, WHITE);
    _display.println(text);
    if (_currentMenuItemIdx == 0) {
      _lines = SCREEN_LINES_TOTAL - 1;
      _itemsOffset++;
      _currentMenuItemIdx = 1;
    }
  }
  
  // This is a bit tricky and maybe will change in the future.
  // To keep a track of the top menus, the pointer must be saved
  // somewhere, and the "back" options is where I've choose
  for (byte i = 0; i < 99; i++) {
    if (_currentMenuTable[i].Type == AM_ITEM_TYPE_EOM) {
      _currentMenuTable[i].SubItems = _oldMenuTable;
      break;
    }
  }

  // Running through the menu items
  for (byte i = 0; i < _lines; i++) {
    if (_currentMenuTable[i + _itemsOffset].Type != AM_ITEM_TYPE_HEADER) {
      _display.setTextColor(BLACK, WHITE);
      char text[SCREEN_COLUMNS + 1];
      char format[14];
      if (_currentMenuTable[i + _itemsOffset].Type == AM_ITEM_TYPE_MENU) {
        snprintf(format, 14, " %%-%d.%ds ", SCREEN_COLUMNS - 3, SCREEN_COLUMNS - 3);
        snprintf(text, SCREEN_COLUMNS + 1, format, _currentMenuTable[i + _itemsOffset].Text);
        _display.print(text);
        _display.write(175);
        _display.println();
      }
      else {
        snprintf(format, 14, " %%-%d.%ds  ", SCREEN_COLUMNS - 3, SCREEN_COLUMNS - 3);
        snprintf(text, SCREEN_COLUMNS + 1, format, _currentMenuTable[i + _itemsOffset].Text);
        _display.println(text);
        if (_currentMenuTable[i + _itemsOffset].Type == AM_ITEM_TYPE_EOM) {
          break;
        }
      }
    }
  }
  // Setting selector to selected item
  if (_currentMenuItemIdx > _lines) {
    _display.setCursor(0, SCREEN_LETTER_H * _lines);
  }
  else {
    _display.setCursor(0, SCREEN_LETTER_H * _currentMenuItemIdx);
  }
  _display.write(16);
  _display.display();
}

void ArduMenu::down() {
  if (_currentMenuTable[_currentMenuItemIdx].Type != AM_ITEM_TYPE_EOM) {
    if ((_currentMenuItemIdx - _itemsOffset) >= (_lines - 1)) {
      _itemsOffset++;
      _currentMenuItemIdx += 1;
      drawMenu();
    } else {
      _display.setCursor(0, SCREEN_LETTER_H * _currentMenuItemIdx);
      _display.print(" ");
      _currentMenuItemIdx += 1;
      _display.setCursor(0, SCREEN_LETTER_H * _currentMenuItemIdx);
      _display.write(16);
      _display.display();
    }
  }
}

void ArduMenu::up() {
  if (_currentMenuItemIdx != 0 && _currentMenuTable[_currentMenuItemIdx - 1].Type != AM_ITEM_TYPE_HEADER) {
    if (_itemsOffset >= _currentMenuItemIdx) {
      _itemsOffset--;
      _currentMenuItemIdx -= 1;
      drawMenu();
    } else {
      _currentMenuItemIdx -= 1;
      if (_currentMenuTable[0].Type == AM_ITEM_TYPE_HEADER) {
        _display.setCursor(0, SCREEN_LETTER_H * (_currentMenuItemIdx + 2 - _itemsOffset));
        _display.print(" ");
        _display.setCursor(0, SCREEN_LETTER_H * (_currentMenuItemIdx + 1 - _itemsOffset));
      }
      else {
        _display.setCursor(0, SCREEN_LETTER_H * (_currentMenuItemIdx + 1 - _itemsOffset));
        _display.print(" ");
        _display.setCursor(0, SCREEN_LETTER_H * (_currentMenuItemIdx - _itemsOffset));
      }
      _display.write(16);
      _display.display();
    }
  }
}

void ArduMenu::enter() {
  switch(_currentMenuTable[_currentMenuItemIdx].Type) {
    //
    // The item is a menu... Displaying the submenu.
    //
    case AM_ITEM_TYPE_MENU: {
      if (_currentMenuTable[_currentMenuItemIdx].SubItems != NULL) {
        _oldMenuTable = _currentMenuTable;
        _currentMenuTable = _currentMenuTable[_currentMenuItemIdx].SubItems;
        _itemsOffset = 0;
        _currentMenuItemIdx = 0;
        drawMenu();
      }
      else {
        // Dumy for debug
        #ifdef DEBUG
        Serial.println("ERROR: The menu contains no items");
        #endif
      }
      break;
    }

    //
    // The item is a function
    //
    case AM_ITEM_TYPE_COMMAND: {
      (_currentMenuTable[_currentMenuItemIdx].Function)();
      break;
    }

    //
    // The item is a function
    //
    case AM_ITEM_TYPE_EOM: {
      //
      // If you have set a function for the exit option, execute it.
      // Usefull if you want to make an action on menu exit
      //
      Serial.println(_currentMenuItemIdx);
      if (_currentMenuTable[_currentMenuItemIdx].Function != NULL) {
        (_currentMenuTable[_currentMenuItemIdx].Function)();
      }
      // Set top menu as current menu and reset variables
      _currentMenuTable = _currentMenuTable[_currentMenuItemIdx].SubItems;
      _itemsOffset = 0;
      _currentMenuItemIdx = 0;
      // Draw the top menu
      drawMenu();
      break;
    }
  }
}

char * ArduMenu::centerText(const char *text) {
  return centerText(text, SCREEN_COLUMNS + 1);
}

char * ArduMenu::centerText(const char *text, unsigned int length) {
  unsigned int N = strlen(text);
  if ( N >= length) {
    char format[6];
    snprintf(format, 6, "%%.%ds", length);
    char * finalText = new char[length + 1]{};
    snprintf(finalText, length + 1, format, text);
    return finalText;
  }
  else {
    short middle = (length - N) / 2;
    char format[11];
    snprintf(format, 11, "%%%ds%%%ds", middle + N, length - (middle + N));
    char * finalText = new char[length + 1]{};
    snprintf(finalText, length + 1, format, text, "");
    return finalText;
  }
}