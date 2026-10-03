#ifndef CONFIG_H
#define CONFIG_H
#include <emscripten.h>
#include <stdlib.h>

#include "./external/cJSON/cJSON.h"
#include "./util/StringUtil.h"

typedef struct SettingsSingleton {
    cJSON *Settings;
} SettingsSingleton;

int EMSCRIPTEN_KEEPALIVE SendSettingsString(char *String);
cJSON *GetSetting(char *SettingName);

#endif  // !_CONFIG_H