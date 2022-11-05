#ifndef STRINGRELATEDFUNCTIONS_H
#define STRINGRELATEDFUNCTIONS_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <emscripten.h>
#include <stdlib.h>
#include "../Regex/RegexFunctions.h"
#include "../SettingsSingleton/settingsSingleton.h"

bool EMSCRIPTEN_KEEPALIVE containsCharacter(char *string, char character); // Checks if string contains a certain character

char EMSCRIPTEN_KEEPALIVE *GetFileExtension(const char *path); // Returns the file extension for the given path without the dot char

char EMSCRIPTEN_KEEPALIVE *getSubstring(char *Text, int StartIndex, int EndIndex); // Returns substring between start and end indexes

char **SplitStringByChar(char *str, const char delimiter);

char EMSCRIPTEN_KEEPALIVE *TurnToFullRelativePath(char *path, char *BasePath);

bool StringToBool(const char *str);

extern struct SettingsSingleton Settings;

#endif // !STRINGFUNCTIONS_H