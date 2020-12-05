# ArduMenu
Create a simple menu for your screen

By Daniel Carrasco

## What does it do?
Allow you to create a simple menu to run functions with every option.

## Compatible displays
For now the module has been tested on the PCD8544 LCD (Nokia 5110), and the ST77XX TFT displays. The tested libraries are:

- PCD8544: [Adafruit PCD8544 Library](https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library)
- ST77XX: [Adarfuit ST7735 Library](https://github.com/adafruit/Adafruit-ST7735-Library)

## What can I do
With this library you'll be able to create simple menus with sub-menus. Every item must be linked to a function to made it work and for now doesn't allow arguments or returns on those functions. The menu navigation is done throught three functions (up, down and enter), wich allows to control the screen with whatever you want, for example bluetooth, serial, rotary encoder, buttons...

## How to use it
To see how to use it, just go to examples folder and check the project/s

## Important
- This library must be loaded after the display library, or will not work propertly. Be sure to place the library header below the display header.

## Extra info
* "Disabled" text is limited to 11 characters for now

## ToDo
* Test more displays
* ~~Configurable letter size~~
* ~~Vertically Centered text on Range Box~~
* ~~Different selection modes (icon, inverted...)~~
* Allow to hide the "exit" item to use another button for example
* ~~Add 2 as argument to range callback, that will be called on exit~~
* ~~Add ability to disable options with personalized text~~
* Configurable colours for tft displays

## Changelog
### 1.2.0
- Rewritten some parts to save about 2k of Program Memory

### 1.1.0
- Now you can disable the menu options.
- Fixed the examples with the new option

### 1.0.1
- Bugfixes, including a memory leak
- Fixed a bug when there's no top menu and a menu option that returns true is selected.
- Now the togle menu option is fully redrawn to avoid errors and made it simple
- Updated examples

### 1.0.0
- First version