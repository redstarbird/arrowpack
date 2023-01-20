#include "settingsSingleton.h"

struct SettingsSingleton Settings = {
    .entry = "src",
    .exit = "dist",
    .autoClear = false,
    .largeProject = false,
    .bundleCSSInHTML = true}; // Initialises global settings structure

static bool LastStringWasKey;
static size_t LastKeyLength = 0;
static char *LastKey;

static int SetSetting(char *key, char *value)
{ // Returns 1 if successful, 0 otherwise
    if (strcasecmp(key, "entry") == 0)
    {
        printf("settings entry to %s\n", value);
        Settings.entry = value;
    }
    else if (strcasecmp(key, "exit") == 0)
    {
        Settings.exit = value;
    }
    else if (strcasecmp(key, "autoClear") == 0)
    {
        Settings.autoClear = StringToBool(value);
    }
    else if (strcasecmp(key, "largeProject") == 0)
    {
        Settings.largeProject = StringToBool(value);
    }
    else if (strcasecmp(key, "bundlecssinhtml") == 0)
    {
        Settings.bundleCSSInHTML = StringToBool(value);
    }
    else if (strcasecmp(key, "productionMode") == 0)
    {
        Settings.productionMode = StringToBool(value);
    }

    else
    {
        return 0;
    }
    return 1;
}

int EMSCRIPTEN_KEEPALIVE SendSettingsString(char *String)
{
    printf("Received settings string: %s,Last key: %s, Last string was key: %i \n", String, LastKey, LastStringWasKey);
    if (LastStringWasKey == false)
    {
        LastKey = strdup(String);
    }
    else
    {
        if (SetSetting(LastKey, strdup(String)) == 0)
        {
            CreateWarning("Error applying settings %s with value %s with Wasm!\n", LastKey, String);
            return 0;
        }
    }
    LastStringWasKey = !LastStringWasKey;
    return 1; // Allows javascript to know when the function has been completed
}
