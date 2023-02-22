#ifndef SETTINGSSINGLETON_H
#define SETTINGSSINGLETON_H
#include <emscripten.h>
#include "../C/StringRelatedFunctions.h"
#include <stdlib.h>

typedef struct SettingsSingleton
{
    char *entry;
    char *exit;
    bool autoClear;
    bool largeProject;
    bool bundleCSSInHTML;
    bool productionMode;
    int devPort;
    int devSocketPort;
} SettingsSingleton; // need to implement

int EMSCRIPTEN_KEEPALIVE SendSettingsString(char *String);

#endif // !_settingsSingleton