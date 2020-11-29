template <class T>
ArduMenu<T>::ArduMenu(MENU_ITEM * menu, T & display):
  inRange(false),
  _currentMenuItemIdx(0),
  _itemsOffset(0),
  _textSize(1),
  _hasBox(true),
  _oldMenuItemIdx(NULL),
  _oldMenuTable(NULL),
  _selectionMode(AM_SELECTION_MODE_ICON),
  _selectionIcon(16),
  _currentMenuTable(menu),
  _display(display)
{
  _setBoxSize();
  setTextSize(_textSize);
}

template <class T>
void ArduMenu<T>::setTextSize(uint8_t size)
{
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
  _toggleMargin = _letterW - (_letterW * 0.8);
  _toggleX = _letterW * (_screen_columns - 1) + _toggleMargin;
  _toggleWH = _letterW  - (_toggleMargin * 2);

  // Adjusting size if there's no enough space
  if ((_boxLinesYMargin - _boxYMargin) < 2)
  {
    _boxLines -= 1;
    _boxLinesYMargin = _boxYMargin + (_boxHeight - (_boxLines * _letterH)) / 2;
  }
  if ((_boxColumnsXMargin - _boxXMargin) < 2)
  {
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
  Serial.print(F("_toggleMargin: "));
  Serial.println(_toggleMargin);
  Serial.print(F("_toggleX: "));
  Serial.println(_toggleX);
  Serial.print(F("_toggleWH: "));
  Serial.println(_toggleWH);
  #endif
}

template <class T>
void ArduMenu<T>::setSelectionMode(uint8_t sMode)
{
  setSelectionMode(sMode, _selectionIcon);
}

template <class T>
void ArduMenu<T>::setSelectionMode(uint8_t sMode, char icon)
{
  _selectionMode = sMode;
  if (_selectionMode == AM_SELECTION_MODE_ICON)
  {
    _selectionIcon = icon;
  }
  else if (sMode == AM_SELECTION_MODE_INVERTED)
  {
    // Nothing to do
  }
  
  if (!inRange)
  {
    drawMenu();
  }
}

template <class T>
void ArduMenu<T>::drawMenu()
{
  _cleanUp();
  _display.setCursor(0, 0);
  _display.setTextSize(_textSize);
  
  if (_currentMenuTable[0].type == AM_ITEM_TYPE_HEADER)
  {
    uint8_t len = strlen_P(_currentMenuTable[0].text);
    char txt[len + 1] = {};
    memcpy_P(txt, _currentMenuTable[0].text, len);
    char * tmpText = _centerText(txt, _screen_columns - 2);
    char text[_screen_columns + 1];
    snprintf_P(text, _screen_columns + 1, PSTR("*%s*"), tmpText);
    delete [] tmpText;
    _display.setTextColor(BLACK, WHITE);
    _display.println(text);
    if (_currentMenuItemIdx == 0)
    {
      _lines = _screen_lines - 1;
      _itemsOffset++;
      _currentMenuItemIdx = 1;
    }
  }
  else
  {
    _lines = _screen_lines;
  }
  
  // This is a bit tricky and maybe will change in the future.
  // To keep a track of the top menus, the pointer must be saved
  // somewhere, and the "back" options is where I've choose
  if (_oldMenuTable != NULL)
  {
    for (byte i = 0; i < 99; i++)
    {
      if (_currentMenuTable[i].type == AM_ITEM_TYPE_EOM)
      {
        _currentMenuTable[i].subItems = _oldMenuTable;
        break;
      }
    }
  }

  // Running through the menu items
  for (uint8_t i = 0; i < _lines; i++)
  {
    if (_currentMenuTable[i + _itemsOffset].type != AM_ITEM_TYPE_HEADER)
    {
      #ifdef DEBUG
      Serial.print(F("Printing item: "));
      Serial.println(i);
      #endif
      _drawMenuItem(i);

      if (_currentMenuTable[i + _itemsOffset].type == AM_ITEM_TYPE_EOM)
      {
        break;
      }
    }
  }

  _reDraw();
}

template <class T>
void ArduMenu<T>::down(int16_t min, int16_t max)
{
  if (_inDisabled)
  {
    // Ignore buttons
  }
  else if (inRange)
  {
    int16_t currentStep = (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(-1);
    _setRangeCurrent(currentStep);

    if (min != max)
    {
      int16_t range = max - min;
      uint8_t n = (float)(currentStep - min) / range * _boxColumns;
      _setRangeMetter(n);
    }
    _reDraw();
  }
  else
  {
    if (_currentMenuTable[_currentMenuItemIdx].type != AM_ITEM_TYPE_EOM)
    {
      _currentMenuItemIdx++;
      if ((_currentMenuItemIdx - _itemsOffset) >= _lines)
      {
        _itemsOffset++;
        drawMenu();
      }
      else
      {
        if (_currentMenuTable[0].type == AM_ITEM_TYPE_HEADER)
        {
          _display.setCursor(0, _letterH * (_currentMenuItemIdx - _itemsOffset));
          _drawMenuItem(_currentMenuItemIdx - _itemsOffset - 1);
          _drawMenuItem(_currentMenuItemIdx - _itemsOffset);
        }
        else
        {
          _display.setCursor(0, _letterH * (_currentMenuItemIdx - _itemsOffset - 1));
          _drawMenuItem(_currentMenuItemIdx - _itemsOffset - 1);
          _drawMenuItem(_currentMenuItemIdx - _itemsOffset);
        }

        _reDraw();
      }
    }
  }
}

template <class T>
void ArduMenu<T>::up(int16_t min, int16_t max)
{
  if (_inDisabled)
  {
    // Ignore buttons
  }
  else if (inRange)
  {
    int16_t currentStep = (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(1);
    _setRangeCurrent(currentStep);
    if (min != max)
    {
      int16_t range = max - min;
      uint8_t n = (float)(currentStep - min) / range * _boxColumns;
      _setRangeMetter(n);
    }
    _reDraw();
  }
  else
  {
    if (_currentMenuItemIdx != 0 && _currentMenuTable[_currentMenuItemIdx - 1].type != AM_ITEM_TYPE_HEADER)
    {
      _currentMenuItemIdx--;
      if (_itemsOffset > _currentMenuItemIdx)
      {
        _itemsOffset--;
        drawMenu();
      }
      else
      {
         if (_currentMenuTable[0].type == AM_ITEM_TYPE_HEADER)
        {
          _display.setCursor(0, _letterH * (_currentMenuItemIdx - _itemsOffset + 1));
          _drawMenuItem(_currentMenuItemIdx - _itemsOffset);
          _drawMenuItem(_currentMenuItemIdx - _itemsOffset + 1);
        }
        else
        {
          _display.setCursor(0, _letterH * (_currentMenuItemIdx - _itemsOffset));
          _drawMenuItem(_currentMenuItemIdx - _itemsOffset);
          _drawMenuItem(_currentMenuItemIdx - _itemsOffset + 1);
        }

        _reDraw();
      }
    }
  } 
}

template <class T>
void ArduMenu<T>::enter(int16_t min, int16_t max)
{
  if (_currentMenuTable[_currentMenuItemIdx].disabled == true)
  {
    if (_inDisabled)
    {
      drawMenu();
      _inDisabled = false;
    }
    else
    {
      if (_hasBox){
        _display.fillRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, WHITE);
        _display.drawRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, BLACK);
      }
      else
      {
        _cleanUp();
      }

      if (_boxLines >= 2)
      {
        uint8_t center = (_boxHeight - _letterH) / 2;
        _display.setCursor(_boxColumnsXMargin, center);
        if (_currentMenuTable[_currentMenuItemIdx].disabledText == NULL)
        {
          uint8_t len = strlen_P(_disabledSTR);
          char txt[len + 1];
          memcpy_P(txt, _disabledSTR, len);
          char * tmpText = _centerText(txt, _boxColumns);
          _display.print(tmpText);
          delete [] tmpText;
        }
        else
        {        
          uint8_t len = strlen_P(_currentMenuTable[_currentMenuItemIdx].disabledText);
          char txt[len + 1];
          memcpy_P(txt, _currentMenuTable[_currentMenuItemIdx].disabledText, len);
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
  switch(_currentMenuTable[_currentMenuItemIdx].type)
  {
    //
    // The item is a menu... Displaying the submenu.
    //
    case AM_ITEM_TYPE_MENU:
    {
      if (_currentMenuTable[_currentMenuItemIdx].subItems != NULL)
      {
        _oldMenuTable = _currentMenuTable;
        _oldMenuItemIdx = _currentMenuItemIdx;
        _oldItemsOffset = _itemsOffset;
        _currentMenuTable = _currentMenuTable[_currentMenuItemIdx].subItems;
        if (min && !max)
        {
          _itemsOffset = min - _lines;
          _currentMenuItemIdx = min;
        }
        else
        {
          _itemsOffset = 0;
          _currentMenuItemIdx = 0;
        }
        drawMenu();
      }
      else
      {
        // Dumy for debug
        #ifdef DEBUG
        Serial.println(F("ERROR: The menu contains no items"));
        #endif
      }
      break;
    }

    case AM_ITEM_TYPE_RANGE:
    {
      if (_currentMenuTable[_currentMenuItemIdx].rangeManage != NULL)
      {
        if (inRange)
        {
          #ifdef DEBUG
          Serial.println("Exiting range...");
          #endif
          (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(2);
          inRange = false;
          drawMenu();
        }
        else
        {
          #ifdef DEBUG
          Serial.println(F("Range item detected..."));
          #endif
          inRange = true;
          uint16_t currentStep = (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(0);
          if (_hasBox){
            _display.fillRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, WHITE);
            _display.drawRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, BLACK);
          }
          else
          {
            _cleanUp();
          }

          if (_boxLines >= 2)
          {
            _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin);
            uint8_t len = strlen_P(_currentMenuTable[_currentMenuItemIdx].text);
            char txt[len + 1];
            memcpy_P(txt, _currentMenuTable[_currentMenuItemIdx].text, len);
            txt[len] = '\0';
            char * tmpText = _centerText(txt, _boxColumns);
            _display.print(tmpText);
            #ifdef DEBUG
            Serial.print(F("Length: "));
            Serial.println(len);
            Serial.print(F("Text In: "));
            Serial.println(_currentMenuTable[_currentMenuItemIdx].text);
            Serial.print(F("Text Extracted: "));
            Serial.println(txt);
            Serial.print(F("Text Out: "));
            Serial.println(tmpText);
            #endif
            delete [] tmpText;
          }

          _setRangeCurrent(currentStep);

          if (min != max)
          {
            int16_t range = max - min;
            uint8_t n = (float)(currentStep - min) / range * _boxColumns;
            _setRangeMetter(n);
          }
          _reDraw();
        }
      }
      break;

      case AM_ITEM_TYPE_TOGGLE:
      {
        uint16_t y = _letterH * _currentMenuItemIdx;
        #ifdef DEBUG
        Serial.print(F("Line Y: "));
        Serial.println(y);
        #endif

        // Set the cursor position
        (*_currentMenuTable[_currentMenuItemIdx].toggleManage)(true);
        // Redraw the entire item to simplify
        _display.setCursor(0, _letterH * (_currentMenuItemIdx - _itemsOffset + 1));
        _drawMenuItem(_currentMenuItemIdx - _itemsOffset);

        // Redraw if required by the display
        _reDraw();
      }
      break;
    }

    //
    // The item is a function
    //
    case AM_ITEM_TYPE_COMMAND:
    {
      bool ret = (_currentMenuTable[_currentMenuItemIdx].function)(&_currentMenuTable[_currentMenuItemIdx]);
      if (ret == true)
      {
        #ifdef DEBUG
        Serial.println(F("Return is true"));
        #endif
        if (_oldMenuTable)
        {
          _currentMenuTable = _oldMenuTable;
          _currentMenuItemIdx = _oldMenuItemIdx;
          _itemsOffset = _oldItemsOffset;
          _oldMenuTable = NULL;
          _oldMenuItemIdx = NULL;
          _oldItemsOffset = NULL;
          drawMenu();
        }
      }
      else
      {
        _oldMenuTable = NULL;
        _oldMenuItemIdx = NULL;
        _oldItemsOffset = NULL;
        #ifdef DEBUG
        Serial.println(F("Return is false"));
        #endif
      }
      break;
    }

    //
    // The item is a function
    //
    case AM_ITEM_TYPE_EOM:
    {
      //
      // If you have set a function for the exit option, execute it.
      // Usefull if you want to make an action on menu exit
      //
      if (_currentMenuTable[_currentMenuItemIdx].function != NULL)
      {
        (_currentMenuTable[_currentMenuItemIdx].function)(&_currentMenuTable[_currentMenuItemIdx]);
      }
      else
      {
        #ifdef DEBUG
        Serial.println(F("Back button has no function"));
        #endif
      }
      
      // Set top menu as current menu (if exists) and reset variables
      if (_currentMenuTable[_currentMenuItemIdx].subItems != NULL)
      {
        _currentMenuTable = _currentMenuTable[_currentMenuItemIdx].subItems;
        _itemsOffset = 0;
        _currentMenuItemIdx = 0;
        _oldMenuTable = NULL;
        _oldMenuItemIdx = NULL;
        _oldItemsOffset = NULL;
        // Draw the top menu
        drawMenu();
      }
      else
      {
        _oldMenuTable = NULL;
        _oldMenuItemIdx = NULL;
        _oldItemsOffset = NULL;
        #ifdef DEBUG
        Serial.println(F("Back button has no top menu"));
        #endif
      }
      break;
    }
  }
}

template <class T>
char * ArduMenu<T>::_centerText(int num)
{
  return _centerText(num, _screen_columns + 1);
}

template <class T>
char * ArduMenu<T>::_centerText(int num, uint8_t length)
{
  char text[7] = "";
  snprintf_P(text, 7, PSTR("%d"), num);
  return ArduMenu<T>::_centerText(text, length);
}

template <class T>
char * ArduMenu<T>::_centerText(const char *text)
{
  return _centerText(text, _screen_columns + 1);
}

template <class T>
char * ArduMenu<T>::_centerText(const char *text, uint8_t length)
{
  uint8_t N = strlen(text);
  if ( N >= length)
  {
    char format[6];
    snprintf_P(format, 6, PSTR("%%.%ds"), length);
    char * finalText = new char[length + 1]{};
    snprintf(finalText, length + 1, format, text);
    return finalText;
  }
  else
  {
    uint8_t middle = (length - N) / 2;
    char format[11];
    snprintf_P(format, 11, PSTR("%%%ds%%%ds"), middle + N, length - (middle + N));
    char * finalText = new char[length + 1]{};
    snprintf(finalText, length + 1, format, text, "");
    return finalText;
  }
}

template <class T>
void ArduMenu<T>::_cleanUp()
{
  #ifdef _ADAFRUIT_PCD8544_H
  _display.clearDisplay();
  #endif
  #ifdef _ADAFRUIT_ST7735H_
  _display.fillScreen(WHITE);
  #endif
}

template <class T>
void ArduMenu<T>::_drawMenuItem(uint8_t i)
{
  char text[_screen_columns + 1];
  char format[14];
  
  snprintf_P(format, 14, PSTR(" %%-%d.%ds "), _screen_columns - 2, _screen_columns - 2);
  uint8_t len = strlen_P(_currentMenuTable[i + _itemsOffset].text);
  char txt[len+1] = {};
  memcpy_P(txt, _currentMenuTable[i + _itemsOffset].text, len);
  snprintf(text, _screen_columns + 1, format, txt);

  if (_selectionMode == AM_SELECTION_MODE_INVERTED && i == (_currentMenuItemIdx - _itemsOffset))
  {
    _display.setTextColor(WHITE, BLACK);
  }
  else if (_selectionMode == AM_SELECTION_MODE_ICON && i == (_currentMenuItemIdx - _itemsOffset))
  {
    _display.setTextColor(BLACK, WHITE);
    text[0] = _selectionIcon;
  }
  else
  {
    _display.setTextColor(BLACK, WHITE);
  }

  if (_currentMenuTable[i + _itemsOffset].type == AM_ITEM_TYPE_MENU)
  {
    text[_screen_columns - 1] = 175;
  }

  #ifdef DEBUG
  Serial.println(text);
  #endif

  _display.print(text);
  if (_currentMenuTable[i + _itemsOffset].type == AM_ITEM_TYPE_TOGGLE)
  {
    uint16_t y = _display.getCursorY() + ((_letterH - _letterW)/2) + _toggleMargin;
    #ifdef DEBUG
    Serial.print(F("Line Y: "));
    Serial.println(y);
    #endif

    if (_selectionMode == AM_SELECTION_MODE_INVERTED && i == (_currentMenuItemIdx - _itemsOffset))
    {
      if ((*_currentMenuTable[i + _itemsOffset].toggleManage)(false))
      {
        _display.fillRoundRect(_toggleX, y, _toggleWH, _toggleWH, 1, WHITE);
      }
      else
      {
        _display.drawRoundRect(_toggleX, y, _toggleWH, _toggleWH, 1, WHITE);
      }
    }
    else
    {
      if ((*_currentMenuTable[i + _itemsOffset].toggleManage)(false))
      {
        _display.fillRoundRect(_toggleX, y, _toggleWH, _toggleWH, 1, BLACK);
      }
      else
      {
        _display.drawRoundRect(_toggleX, y, _toggleWH, _toggleWH, 1, BLACK);
      }
    }
  }
  _display.println();
}

template <class T>
void ArduMenu<T>::_reDraw()
{
  // This function just redraw the screen depending if is necessary
  #ifdef _ADAFRUIT_PCD8544_H
  _display.display();
  #endif
}

template <class T>
void ArduMenu<T>::_setBoxSize()
{
  // Calculating message box sizes and position
  if (_hasBox)
  {
    _boxWidth = _display.width() * SCREEN_BOX_AREA;
    _boxHeight = _display.height() * SCREEN_BOX_AREA;
    _boxXMargin = (_display.width() - _boxWidth) / 2;
    _boxYMargin = (_display.height() - _boxHeight) / 2;
  }
  else
  {
    _boxWidth = _display.width();
    _boxHeight = _display.height();
    _boxXMargin = 0;
    _boxYMargin = 0;
  }
  

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
}

template <class T>
void ArduMenu<T>::_setRangeCurrent(uint16_t num)
{
  if (_boxLines >= 3){
    #ifdef DEBUG
    Serial.println(F("Display box have more than three lines"));
    #endif
    _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin + (_letterH * (_boxLines / 2)));
    char * tmpText = _centerText(num, _boxColumns);
    _display.print(tmpText);
    delete [] tmpText;
  }
  else
  {
    #ifdef DEBUG
    Serial.println(F("Display box have less than three lines"));
    #endif
    _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin + (_letterH * (_boxLines - 1)));
    char * tmpText = _centerText(num, _boxColumns);
    _display.print(tmpText);
    delete [] tmpText;
  }
}

template <class T>
void ArduMenu<T>::_setRangeMetter(uint8_t num)
{
  if (_boxLines >= 3){
    _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin + (_letterH * (_boxLines / 2 + 1)));
    for (int i = 0; i < num; i++)
    {
      _display.write(220);
    }
    for (int i = 0; i < (_boxColumns - num); i++)
    {
      _display.write(219);
    }
  }
}

template <class T>
void ArduMenu<T>::_textBox(bool hasbox)
{
  _hasBox = hasbox;
  _setBoxSize();
}