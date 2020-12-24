template <class T>
ArduMenu<T>::ArduMenu(MENU_ITEM * menu, T & display):
  currentMenuTable(menu),
  _display(display) 
{
  _boxWidth = _display.width() * SCREEN_BOX_AREA;
  _boxHeight = _display.height() * SCREEN_BOX_AREA;
  _boxXMargin = (_display.width() - _boxWidth) / 2;
  _boxYMargin = (_display.height() - _boxHeight) / 2;

  #ifdef DEBUG
  Serial.print(F("_boxWidth: "));
  Serial.println(_boxWidth);
  Serial.print(F("_boxHeight: "));
  Serial.println(_boxHeight);
  Serial.print(F("_boxXMargin: "));
  Serial.println(_boxXMargin);
  Serial.print(F("_boxYMargin: "));
  Serial.println(_boxYMargin);
  #endif

  setTextSize(_textSize);
}

template <class T>
void ArduMenu<T>::setTextSize(uint8_t size) {
  _textSize = size;
  _letterW = SCREEN_LETTER_W * (1 << size - 1);
  _letterH = SCREEN_LETTER_H * (1 << size - 1);
  // Calculatin screen lines and columns
  _screen_lines = _display.height() / _letterH;
  _screen_columns = _display.width() / _letterW;

  // Calculating message box sizes and position
  _boxLines = _boxHeight / _letterH;
  _boxColumns = _boxWidth / _letterW;
  _boxLinesYMargin = _boxYMargin + (_boxHeight - (_boxLines * _letterH)) / 2;
  _boxColumnsXMargin = _boxXMargin + (_boxWidth - (_boxColumns * _letterW)) / 2;

  // Adjusting size if there's no enough space
  if ((_boxLinesYMargin - _boxYMargin) < 2) {
    _boxLines -= 1;
    _boxLinesYMargin = _boxYMargin + (_boxHeight - (_boxLines * _letterH)) / 2;
  }
  if ((_boxColumnsXMargin - _boxXMargin) < 2) {
    _boxColumns -= 1;
    _boxColumnsXMargin = _boxXMargin + (_boxWidth - (_boxColumns * _letterW)) / 2;
  }

  #ifdef DEBUG
  Serial.print(F("_letterW: "));
  Serial.println(_letterW);
  Serial.print(F("_letterH: "));
  Serial.println(_letterH);
  Serial.print(F("_textSize mult: "));
  Serial.println((1 << size - 1));
  Serial.print(F("_boxLines: "));
  Serial.println(_boxLines);
  Serial.print(F("_boxColumns: "));
  Serial.println(_boxColumns);
  Serial.print(F("_boxLinesYMargin: "));
  Serial.println(_boxLinesYMargin);
  Serial.print(F("_boxColumnsXMargin: "));
  Serial.println(_boxColumnsXMargin);
  #endif
}

template <class T>
void ArduMenu<T>::setSelectionMode(uint8_t sMode, char icon) {
  _selectionMode = sMode;
  if (_selectionMode == AM_SELECTION_MODE_ICON) {
    _selectionIcon = icon;
  }
  
  if (!inRange) {
    drawMenu();
  }
}

