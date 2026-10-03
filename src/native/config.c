#include "config.h"

/**
 * Config is stored as a global singleton struct.
 * The settings values themselves are stored within a cJSON struct which
 * is copied from the node config.js JSON
 */
typedef struct SettingsSingleton {
    cJSON *Settings;
} SettingsSingleton;

/**
 * Global config wrapper
 * Setup by InitSettings
 * Accessible via GetSetting()
 */
struct SettingsSingleton Settings;

/**
 * @brief Parses and stores the user's config JSON
 * Called from bin/arrowpack.js with a stringified version of the user's config JSON
 * Returns true or false to signify successful parsing
 */
bool EMSCRIPTEN_KEEPALIVE InitSettings(char *JSON)
{
    Settings.Settings = cJSON_Parse(JSON);
    return Settings.Settings != NULL;
}

cJSON EMSCRIPTEN_KEEPALIVE *GetSetting(char *SettingName)
{
    return cJSON_GetObjectItemCaseSensitive(Settings.Settings, SettingName);
}
