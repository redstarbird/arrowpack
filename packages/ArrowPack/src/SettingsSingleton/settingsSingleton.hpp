#ifndef _settingsSingleton
#define _settingsSingleton
#include <iostream>
#include <string>
class Settings
{
    public:
        std::string entry;
        std::string exit;
        bool autoClear;
        
        Settings(std::string WrappedSettings);
        void InitSettings(std::string WrappedSettings);
};
#endif