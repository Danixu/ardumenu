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

## ToDo
* Test more displays
* ~~Configurable letter size~~
* ~~Vertically Centered text on Range Box~~
* ~~Different selection modes (icon, inverted...)~~
* Allow to hide the "exit" item to use another button for example
* ~~Add 2 as argument to range callback, that will be called on exit~~
* Configurable colours for tft displays