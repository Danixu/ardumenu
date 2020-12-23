#include <Adafruit_GFX.h> // Core graphics library
#include <Adafruit_PCD8544.h> // Hardware-specific library
#include <SPI.h>
#include "ArduMenu.h" // Menu class header. Is important to load this library after the display library.

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Functions definitions must be done before the menu definition
bool redLedOn(MENU_ITEM);
bool redLedOff(MENU_ITEM);
bool allLedsOff(MENU_ITEM);
bool greenLedToggle(bool);
int whiteLedRange(int8_t);
int whiteLedRangeOnExit(int8_t);

// ArduMenu:
//
// NOTES: Is important to use PROGMEM, because it helps to save SRAM and is required by menu class
// More info: https://www.arduino.cc/reference/en/language/variables/utilities/progmem/
//
// Items should be MENU_ITEM objects which have the following members
//   Type: the menu type, which can be AM_ITEM_TYPE_HEADER, AM_ITEM_TYPE_MENU, AM_ITEM_TYPE_COMMAND, AM_ITEM_TYPE_RANGE, AM_ITEM_TYPE_TOGGLE or AM_ITEM_TYPE_EOM
//      AM_ITEM_TYPE_HEADER: This item is the menu header. Only can be the first item (other will be ignored), and is not selectable.
//      AM_ITEM_TYPE_MENU: A menu item that allow you to add a submenu. There is no level limit.
//      AM_ITEM_TYPE_COMMAND: This menu item just runs a function when is selected
//      AM_ITEM_TYPE_RANGE: Shows a range box that allows you to set a number. The callback function will receive 0 as argument to get the current status,
//                          1 to sum, and -1 to subtract. This callback function must return the current number in range after the operations.
//      AM_ITEM_TYPE_TOGGLE: A toggle item will call the function every time is pressed like AM_ITEM_TYPE_COMMAND, but it expect a boolean as return to set the icon.
//                           The callback function will be called with false as argument to get the current status, and true to change the status.
//      AM_ITEM_TYPE_EOM: This is an IMPORTANT!!! item. Is like an AM_ITEM_TYPE_COMMAND, but sets the end of the menu. Is always required as last item or the menu will not work.
//   Text: A char array that contains the text used in menu item. PROGMEM is required or will not work.
//   Function: Callback function used on AM_ITEM_TYPE_COMMAND and AM_ITEM_TYPE_EOM modes.
//   SubItems: A MENU_ITEM object that is used on AM_ITEM_TYPE_MENU items.
//   rangeManage: Callback function to manage the range item. Is used on AM_ITEM_TYPE_RANGE.
//   toggleManage: Callback function to manage the toggle item. Is used on AM_ITEM_TYPE_TOGGLE
//   disabled: Boolean to set the menu item as disabled
//   disabledText: const char PROGMEM which contains the text that will be sown when the option is disabled, limited to 11 characters for now. If not set, a defatult message will be used
//   
// Class methods:
//   drawMenu: Draw the current menu on the display
//   down: On menu will select the next item. On range will call the callback function with 1 as argument.
//   up: On menu will select the previous item. On range will call the callback function with -1 as argument.
//   enter: Select the current menu item. On AM_ITEM_TYPE_COMMAND and AM_ITEM_TYPE_EOM, callback function will be called. On AM_ITEM_TYPE_COMMAND if the callback function returns true, the top menu will be drawn.
//                                        On AM_ITEM_TYPE_MENU the SubItems menu will be shown.
//                                        On AM_ITEM_TYPE_RANGE, a range box will be shown. The callback function will be called with 0 as argument to retreive the current status. On exit function callback will be called with 32767 as argument.
//                                        On AM_ITEM_TYPE_TOGGLE, the callback function will be called with true as argument, and the returned bool will be used to redraw the icon.
//   setSelectionMode: Se the selection mode between AM_SELECTION_MODE_ICON and AM_SELECTION_MODE_INVERTED. AM_SELECTION_MODE_ICON accepts a second argument, that allows to change the char used as icon.
//   setTextSize: Sets the menu text size (default 1).
//
// Class variables:
//   inRange: This variable allows you to know then a range item is selected, and the user is currently setting the range.
// 

