#ifndef _settingsSingleton
#define _settingsSingleton
#include <iostream>
#include <string>
class SettingsSingleton
{
    public:
        std::string entry;
        std::string exit;
        bool autoClear;
        
        SettingsSingleton(std::string WrappedSettings);
        void InitSettings(std::string WrappedSettings);
};
#endif