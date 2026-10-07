#ifndef CONFIG_H
#define CONFIG_H
#include <stdlib.h>

#include "./bridge/Export.h"
#include "./external/cJSON/cJSON.h"
#include "./util/StringUtil.h"

/**
 * @brief Retrieves a config value by key/name.
 *
 * @param SettingName Name of the requested setting.
 * @return `cJSON*` item containing a value accessible via `->valueint`, `->valuestring`, or `->valuedouble`, or
 * `NULL` if unfound/uninitialised. This does not need to be freed.
 */
cJSON *GetSetting(char *SettingName);

#endif  // !_CONFIG_H