template <class T>
void ArduMenu<T>::drawMenu() {
  _cleanUp();
  _display.setTextSize(_textSize);
  _display.setCursor(0, 0);
  
  // This is a bit tricky and maybe will change in the future.
  // To keep a track of the top menus, the pointer must be saved
  // somewhere, and the "back" options is where I've choosen
  if (_oldMenuTable != NULL) {
    for (uint8_t i = 0; i < 99; i++) {
      if (currentMenuTable[i].type == AM_ITEM_TYPE_EOM) {
        currentMenuTable[i].subItems = _oldMenuTable;
        break;
      }
    }
  }

  uint8_t loopEnd = _screen_lines + _itemsOffset;

  // Running through the menu items
  for (uint8_t i = 0; i < loopEnd; i++) {
    if (currentMenuTable[i].type == AM_ITEM_TYPE_HEADER || i >= (_itemsOffset + _baseOffset)) {
      #ifdef DEBUG
      Serial.print(F("Printing item: "));
      Serial.println(i);
      Serial.print(F("Items Offset: "));
      Serial.println(_itemsOffset);
      Serial.print(F("Base Offset: "));
      Serial.println(_baseOffset);
      #endif
      _drawMenuItem(i);
    }
    else {
      #ifdef DEBUG
      Serial.print(F("Ignoring item: "));
      Serial.println(i);
      Serial.print(F("Items Offset: "));
      Serial.println(_itemsOffset);
      Serial.print(F("Base Offset: "));
      Serial.println(_baseOffset);
      #endif
    }

    if (currentMenuTable[i].type == AM_ITEM_TYPE_EOM) {
      break;
    }
  }

  _reDraw();
}

template <class T>
void ArduMenu<T>::down(int16_t min, int16_t max, uint16_t steps) {
  if (_inDisabled) {
    // Ignore buttons
  }
  else if (inRange) {
    int16_t currentStep = (*currentMenuTable[currentMenuItemIdx].rangeManage)(-steps);
    _setRangeCurrent(currentStep);

    #ifdef DEBUG
    Serial.print(F("max: "));
    Serial.println(max);
    Serial.print(F("min: "));
    Serial.println(min);
    #endif

    if (min != max) {
      int16_t range = max - min;
      uint8_t n = (float)(currentStep - min) / range * _boxColumns;
      _setRangeMetter(n);
    }
    _reDraw();
  }
  else {
    if (currentMenuTable[currentMenuItemIdx].type != AM_ITEM_TYPE_EOM) {
      currentMenuItemIdx++;
      if ((currentMenuItemIdx - _itemsOffset) >= (_screen_lines)) {
        _itemsOffset++;
        drawMenu();
      }
      else {
        _display.setCursor(0, _letterH * (currentMenuItemIdx - _itemsOffset - 1));
        _drawMenuItem(currentMenuItemIdx - 1);
        _drawMenuItem(currentMenuItemIdx);

        _reDraw();
      }
    }
  }
}

template <class T>
void ArduMenu<T>::up(int16_t min, int16_t max, uint16_t steps) {
  if (_inDisabled) {
    // Ignore buttons
  }
  else if (inRange) {
    int16_t currentStep = (*currentMenuTable[currentMenuItemIdx].rangeManage)(steps);
    _setRangeCurrent(currentStep);
    
    #ifdef DEBUG
    Serial.print(F("max: "));
    Serial.println(max);
    Serial.print(F("min: "));
    Serial.println(min);
    #endif

    if (min != max) {
      int16_t range = max - min;
      uint8_t n = (float)(currentStep - min) / range * _boxColumns;
      _setRangeMetter(n);
    }
    _reDraw();
  }
  else {
    if (currentMenuItemIdx != 0 && currentMenuTable[currentMenuItemIdx - 1].type != AM_ITEM_TYPE_HEADER) {
      currentMenuItemIdx--;
      if ((_baseOffset + _itemsOffset) > currentMenuItemIdx) {
        _itemsOffset--;
        drawMenu();
      }
      else {
        _display.setCursor(0, _letterH * (currentMenuItemIdx - _itemsOffset));
        _drawMenuItem(currentMenuItemIdx);
        _drawMenuItem(currentMenuItemIdx + 1);

        _reDraw();
      }
    }
  } 
}

