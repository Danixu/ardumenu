#include "Arduino.h"
#include "ArduMenu.h"

#ifdef _ADAFRUIT_PCD8544_H
ArduMenu::ArduMenu(MENU_ITEM *menu, Adafruit_PCD8544 display):
  inRange(false),
  _currentMenuItemIdx(0),
  _itemsOffset(0),
  _currentMenuTable(menu),
  _lines(SCREEN_LINES_TOTAL),
  _display(display)
{
  // Calculating message box sizes and position
  _boxWidth = _display.width() * SCREEN_BOX_AREA;
  _boxHeight = _display.height() * SCREEN_BOX_AREA;
  _boxXMargin = (_display.width() - _boxWidth) / 2;
  _boxYMargin = (_display.height() - _boxHeight) / 2;
  _boxLines = _boxHeight / SCREEN_LETTER_H;
  _boxColumns = _boxWidth / SCREEN_LETTER_W;
  _boxLinesYMargin = _boxYMargin + (_boxHeight - (_boxLines * SCREEN_LETTER_H)) / 2;
  _boxColumnsXMargin = _boxXMargin + (_boxWidth - (_boxColumns * SCREEN_LETTER_W)) / 2;
  _haveBox = true;

  // Adjusting size if there's no enough space
  if ((_boxLinesYMargin - _boxYMargin) < 2)
  {
    _boxLines -= 1;
    _boxLinesYMargin = _boxYMargin + (_boxHeight - (_boxLines * SCREEN_LETTER_H)) / 2;
  }
  if ((_boxColumnsXMargin - _boxXMargin) < 2)
  {
    _boxColumns -= 1;
    _boxColumnsXMargin = _boxXMargin + (_boxWidth - (_boxColumns * SCREEN_LETTER_W)) / 2;
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
#endif

void ArduMenu::drawMenu()
{
  _display.clearDisplay();
  _display.setCursor(0, 0);
  
  if (_currentMenuTable[0].Type == AM_ITEM_TYPE_HEADER)
  {
    auto len = strlen_P(_currentMenuTable[0].Text);
    char txt[len + 1] = {};
    memcpy_P(txt, _currentMenuTable[0].Text, len);
    char * tmpText = _centerText(txt, 12);
    char text[15];
    snprintf_P(text, 15, PSTR("*%s*"), tmpText);
    delete [] tmpText;
    _display.setTextColor(BLACK, WHITE);
    _display.println(text);
    if (_currentMenuItemIdx == 0)
    {
      _lines = SCREEN_LINES_TOTAL - 1;
      _itemsOffset++;
      _currentMenuItemIdx = 1;
    }
  }
  
  // This is a bit tricky and maybe will change in the future.
  // To keep a track of the top menus, the pointer must be saved
  // somewhere, and the "back" options is where I've choose
  for (byte i = 0; i < 99; i++)
  {
    if (_currentMenuTable[i].Type == AM_ITEM_TYPE_EOM)
    {
      _currentMenuTable[i].SubItems = _oldMenuTable;
      break;
    }
  }

  // Running through the menu items
  for (byte i = 0; i < _lines; i++)
  {
    if (_currentMenuTable[i + _itemsOffset].Type != AM_ITEM_TYPE_HEADER)
    {
      _display.setTextColor(BLACK, WHITE);
      char text[SCREEN_COLUMNS + 1];
      char format[14];
      
      snprintf_P(format, 14, PSTR(" %%-%d.%ds "), SCREEN_COLUMNS - 3, SCREEN_COLUMNS - 3);
      auto len = strlen_P(_currentMenuTable[i + _itemsOffset].Text);
      char txt[len+1] = {};
      memcpy_P(txt, _currentMenuTable[i + _itemsOffset].Text, len);
      snprintf(text, SCREEN_COLUMNS + 1, format, txt);
      _display.print(text);
      if (_currentMenuTable[i + _itemsOffset].Type == AM_ITEM_TYPE_MENU)
      {  
        _display.write(175);
      }
      _display.println();

      if (_currentMenuTable[i + _itemsOffset].Type == AM_ITEM_TYPE_EOM)
      {
        break;
      }
    }
  }
  // Setting selector to selected item
  if (_currentMenuItemIdx > _lines)
  {
    _display.setCursor(0, SCREEN_LETTER_H * _lines);
  }
  else
  {
    _display.setCursor(0, SCREEN_LETTER_H * _currentMenuItemIdx);
  }
  _display.write(16);
  _display.display();
}

void ArduMenu::down()
{
  ArduMenu::down(NULL, NULL);
}

void ArduMenu::down(int min, int max)
{
  if (inRange)
  {
    (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(-1);
    int currentStep = (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(0);
    Serial.print("Current step: ");
    Serial.println(currentStep);

    _setRangeCurrent(currentStep);
    if (min != max)
    {
      int range = max - min;
      int n = (float)(currentStep - min) / range * _boxColumns;
      _setRangeMetter(n);
    }
    _display.display();
  }
  else
  {
    if (_currentMenuTable[_currentMenuItemIdx].Type != AM_ITEM_TYPE_EOM)
    {
      if ((_currentMenuItemIdx - _itemsOffset) >= (_lines - 1))
      {
        _itemsOffset++;
        _currentMenuItemIdx += 1;
        drawMenu();
      }
      else
      {
        _display.setCursor(0, SCREEN_LETTER_H * _currentMenuItemIdx);
        _display.print(F(" "));
        _currentMenuItemIdx += 1;
        _display.setCursor(0, SCREEN_LETTER_H * _currentMenuItemIdx);
        _display.write(16);
        _display.display();
      }
    }
  }
}

void ArduMenu::up()
{
  ArduMenu::up(NULL, NULL);
}

void ArduMenu::up(int min, int max)
{
  if (inRange)
  {
    (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(1);
    int currentStep = (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(0);
    _setRangeCurrent(currentStep);
    if (min != max)
    {
      int range = max - min;
      int n = (float)(currentStep - min) / range * _boxColumns;
      _setRangeMetter(n);
    }
    _display.display();
    Serial.print("Current step: ");
    Serial.println(currentStep);
  }
  else
  {
    if (_currentMenuItemIdx != 0 && _currentMenuTable[_currentMenuItemIdx - 1].Type != AM_ITEM_TYPE_HEADER)
    {
      if (_itemsOffset >= _currentMenuItemIdx)
      {
        _itemsOffset--;
        _currentMenuItemIdx -= 1;
        drawMenu();
      }
      else
      {
        _currentMenuItemIdx -= 1;
        if (_currentMenuTable[0].Type == AM_ITEM_TYPE_HEADER)
        {
          _display.setCursor(0, SCREEN_LETTER_H * (_currentMenuItemIdx + 2 - _itemsOffset));
          _display.print(F(" "));
          _display.setCursor(0, SCREEN_LETTER_H * (_currentMenuItemIdx + 1 - _itemsOffset));
        }
        else
        {
          _display.setCursor(0, SCREEN_LETTER_H * (_currentMenuItemIdx + 1 - _itemsOffset));
          _display.print(F(" "));
          _display.setCursor(0, SCREEN_LETTER_H * (_currentMenuItemIdx - _itemsOffset));
        }
        _display.write(16);
        _display.display();
      }
    }
  } 
}

void ArduMenu::enter()
{
  switch(_currentMenuTable[_currentMenuItemIdx].Type)
  {
    //
    // The item is a menu... Displaying the submenu.
    //
    case AM_ITEM_TYPE_MENU:
    {
      if (_currentMenuTable[_currentMenuItemIdx].SubItems != NULL)
      {
        _oldMenuTable = _currentMenuTable;
        _currentMenuTable = _currentMenuTable[_currentMenuItemIdx].SubItems;
        _itemsOffset = 0;
        _currentMenuItemIdx = 0;
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
          Serial.println("Exiting range...");
          inRange = false;
          drawMenu();
        }
        else
        {
          Serial.println(F("Range item detected..."));
          inRange = true;
          int currentStep = (*_currentMenuTable[_currentMenuItemIdx].rangeManage)(0);
          if (_haveBox){
            _display.fillRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, WHITE);
            _display.drawRect(_boxXMargin, _boxYMargin, _boxWidth, _boxHeight, BLACK);
          }

          if (_boxLines >= 2)
          {
            _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin);
            auto len = strlen_P(_currentMenuTable[_currentMenuItemIdx].Text);
            char txt[len + 1] = {};
            memcpy_P(txt, _currentMenuTable[_currentMenuItemIdx].Text, len);
            char * tmpText = _centerText(txt, _boxColumns);
            _display.print(tmpText);
            delete [] tmpText;
          }

          _setRangeCurrent(currentStep);

          Serial.print("Current step: ");
          Serial.println(currentStep);
          _display.display();
        }
      }
      break;
    }

    //
    // The item is a function
    //
    case AM_ITEM_TYPE_COMMAND:
    {
      (_currentMenuTable[_currentMenuItemIdx].Function)(&_currentMenuTable[_currentMenuItemIdx]);
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
      
      if (_currentMenuTable[_currentMenuItemIdx].Function != NULL)
      {
        (_currentMenuTable[_currentMenuItemIdx].Function)(&_currentMenuTable[_currentMenuItemIdx]);
      }
      else
      {
        Serial.println(F("Back button has no function"));
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

void ArduMenu::_setRangeCurrent(int num)
{
  if (_boxLines >= 3){
    _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin + (SCREEN_LETTER_H * (_boxLines - 2)));
    _display.print(_centerText(num, _boxColumns));
  }
  else
  {
    _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin + (SCREEN_LETTER_H * (_boxLines - 1)));
    _display.print(_centerText(num, _boxColumns));
  }
}

void ArduMenu::_setRangeMetter(int num)
{
  if (_boxLines >= 3){
    _display.setCursor(_boxColumnsXMargin, _boxLinesYMargin + (SCREEN_LETTER_H * (_boxLines - 1)));
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

char * ArduMenu::_centerText(int num)
{
  return _centerText(num, SCREEN_COLUMNS + 1);
}

char * ArduMenu::_centerText(int num, unsigned int length)
{
  char text[7] = "";
  snprintf_P(text, 7, PSTR("%d"), num);
  return ArduMenu::_centerText(text, length);
}

char * ArduMenu::_centerText(const char *text)
{
  return _centerText(text, SCREEN_COLUMNS + 1);
}

char * ArduMenu::_centerText(const char *text, unsigned int length)
{
  unsigned int N = strlen(text);
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
    short middle = (length - N) / 2;
    char format[11];
    snprintf_P(format, 11, PSTR("%%%ds%%%ds"), middle + N, length - (middle + N));
    char * finalText = new char[length + 1]{};
    snprintf(finalText, length + 1, format, text, "");
    return finalText;
  }
}