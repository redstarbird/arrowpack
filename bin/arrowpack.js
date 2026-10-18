#!/usr/bin/env node
"use strict";

/**
 * @file The main CLI executable for arrowpack.
 * This file is responsible for interpeting the user's command and internally invoking the arrowpack API.
 * It uses the arrowargs library to register and process the command into flags and data.
 */

const fs = require("fs");
const path = require("path");
const chalk = require("chalk");
const Config = require("../src/node/config.js");
const { performance } = require('perf_hooks');
const arrowpack = require("../src/node/index.js");

// Returns whether an object is empty or not
function ObjectIsEmpty(object) {
    for (var Property in object) {
        if (object.hasOwnProperty(Property))
            return false;
    }
    return true;
}

/**
 * Parses the command line arguments into an object
 * @returns Object containing valid command line arguments
 */
function parseArgs() {
    return require("arrowargs")(process.argv.slice(2))
        .option("c",
            { alias: "config-path", describe: "Path to config file if not in working directory", type: "string" })
        .option("dev",
            { alias: "dev-server", describe: "Starts the arrowpack dev server", type: "boolean", default: false })
        .option("v", { alias: "version", describe: "Display version information", type: "boolean", default: false })
        .option("init",
            { alias: "initialize", describe: "Initialize arrowpack in a project", type: "boolean", default: false })
        .option("forceWasm",
            { describe: "Force usage of the native Wasm module instead of the default N-API module.", type: "boolean", default: false })
        .help()
        .argv;
}

/**
 * Finds and loads the config file into a Config class instance. Validates keys and modifies the config based on command line arguments.
 * @param {Object} argv command line arguments parsed by ArrowArgs
 * @returns {Config} configured config object
 */
function createConfig(argv) {
    /**
     * Get configuration file path
     * If the -c argument is used, use it for the configuration file path.
     * If the provided path is a directory, add the config file name to the end
     */
    var CONFIG_FILE_NAME = "arrowpack.config.js";
    if (argv.c) {
        if (fs.lstatSync(argv.c).isDirectory()) {
            CONFIG_FILE_NAME = path.join(argv.c, CONFIG_FILE_NAME);
        }
        else {
            CONFIG_FILE_NAME = argv.c;
        }
    }

    /*
    * Find the config file
    * Attempt to find the config file with either .mjs or .cjs extension if the .js version cannot be found
    */
    var rawconfigData = null;
    if (!fs.existsSync(CONFIG_FILE_NAME)) {
        var CJSName = CONFIG_FILE_NAME.substring(0, CONFIG_FILE_NAME.length - 3) + ".cjs";
        var EMJSName = CONFIG_FILE_NAME.substring(0, CONFIG_FILE_NAME.length - 3) + ".mjs";
        if (fs.existsSync(CJSName)) {
            CONFIG_FILE_NAME = CJSName;
        }
        else if (fs.existsSync(EMJSName)) {
            CONFIG_FILE_NAME = EMJSName;
        }
        else {
            CONFIG_FILE_NAME = "";
        }
    }

    // Get config file data otherwise use an empty object so the config object inherits default values
    if (CONFIG_FILE_NAME !== "") {
        CONFIG_FILE_NAME = path.join(process.cwd(), CONFIG_FILE_NAME);
        rawconfigData = require(CONFIG_FILE_NAME);
    }
    else {
        rawconfigData = {};
    }

    if (argv.forceWasm) {
        rawconfigData["forceWasm"] = true;
    }

    // Configure internal settings
    if (argv.c) {
        if (!argv.c.endsWith("/")) {
            argv.c += "/";
        }
        rawconfigData["INTERNAL_CONFIG_DIR"] = argv.c;
        rawconfigData["INTERNAL_FULL_CONFIG_PATH"] = path.join(process.cwd(), argv.c)
    }

    // Initialize settings singleton
    return new Config(rawconfigData);
}

async function runCLI() {
    // Parse command line arguments
    const argv = parseArgs();

    // Get config object
    const config = createConfig(argv);

    /* Handle bundler execution based on command line arguments */

    // v: print version and terminate
    if (argv.v) {
        const version = require("../package.json").version;
        console.log(version);
    }
    // init: initialise directory for arrowpack
    else if (argv.init) {
        const initialise = require("../src/node/initialise.js");
        initialise();
    }
    // dev: start dev server, this will automatically bundle when the server initially starts
    else if (argv.dev) {
        console.log("Entering dev mode");

        const server = await arrowpack.createServer(config.settings);

        // Handle clean shutdown of server upon Ctrl + C
        process.on("SIGINT", async () => {
            console.log("Stopping dev server...");
            await server.stop();
            process.exit(0);
        })
    }
    // Standard control flow, one-off bundle
    else {
        await Bundle(config);
    }
}