template <class T>
void ArduMenu<T>::enter(int16_t min, int16_t max) {
  if (currentMenuTable[currentMenuItemIdx].disabled == true) {
    if (_inDisabled) {
      drawMenu();
      _inDisabled = false;
    }
    else {
      _display.fillRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, WHITE);
      _display.drawRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, BLACK);

      if (_boxLines >= 2) {
        uint8_t center = (_boxHeight - _letterH) / 2;
        _display.setCursor(_boxColumnsXMargin, center);
        if (currentMenuTable[currentMenuItemIdx].disabledText == NULL) {
          uint8_t len = strlen_P(_disabledSTR);
          char txt[len + 1];
          memcpy_P(txt, _disabledSTR, len);
          txt[len] = '\0';
          char * tmpText = _centerText(txt, _boxColumns);
          _display.print(tmpText);
          delete [] tmpText;
        }
        else {        
          uint8_t len = strlen_P(currentMenuTable[currentMenuItemIdx].disabledText);
          char txt[len + 1];
          memcpy_P(txt, currentMenuTable[currentMenuItemIdx].disabledText, len);
          txt[len] = '\0';
          char * tmpText = _centerText(txt, _boxColumns);
          _display.print(tmpText);
          delete [] tmpText;
        }
      }

      _reDraw();

      _inDisabled = true;
    }
    
    return;
  }
  switch(currentMenuTable[currentMenuItemIdx].type) {
    //
    // The item is a menu... Displaying the submenu.
    //
    case AM_ITEM_TYPE_MENU: {
      if (currentMenuTable[currentMenuItemIdx].subItems != NULL) {
        _oldMenuTable = currentMenuTable;
        _oldMenuItemIdx = currentMenuItemIdx;
        _oldItemsOffset = _itemsOffset;
        _oldBaseOffset = _baseOffset;
        currentMenuTable = currentMenuTable[currentMenuItemIdx].subItems;

        _itemsOffset = 0;
        _baseOffset = 0;
        currentMenuItemIdx = 0;
        drawMenu();
      }
      else {
        // Dumy for debug
        #ifdef DEBUG
        Serial.println(F("ERROR: The menu contains no items"));
        #endif
      }
      break;
    }

    case AM_ITEM_TYPE_RANGE: {
      if (currentMenuTable[currentMenuItemIdx].rangeManage != NULL) {
        if (inRange) {
          #ifdef DEBUG
          Serial.println("Exiting range...");
          #endif
          (*currentMenuTable[currentMenuItemIdx].rangeManage)(32767);
          inRange = false;
          drawMenu();
        }
        else {
          #ifdef DEBUG
          Serial.println(F("Range item detected..."));
          #endif
          inRange = true;
          uint16_t currentStep = (*currentMenuTable[currentMenuItemIdx].rangeManage)(0);

          _display.fillRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, WHITE);
          _display.drawRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, BLACK);

          _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin);
          uint8_t len = strlen_P(currentMenuTable[currentMenuItemIdx].text);
          char txt[len + 1];
          memcpy_P(txt, currentMenuTable[currentMenuItemIdx].text, len);
          txt[len] = '\0';
          char * tmpText = _centerText(txt, _boxColumns);
          _display.print(tmpText);
          #ifdef DEBUG
          Serial.print(F("Length: "));
          Serial.println(len);
          Serial.print(F("Text In: "));
          Serial.println(currentMenuTable[currentMenuItemIdx].text);
          Serial.print(F("Text Extracted: "));
          Serial.println(txt);
          Serial.print(F("Text Out: "));
          Serial.println(tmpText);
          #endif
          delete [] tmpText;

          _setRangeCurrent(currentStep);

          
          #ifdef DEBUG
          Serial.print(F("max: "));
          Serial.println(max);
          Serial.print(F("min: "));
          Serial.println(min);
          #endif

          if (min != max) {
            int16_t range = max - min;
            uint8_t n = (float)(currentStep - min) / range * _boxColumns;
            _rangeStatusLevel = -1;
            _setRangeMetter(n);
          }
          _reDraw();
        }
      }
      break;

      case AM_ITEM_TYPE_TOGGLE: {
        uint16_t y = _letterH * currentMenuItemIdx;
        #ifdef DEBUG
        Serial.print(F("Line Y: "));
        Serial.println(y);
        #endif

        // Set the cursor position
        (*currentMenuTable[currentMenuItemIdx].toggleManage)(true);
        // Redraw the entire item to simplify
        _display.setCursor(0, _letterH * (currentMenuItemIdx - _itemsOffset));
        _drawMenuItem(currentMenuItemIdx);

        // Redraw if required by the display
        _reDraw();
      }
      break;
    }

    //
    // The item is a function
    //
    case AM_ITEM_TYPE_COMMAND: {
      bool ret = (currentMenuTable[currentMenuItemIdx].function)(&currentMenuTable[currentMenuItemIdx]);
      if (ret == true) {
        #ifdef DEBUG
        Serial.println(F("Return is true"));
        #endif
        if (_oldMenuTable) {
          currentMenuTable = _oldMenuTable;
          currentMenuItemIdx = _oldMenuItemIdx;
          _itemsOffset = _oldItemsOffset;
          _baseOffset = _oldBaseOffset;
          _oldMenuTable = NULL;
          _oldMenuItemIdx = NULL;
          _oldItemsOffset = NULL;
          _oldBaseOffset = NULL;
          drawMenu();
        }
      }
      else {
        _oldMenuTable = NULL;
        _oldMenuItemIdx = NULL;
        _oldItemsOffset = NULL;
        _oldBaseOffset = NULL;
        #ifdef DEBUG
        Serial.println(F("Return is false"));
        #endif
      }
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
      if (currentMenuTable[currentMenuItemIdx].function != NULL) {
        (currentMenuTable[currentMenuItemIdx].function)(&currentMenuTable[currentMenuItemIdx]);
      }
      else {
        #ifdef DEBUG
        Serial.println(F("Back button has no function"));
        #endif
      }
      
      // Set top menu as current menu (if exists) and reset variables
      if (currentMenuTable[currentMenuItemIdx].subItems != NULL) {
        currentMenuTable = currentMenuTable[currentMenuItemIdx].subItems;
        _itemsOffset = 0;
        currentMenuItemIdx = 0;
        _baseOffset = 0;
        _oldMenuTable = NULL;
        _oldMenuItemIdx = NULL;
        _oldItemsOffset = NULL;
        _oldBaseOffset = NULL;
        // Draw the top menu
        drawMenu();
      }
      else {
        _oldMenuTable = NULL;
        _oldMenuItemIdx = NULL;
        _oldItemsOffset = NULL;
        _oldBaseOffset = NULL;
        #ifdef DEBUG
        Serial.println(F("Back button has no top menu"));
        #endif
      }
      break;
    }
  }
}