// On Off Menu
const char onOffMenu00 [] PROGMEM = { "On"   };
const char onOffMenu01 [] PROGMEM = { "Off"  };
const char onOffMenu02 [] PROGMEM = { "Back" };
MENU_ITEM onOffMenu[]= {
  // byte Type, char Text, bool Function, MENU_ITEM SubItems, int rangeManage, bool toggleManage
  { AM_ITEM_TYPE_COMMAND, onOffMenu00, redLedOn,  NULL, NULL, NULL, NULL, NULL },
  { AM_ITEM_TYPE_COMMAND, onOffMenu01, redLedOff, NULL, NULL, NULL, NULL, NULL },
  { AM_ITEM_TYPE_EOM,     onOffMenu02, NULL,      NULL, NULL, NULL, NULL, NULL },
};


// Main Menu
// Also PROGMEM is required
const char mainMenu00 [] PROGMEM = { "Main Menu"   };
const char mainMenu01 [] PROGMEM = { "Red Led"     };
const char mainMenu02 [] PROGMEM = { "Green Led"   };
const char mainMenu03 [] PROGMEM = { "Blue Led"    };
const char mainMenu04 [] PROGMEM = { "Blue Led OE" };
const char mainMenu05 [] PROGMEM = { "All Off"     };

MENU_ITEM mainMenu[] = {
  // byte Type, char Text, bool Function, MENU_ITEM SubItems, int rangeManage, bool toggleManage  
  { AM_ITEM_TYPE_HEADER, mainMenu00, NULL,       NULL,      NULL,                NULL          , NULL, NULL },
  { AM_ITEM_TYPE_MENU,   mainMenu01, NULL,       onOffMenu, NULL,                NULL          , NULL, NULL },
  { AM_ITEM_TYPE_TOGGLE, mainMenu02, NULL,       NULL,      NULL,                greenLedToggle, NULL, NULL },
  { AM_ITEM_TYPE_RANGE,  mainMenu03, NULL,       NULL,      whiteLedRange,       NULL          , NULL, NULL },
  { AM_ITEM_TYPE_RANGE,  mainMenu04, NULL,       NULL,      whiteLedRangeOnExit, NULL          , NULL, NULL },
  { AM_ITEM_TYPE_EOM,    mainMenu05, allLedsOff, NULL,      NULL,                NULL          , NULL, NULL },
};


// First you must declare the object in global scope to allow to use it on the whole program
// Screen object type will be used as template argument
ArduMenu<Adafruit_PCD8544> * menuObject;

// Control variables
int whiteLedStatus = 0;
bool greenLedState = false;
#define STEPS 10
#define GREENLED 8
#define REDLED 9
#define WHITELED 10

