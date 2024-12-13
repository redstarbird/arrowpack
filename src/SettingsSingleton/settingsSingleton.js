"use strict";

const fs = require("fs");

// The settings singleton containing settings and configuration
class settingsSingleton {
	constructor(json) {
		this.settings = {
			entry: "src/",
			exit: "public/",
			autoClear: false,
			largeProject: false,
			bundleCSSInHTML: true,
			productionMode: true,
			devPort: 8080,
			devSocketPort: 8081,
			addBaseTag: false,
			faviconPath: "",
			transformers: {},
			resolvers: [],
			validators: {},
			postProcessors: {},
			INTERNAL_CWD: process.cwd(),
			INTERNAL_CONFIG_DIR: "",
			INTERNAL_FULL_CONFIG_PATH: ""
		};

		/*if (fs.existsSync("./lib")) {
			this.settings.entry = "lib";
		}*/
		if (fs.existsSync("./dist")) { // Try to automatically detect exit directory
			this.settings.exit = "dist/";
		} else if (fs.existsSync("./build")) {
			this.settings.exit = "build/";
		}

		if (json !== undefined && json !== null) { // Apply settings from config file
			// json = JSON.parse(json);
			for (var key in json) {
				if (this.settings[key] !== undefined) {
					this.settings[key] = json[key];
				}
			}
		}

		var AddFileSeperatorToEnd = ["entry", "exit"];
		for (let i = 0; i < AddFileSeperatorToEnd.length; i++) { // Adds file path separator to end of each path in settings
			if (this.settings[AddFileSeperatorToEnd[i]][this.settings[AddFileSeperatorToEnd[i]].length - 1] !== "/") {
				this.settings[AddFileSeperatorToEnd[i]] += "/";
			}
		}
		//console.log(this.settings);
	}
	getValue(key, required) { // Will throw an error if the key is not present and required is true
		if (this.settings[key] !== undefined) { return this.settings[key]; } else { if (required === true) { throw "Error: could not find required key :("; } else { return null; } }
	}
}

module.exports = settingsSingleton;