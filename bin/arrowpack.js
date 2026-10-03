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
const config = require("../src/node/config.js");
const DirFunctions = require("../src/node/util/FSUtil.js");
const CFunctionFactory = require("../build/CFunctions.js");
const chokidar = require('chokidar');
const {ArrowDeserialize} = require("../src/node/util/serialize.cjs");
const {performance} = require('perf_hooks');

// Track the start time to track bundle time
var StartTime = performance.now();

// Currently unused helper function
function requireModule(modulePath, exportName)
{
    try {
        const imported = require(modulePath);
        return exportName ? imported[exportName] : imported;
    } catch (err) {
        return err.code;
    }
}

// Returns whether an object is empty or not
function ObjectIsEmpty(object)
{
    for (var Property in object) {
        if (object.hasOwnProperty(Property))
            return false;
    }
    return true;
}


// Parse command line arguments
const argv =
    require("arrowargs")(process.argv.slice(2))
        .option("c",
                {alias: "config-path", describe: "Path to config file if not in working directory", type: "string"})
        .option("dev",
                {alias: "dev-server", describe: "Starts the arrowpack dev server", type: "boolean", default: false})
        .option("v", {alias: "version", describe: "Display version information", type: "boolean", default: false})
        .option("init",
                {alias: "initialize", describe: "Initialize arrowpack in a project", type: "boolean", default: false})
        .help()
        .argv;


/**
 * Handle config file
 */

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

// Get config file data
if (CONFIG_FILE_NAME !== "") {
    CONFIG_FILE_NAME = path.join(process.cwd(), CONFIG_FILE_NAME);
    rawconfigData = require(CONFIG_FILE_NAME);
}
else {
    rawconfigData = {};
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
const Settings = new config(rawconfigData);

/**
 *  Plugin handling/logic
 * This will soon be removed/revamped into an improved plugin system
 */

// Caches used plugins so they don't need to be reloaded every time they are used during the dev server
const PluginsCache = {};

// Function for transforming files that is called from C code
function JSTransformFiles(EncodedOriginalContents, PluginPath)
{
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
async function JSValidateFiles(FileContents, PluginPath, FilePath)
{
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


// Hold the loaded Wasm Module
let CFunctions;

// Pointer to the dependency graph
let DependencyGraphPtr;

/* Handle bundler execution based on command line arguments */

// -v: print version and terminate
if (argv.v) {
    const version = require("../package.json").version;
    console.log(version);
}
// init: initialise directory for arrowpack
else if (argv.init) {
    const initialise = require("../src/node/initialise.js");
    initialise();
}
else {
    // Runs the main bundler and handles dev server
    (async () => {
        // Get native module
        CFunctions = await CFunctionFactory();

        // -dev: start dev server, setup file watcher, bundle
        if (argv.dev === true) {
            console.log("Entering dev mode");

            // Start the static HTTP dev server
            const DevServer = require("../src/node/server/DevServer.js");
            DevServer.StartServer(Settings);

            // Watch file system of CWD
            const watcher = chokidar.watch(Settings.getValue("entry"));

            /**
             * When files are changed, rebuild the changed file and its dependents, then send the updated page to all
             * connected clients
             */
            watcher.on("change", (FilePath) => {
                console.log("File " + FilePath + " has changed, rebuilding...");

                // Store start time to track bundling time
                var StartTime = performance.now();

                /*
                 * Calls the native C RebuildFiles function from Main.c
                 * Called via ccall and provided the graph pointer, the path of the changed file, and the number of
                 * files (currently always 1) This returns a serialised array of files that were modified so that
                 * clients connected to those files can be refreshed
                 */
                var RebuiltFiles = CFunctions.ccall("RebuildFiles", "string", ["number", "string", "number"],
                                                    [DependencyGraphPtr, FilePath, 1]);

                // Deserialise string of changed files
                RebuiltFiles = ArrowDeserialize(RebuiltFiles);

                // Send the updated pages to clients
                DevServer.SendUpdatedPage(RebuiltFiles, Settings);

                console.log(chalk.magentaBright("\n\nBundling files completed in " +
                                                (performance.now() - StartTime) / 1000 + " seconds\n\n"));
            });
        }

        // Call main bundle logic, this happens in normal execution and in dev mode
        Bundle();
    })();
}

// Main function for bundling files
function Bundle()
{
    // Create temporary directory for temp files
    DirFunctions.mkdirIfNotExists("ARROWPACK_TEMP_PREPROCESS_DIR");

    // Check that Wasm has been initialized correctly
    CFunctions._CheckWasm();

    // Initialize file types structs
    CFunctions._InitFileTypes();

    // Convert settings to a JSON string
    const StringifiedJSON = JSON.stringify(Settings.settings)

    // Initialize settings on the Wasm side
    var Success = CFunctions.ccall("InitSettings", "number", ["string"], [StringifiedJSON]);
    if (Success !== 1) {
        throw "Error setting up Wasm settings";
    }

    // Find all dependencies and create the dependency graph
    DependencyGraphPtr = CFunctions.ccall(
        "CreateGraph",
        "number",
    );

    // Run validators on the Wasm side if any exist
    if (!ObjectIsEmpty(Settings.settings.validators)) {
        Success = false;
        const ValidateJSFunctionPointer = CFunctions.addFunction(JSValidateFiles, "iiii");
        Success = CFunctions.ccall("ExecutePlugin", "number", ["number", "number", "number"],
                                   [DependencyGraphPtr, ValidateJSFunctionPointer, 2])
    }

    // Run transformers on the Wasm side if any exist
    let TransformJSFunctionPointer = Success;
    if (!ObjectIsEmpty(Settings.settings.transformers)) {
        Success = false;
        TransformJSFunctionPointer = CFunctions.addFunction(JSTransformFiles, "iiii");
        Success = CFunctions.ccall("TransformFiles", "number", ["number", "number"],
                                   [DependencyGraphPtr, TransformJSFunctionPointer])
        if (Success !== 1)
        {
            throw "Error transforming files!";
        }
    }

    // Sort the dependency graph topologically
    CFunctions._topological_sort(DependencyGraphPtr);

    // Bundle all the files in the graph
    Success = CFunctions.ccall("BundleFiles", "number", ["number"], [DependencyGraphPtr]);

    // Run the post processors on the Wasm side if any exist
    if (!ObjectIsEmpty(Settings.settings.postProcessors)) {
        Success = false;
        Success = CFunctions.ccall("ExecutePlugin", "number", ["number", "number", "number"],
                                   [DependencyGraphPtr, TransformJSFunctionPointer, 3]);
    }

    if (Success === 1 || Success === 0) {
        console.log(chalk.magentaBright("\n\nBundling files completed in " + (performance.now() - StartTime) / 1000 +
                                        " seconds\n\n"));

        if (argv.dev) {
            console.log("Dev server running...");
        }
        else {
            DeletePreprocessDir();
        }
    }
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
function DeletePreprocessDir()
{
    fs.rm("ARROWPACK_TEMP_PREPROCESS_DIR", {recursive: true}, (err) => {
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