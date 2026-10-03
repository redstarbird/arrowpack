#ifndef SETTINGSSINGLETON_H
#define SETTINGSSINGLETON_H
#include <emscripten.h>
#include <stdlib.h>

#include "./external/cJSON/cJSON.h"
#include "./util/StringUtil.h"

typedef struct SettingsSingleton {
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

#endif  // !_settingsSingleton