#ifndef SETTINGSSINGLETON_H
#define SETTINGSSINGLETON_H
#include <emscripten.h>
#include "../C/StringRelatedFunctions.h"

typedef struct SettingsSingleton
{
    char *entry;
    char *exit;
    bool autoClear;
} SettingsSingleton; // need to implement

void EMSCRIPTEN_KEEPALIVE SendSettingsString(char *String);

#endif // !_settingsSingleton