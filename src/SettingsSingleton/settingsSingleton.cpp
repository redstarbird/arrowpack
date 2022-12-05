#include <settingsSingleton.hpp>

SettingsSingleton::SettingsSingleton(std::string WrappedSettings)
{        
    if (!WrappedSettings.empty()) { // allows class to be defined and then properly initialized later
            int lastSplitIndex,lastColonIndex,NextSplitIndex = 0;
            std::pair<int,int> lastLocations(0,0);

            for (int i = 0; i < WrappedSettings.length(); i++) {

                if (NextSplitIndex != lastLocations.second) {
                    
                }
            }
        }
}

void SettingsSingleton::InitSettings(std::string WrappedSettings) {
    if (!WrappedSettings.empty()) {

    } else {throw std::invalid_argument("String cannot be empty");}

}

// Need to make code work with DRY more