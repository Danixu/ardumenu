# ArduMenu
Create a simple menu for your screen

By Daniel Carrasco

## What does it do?
Allow you to create a simple menu to run functions with every option.

## Compatible displays
For now the module is only compatible with the PCD8544 LCD (Nokia 5110), but I'm working in make it compatible with the ST7735R TFT display. The library used for the display are:

- PCD8544: Adafruit PCD8544 Library

## What can I do
With this library you'll be able to create simple menus with sub-menus. Every item must be linked to a function to made it work and for now doesn't allow arguments or returns on those functions. The menu navigation is done throught three functions (up, down and enter), wich allows to control the screen with whatever you want, for example bluetooth, serial, rotary encoder, buttons... 

## Important
This library must be loaded after the LCD library, because uses some functions available on that library.