template <class T>
char * ArduMenu<T>::_centerText(int num, uint8_t length) {
  char text[7];
  #ifndef ARDUINO_ARCH_STM32F1
  snprintf_P(text, 7, PSTR("%d"), num);
  #else
  snprintf(text, 7, "%d", num);
  #endif
  return ArduMenu<T>::_centerText(text, length);
}

template <class T>
char * ArduMenu<T>::_centerText(const char *text, uint8_t length) {
  uint8_t N = strlen(text);
  N = (N > length) ? length : N;

  uint8_t middle = (length - N) / 2;
  char format[11];
  #ifndef ARDUINO_ARCH_STM32F1
  snprintf_P(format, 11, PSTR("%%%ds%%%ds"), middle + N, length - (middle + N));
  #else
  snprintf(format, 11, "%%%ds%%%ds", middle + N, length - (middle + N));
  #endif
  char * finalText = new char[length + 1]{};
  snprintf(finalText, length + 1, format, text, "");
  return finalText;
}

template <class T>
void ArduMenu<T>::_cleanUp() {
  #ifdef _ADAFRUIT_PCD8544_H
  _display.clearDisplay();
  #ifdef DEBUG
  Serial.println(F("Clearing PCD8544"));
  #endif
  #endif
  #if defined(_ADAFRUIT_ST7735H_)||defined(_ADAFRUIT_ST7789H_)
  #ifdef DEBUG
  Serial.println(F("Clearing ST7735|ST7789"));
  #endif
  _display.fillScreen(WHITE);
  #endif
}