/** 
* CLI wrapper for the arrowpack API build function
* @param {Config} config arrowpack configuration object from `runCLI`
*/
async function Bundle(config) {
    // Store start time to track bundling time
    var StartTime = performance.now();

    await arrowpack.build(config.settings)

    // Output bundling time
    console.log(chalk.magentaBright("\n\nBundling files completed in " + (performance.now() - StartTime) / 1000 +
        " seconds\n\n"));
}

/* Preprocess directory cleanup */

// Delete the preprocess directory on sigterm
process.on("SIGTERM", () => {
    print("Exiting due to SIGTERM, deleting temp directory...");
    DeletePreprocessDir();
})

// Delete the preprocess directory on exit
process.on("exit", () => {
    DeletePreprocessDir();
});

// Delete the preprocess directory on force stop from user
process.on("SIGINT", () => {
    var DelResult = DeletePreprocessDir();
    process.exit(DelResult);
});

// Deletes the preprocess directory
function DeletePreprocessDir() {
    fs.rm("ARROWPACK_TEMP_PREPROCESS_DIR", { recursive: true }, (err) => {
        if (err) {
            console.error(err);
            return 1;
        }
        else {
            console.log("Sucessfully removed temporary preprocess directory");
        }
        return 0;
    });
}

runCLI();

/*
 * ===================================================================
 * Old plugin code
 * This will soon be removed/revamped into an improved plugin system.
 * ===================================================================
*/

// Caches used plugins so they don't need to be reloaded every time they are used during the dev server
const PluginsCache = {};

// Function for transforming files that is called from C code
function JSTransformFiles(EncodedOriginalContents, PluginPath) {
    const OriginalFileContents = CFunctions.UTF8ToString(EncodedOriginalContents);
    PluginPath = CFunctions.UTF8ToString(PluginPath);

    if (!PluginsCache[PluginPath]) {  // Make sure that the plugin is in the cache
        try {
            PluginsCache[PluginPath] = require(PluginPath);
        } catch (error) {
            throw "Error loading plugin: " + PluginPath + "\n\n" + error;
        }
    }
    const Transformer = PluginsCache[PluginPath];
    const FileContents = Transformer(OriginalFileContents);  // Run the transformation on the file contents
    if (FileContents === OriginalFileContents) {             // Don't waste time encoding strings if the file
        // contents haven't changed
        return null;
    }
    // Encode string into UTF8 encoding
    var lengthBytes = CFunctions.lengthBytesUTF8(FileContents) + 1;
    var stringOnWasmHeap = CFunctions._malloc(lengthBytes);
    CFunctions.stringToUTF8(FileContents, stringOnWasmHeap, lengthBytes);
    return stringOnWasmHeap;  // Return the encoded string
}

// Function for validating files
async function JSValidateFiles(FileContents, PluginPath, FilePath) {
    // Decode UTF8 string into JS string
    PluginPath = CFunctions.UTF8ToString(PluginPath);
    FilePath = CFunctions.UTF8ToString(FilePath);
    FileContents = CFunctions.UTF8ToString(FileContents);

    let imported = false;
    if (!PluginsCache[PluginPath]) {  // Ensure the plugin is in the cache
        try {
            await (async () => {
                PluginsCache[PluginPath] = (await import(PluginPath)).default;
                imported = true;
                console.log(PluginsCache[PluginPath]);
            })();
        } catch (error) {
            throw "Error loading plugin: " + PluginPath + "\n\n" + error;
        }
    }
    else {
        imported = true;
    }

    const Validator = PluginsCache[PluginPath];
    await (async () => {
        const Results = await Validator.Validate(FileContents, FilePath);  // Run validator
        if (Results.warnings) {                                            // Check for warnings
            if (Results.warnings.length > 0) {
                for (let i = 0; i < Results.warnings.length; i++) {
                    console.log("Warning: " + Results.warnings[i]);
                }
            }
        }
        if (Results.errors) {  // Check for errors
            if (Results.errors.length > 0) {
                for (let i = 0; i < Results.errors.length; i++) {
                    console.log("Error: " + Results.errors[i]);
                }
                process.exit(1);
            }
        }
        return null;
    })();
    return null;
}