void setup()
{
  display.begin();
  display.setContrast(50);
  display.clearDisplay();
  display.display();

  // Initialize the object to use it. Is important to do it after the screen initialization, or will fail.
  menuObject = new ArduMenu<Adafruit_PCD8544>(mainMenu, display);

  pinMode(GREENLED, OUTPUT);
  pinMode(REDLED, OUTPUT);
  pinMode(WHITELED, OUTPUT);

  // Draw the menu
  menuObject->drawMenu();

  // Red led On/Off (Submenu)
  delay(1000);
  // Enter on submenu
  menuObject->enter();
  delay(1000);
  // Select On function
  menuObject->enter();
  delay(1000);
  // The return of the function is true, so we are in main menu again
  // Enter into led menu again
  menuObject->enter();
  delay(1000);
  // Go down to off option
  menuObject->down();
  delay(1000);
  // Select the off option
  menuObject->enter();
  delay(1000);
  // Now we are again on main menu again, setting the led to on again
  menuObject->enter();
  delay(1000);
  menuObject->enter();

  // Green led On/Off (Toggle)
  delay(1000);
  // Select the green led toggle
  menuObject->down();
  delay(1000);
  // Turn it on
  menuObject->enter();
  delay(1000);
  // Turn it off
  menuObject->enter();
  delay(1000);
  // Turn it on again
  menuObject->enter();

  // Blue led dim in real time (Range)
  // The callback function set to this menu item, always sets the
  // led status, so changes are updated in real time.
  delay(1000);
  menuObject->down();
  delay(1000);
  // On this enter call we include the min and max range values to 
  // show the meter.
  menuObject->enter(0, 255);
  delay(1000);
  // Same when we want to change the value, to update the range meter in real time
  for (int i = 0; i < 14; i++)
  {
    delay(500);
    menuObject->up(0, 255);
  }
  for (int i = 0; i < 14; i++)
  {
    delay(500);
    menuObject->down(0, 255);
  }
  // On exit call range is not required because the metter is hidden.
  menuObject->enter();

  // Blue led dim on set (Range)
  // This function is similar to real time function, but this one only sets the led
  // bright when 32767 is received as argument (On range Exit).
  delay(1000);
  menuObject->down();
  delay(1000);
  menuObject->enter(0, 255);
  delay(1000);
  for (int i = 0; i < 14; i++)
  {
    delay(500);
    menuObject->up(0, 255);
  }
  delay(1000);
  // Sets the current value as acepted to call the function with 32767 as argument
  menuObject->enter();
  delay(1000);
  // Enter in range option to change it again
  menuObject->enter(0, 255);
  delay(1000);
  for (int i = 0; i < 14; i++)
  {
    delay(500);
    menuObject->down(0, 255);
  }
  delay(1000);
  // As above, this enter will call the callback function with 32767 as argument
  menuObject->enter();

  // Go down to exit function, which is called "All Off" in the example
  // Yo can call a function to hide the menu for example...
  delay(1000);
  menuObject->down();
  delay(1000);
  menuObject->enter();
}

void loop() {
}

bool redLedOn(MENU_ITEM item)
{
  digitalWrite(REDLED, HIGH);
  // Return true makes the menu to go back 
  return true;
}
bool redLedOff(MENU_ITEM item)
{
  digitalWrite(REDLED, LOW);
  // Return true makes the menu to go back 
  return true;
}
bool allLedsOff(MENU_ITEM item)
{
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);
  analogWrite(WHITELED, 0);
  // Return false keeps the current menu 
  return false;
}

int whiteLedRange(int8_t mode)
{
  if (mode == 1 || mode == -1)
  {
    whiteLedStatus += (mode * STEPS);
    if (whiteLedStatus > 255)
    {
      whiteLedStatus = 255;  
    }
    else if (whiteLedStatus < 0)
    {
      whiteLedStatus = 0;
    }
  }
  analogWrite(WHITELED, whiteLedStatus);
  return whiteLedStatus;
}

int whiteLedRangeOnExit(int8_t mode)
{
  if (mode == 1 || mode == -1)
  {
    whiteLedStatus += (mode * STEPS);
    if (whiteLedStatus > 255)
    {
      whiteLedStatus = 255;  
    }
    else if (whiteLedStatus < 0)
    {
      whiteLedStatus = 0;
    }
  }
  if (mode == 32767)
  {
    analogWrite(WHITELED, whiteLedStatus);
  }
  return whiteLedStatus;
}

bool greenLedToggle(bool mode)
{
  if (mode)
  {
    greenLedState = !greenLedState;
    if (greenLedState)
    {
      digitalWrite(GREENLED, HIGH);
    }
    else
    {
      digitalWrite(GREENLED, LOW);
    }
    return greenLedState;
  }
  else
  {
    return greenLedState;
  }
}
