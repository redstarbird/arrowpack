#ifndef SETTINGSSINGLETON_H
#define SETTINGSSINGLETON_H
#include <emscripten.h>
#include "../C/StringRelatedFunctions.h"

struct SettingsSingleton Settings;

typedef struct SettingsSingleton
{
    char *entry;
    char *exit;
    bool autoClear;
}; // need to implement

static void EMSCRIPTEN_KEEPALIVE SendSettingsString(char *String);

struct SettingsSingleton InitSettings(char *WrappedSettings); // also need to implement

#endif // !_settingsSingleton