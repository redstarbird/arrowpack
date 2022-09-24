#ifndef STRINGRELATEDFUNCTIONS_H
#define STRINGRELATEDFUNCTIONS_H

#include <string>
#include <stdio.h>
#include <stdbool.h>
#include <emscripten.h>

bool EMSCRIPTEN_KEEPALIVE containsCharacter(char *string, char character); // Checks if string contains a certain character

char EMSCRIPTEN_KEEPALIVE *GetFileExtension(const char *path); // Returns the file extension for the given path without the dot char

char EMSCRIPTEN_KEEPALIVE *getSubstring(char *Text, int StartIndex, int EndIndex); // Returns substring between start and end indexes

#endif // !STRINGFUNCTIONS_H