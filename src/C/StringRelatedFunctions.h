#ifndef STRINGRELATEDFUNCTIONS_H
#define STRINGRELATEDFUNCTIONS_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <emscripten.h>
#include <stdlib.h>
#include "../Regex/RegexFunctions.h"
#include "../SettingsSingleton/settingsSingleton.h"
#include "IntFunctions.h"
#include "TextColors.h"

#define PREPROCESS_DIR "ARROWPACK_TEMP_PREPROCESS_DIR"

int EMSCRIPTEN_KEEPALIVE LastOccurenceOfChar(const char *text, char character);

bool EMSCRIPTEN_KEEPALIVE containsCharacter(char *string, char character); // Checks if string contains a certain character

char EMSCRIPTEN_KEEPALIVE *GetFileExtension(const char *path); // Returns the file extension for the given path without the dot char

char EMSCRIPTEN_KEEPALIVE *getSubstring(char *Text, int StartIndex, int EndIndex); // Returns substring between start and end indexes

char **SplitStringByChar(char *str, const char delimiter);

char EMSCRIPTEN_KEEPALIVE *TurnToFullRelativePath(const char *PATH, char *BasePath);

bool StringToBool(const char *str);

char *EMSCRIPTEN_KEEPALIVE GetBasePath(const char *filename);

extern struct SettingsSingleton Settings;

char *ReplaceSectionOfString(char *string, int start, int end, const char *ReplaceString);

bool EMSCRIPTEN_KEEPALIVE StringStartsWith(const char *string, const char *substring);

char *EntryToExitPath(const char *path);

char *RemoveSubstring(char *string, const char *substring);

char *InsertStringAtPosition(char *OriginalString, char *ReplaceString, int position);

void RemoveSectionOfString(char *str, int start, int end);

bool StringEndsWith(char *str, char *substr);

void RemoveCharFromString(char *str, char c);

bool StringContainsSubstring(const char *string, const char *substring);

char *EntryToPreprocessPath(char *path);

char *CreateUnusedName();

char *AddPreprocessDIR(char *Path);

char *EMSCRIPTEN_KEEPALIVE GetTrueBasePath(const char *filename);

bool IsURL(char *str);

#endif // !STRINGFUNCTIONS_H