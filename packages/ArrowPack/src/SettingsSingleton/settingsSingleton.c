#include "settingsSingleton.h"

struct SettingsSingleton Settings = {.entry = "src", .exit = "dist"}; // Initialises global settings structure

static bool LastStringWasKey;
static char *LastKey;
static int KeysSent = 0;

bool StringToBool(const char *str)
{
    return strcasecmp(str, "true") == 0;
}

int SetSetting(char *key, char *value)
{ // Returns 1 if successful, 0 otherwise
    if (strcasecmp(key, "Entry"))
    {
        Settings.entry = value;
    }
    else if (strcasecmp(key, "exit"))
    {
        Settings.exit = value;
    }
    else if (strcasecmp(key, "autoClear"))
    {
        Settings.autoClear = StringToBool(value);
    }
    else
    {
        return 0;
    }
    return 1;
}

void EMSCRIPTEN_KEEPALIVE SendSettingsString(char *String)
{
    if (!LastStringWasKey)
    {
        strcpy(LastKey, String);
    }
    else
    {
        if (SetSetting(LastKey, String) == 0)
        {
            printf("Error applying settings %s with value %s with Wasm!\n", LastKey, String);
        }
    }
    LastStringWasKey = !LastStringWasKey;
}
