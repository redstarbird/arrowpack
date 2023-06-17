/* This file contains very basic functions for using colours in the terminal */
#include "TextColors.h"

void ColorRed() { printf("\x1b[31m"); }
void ColorBlack() { printf("\x1b[30m"); }
void ColorGreen() { printf("\x1b[32m"); }
void ColorYellow() { printf("\x1b[33m"); }
void ColorBlue() { printf("\x1b[34m"); }
void ColorPink() { ColorMagenta(); }
void ColorMagenta() { printf("\x1b[35m"); }
void ColorCyan() { printf("\x1b[36m"); }
void ColorWhite() { printf("\x1b[37m"); }
void ColorReset() { ColorNormal(); }
void ColorNormal() { printf("\x1b[m"); }