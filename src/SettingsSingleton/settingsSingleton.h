#ifndef SETTINGSSINGLETON_H
#define SETTINGSSINGLETON_H
#include <emscripten.h>
#include "../C/StringRelatedFunctions.h"
#include <stdlib.h>

typedef struct SettingsSingleton
{
    char *entry;
    char *exit;
    char *faviconPath;
    bool autoClear;
    bool largeProject;
    bool bundleCSSInHTML;
    bool productionMode;
    bool addBaseTag;
    int devPort;
    int devSocketPort;
} SettingsSingleton;

int EMSCRIPTEN_KEEPALIVE SendSettingsString(char *String);

#endif // !_settingsSingleton