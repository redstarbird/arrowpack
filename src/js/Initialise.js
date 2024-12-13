const settingsSingleton = require("../SettingsSingleton/settingsSingleton");
const fs = require("fs");
resolve = require("path").resolve;

function initialise() {
    const sampleSettings = new settingsSingleton(null);
    // Delete internal settings
    delete sampleSettings.settings["INTERNAL_CWD"];
    delete sampleSettings.settings["INTERNAL_CONFIG_DIR"];
    delete sampleSettings.settings["INTERNAL_FULL_CONFIG_PATH"];

    // Ensure that production mode is false
    sampleSettings.settings["productionMode"] = false;

    let ableToCreate = true;
    let configPath = "./arrowpack.config.js";
    // Check that a configuration file doesn't already exist
    if (fs.existsSync(configPath)) {
        console.warn("Cannot create config file, config file already exists!");
        ableToCreate = false;
    }

    if (ableToCreate) {
        try {
            fs.writeFileSync(configPath, "module.exports = " + JSON.stringify(sampleSettings.settings, null, 4));
        } catch (e) {
            console.error("Couldn't write file due to error: " + e);
        }
    }
}

module.exports = initialise;