template <class T>
void ArduMenu<T>::_drawMenuItem(uint8_t i) {
  char text[_screen_columns + 1];
  char format[14];
  #ifdef ARDUINO_ARCH_STM32F1
  snprintf(format, 14, " %%-%d.%ds ", _screen_columns - 2, _screen_columns - 2);
  #else
  snprintf_P(format, 14, PSTR(" %%-%d.%ds "), _screen_columns - 2, _screen_columns - 2);
  #endif
  uint8_t len = strlen_P(currentMenuTable[i].text);
  char txt[len + 1];
  memcpy_P(txt, currentMenuTable[i].text, len);

  if (currentMenuTable[i].type == AM_ITEM_TYPE_HEADER) {
    char * tmpText = _centerText(txt, _screen_columns - 2);
    snprintf(text, _screen_columns + 1, format, tmpText);
    delete [] tmpText;
    _baseOffset = 1;
    if (currentMenuItemIdx == 0) {
      currentMenuItemIdx = 1;
    }
  }
  else {
    snprintf(text, _screen_columns + 1, format, txt);
  }

  if (currentMenuTable[i].type == AM_ITEM_TYPE_HEADER) {
    _display.setTextColor(BLACK, WHITE);
    text[_screen_columns - 1] = 4;
    text[0] = 4;
  }
  else if (_selectionMode == AM_SELECTION_MODE_INVERTED && i == (currentMenuItemIdx)) {
    _display.setTextColor(WHITE, BLACK);
  }
  else if (_selectionMode == AM_SELECTION_MODE_ICON && i == (currentMenuItemIdx)) {
    _display.setTextColor(BLACK, WHITE);
    text[0] = _selectionIcon;
  }
  else {
    _display.setTextColor(BLACK, WHITE);
  }

  if (currentMenuTable[i].type == AM_ITEM_TYPE_MENU) {
    text[_screen_columns - 1] = 175;
  }
  else if (currentMenuTable[i].type == AM_ITEM_TYPE_TOGGLE) {
    if ((*currentMenuTable[i].toggleManage)(false)) {
      text[_screen_columns - 1] = 7;
    }
    else {
      text[_screen_columns - 1] = 9;
    }
  }

  #ifdef DEBUG
  Serial.println(text);
  #endif

  _display.println(text);
}

template <class T>
void ArduMenu<T>::_reDraw() {
  // This function just redraw the screen depending if is necessary
  #ifdef _ADAFRUIT_PCD8544_H
  _display.display();
  #endif
}

template <class T>
void ArduMenu<T>::_setRangeCurrent(uint16_t num) {
  #ifdef DEBUG
  Serial.println(F("Display box have more than three lines"));
  #endif
  _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin + (_letterH * (_boxLines / 2)));
  char * tmpText = _centerText(num, _boxColumns);
  _display.print(tmpText);
  delete [] tmpText;
}

template <class T>
void ArduMenu<T>::_setRangeMetter(uint8_t num) {
  #ifdef DEBUG
  Serial.print(F("Draw lines: "));
  Serial.println(num);
  #endif
  uint8_t fullBoxes = 0;
  uint8_t emptyBoxes = 0;
  uint16_t startPosition = 0; 

  if (_rangeStatusLevel == -1) {
    fullBoxes = num;
    emptyBoxes = _boxColumns - num;
    startPosition = _boxColumnsXMargin;
  }
  else {
    if (num > _rangeStatusLevel) {
      startPosition = _rangeStatusLevel * _letterW;
      fullBoxes = num - _rangeStatusLevel;
      emptyBoxes = 0;
    }
    else if (num < _rangeStatusLevel) {
      startPosition = num * _letterW;
      fullBoxes = 0;
      emptyBoxes = _rangeStatusLevel - num;
    }
  }

  _display.setCursor(startPosition, _boxLinesYMargin + (_letterH * (_boxLines / 2 + 1)));
  for (int i = 0; i < fullBoxes; i++) {
    _display.write(220);
  }
  for (int i = 0; i < emptyBoxes; i++) {
    _display.write(219);
  }
}