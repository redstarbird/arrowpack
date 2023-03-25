#ifndef SETTINGSSINGLETON_H
#define SETTINGSSINGLETON_H
#include <emscripten.h>
#include "../C/StringRelatedFunctions.h"
#include <stdlib.h>
#include "../C/cJSON/cJSON.h"

typedef struct SettingsSingleton
{
    cJSON *Settings;
    /*
    char *entry;
    char *exit;
    char *faviconPath;
    bool autoClear;
    bool largeProject;
    bool bundleCSSInHTML;
    bool productionMode;
    bool addBaseTag;
    int devPort;
    int devSocketPort;*/
    
} SettingsSingleton;

int EMSCRIPTEN_KEEPALIVE SendSettingsString(char *String);
cJSON *GetSetting(char *SettingName);

#endif // !_settingsSingleton