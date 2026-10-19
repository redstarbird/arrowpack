/*
 * @file The main arrowpack API for Node.js.
 * This file calls and orchestrates the internal native C bundler.
 * The public build and createServer functions are exposed from this file.
 */

const loadNativeModule = require("./bridge.js")
const DirFunctions = require("./util/FSUtil.js");
const Config = require("./config.js")
const DevServer = require("./server/DevServer.js");
const { isPlainObject } = require("./utils.js");

/**
 * Reusable helper function that actually builds the project. This is called in both `build` and `createServer`.
 * 
 * Creates the temporary preprocess directory, initialising config class from JSON, connects to the native module, 
 * sets up native structures, resolves depedencies, 
 * @param {Config} config 
 * @returns Object containing config instance, native module, and dependency graph pointer
 */
async function buildProject(config) {
    // Preprocess directory
    DirFunctions.mkdirIfNotExists("ARROWPACK_TEMP_PREPROCESS_DIR");

    config = new Config(config);

    let nativeModule = await loadNativeModule(config);

    /*
    * Check that native functions can be called correctly.
    * checkNative returns 42 to ensure that values are 
    * properly being translated between JS and C.
    */
    if (nativeModule.CheckNative() != 42) {
        throw new Error("Unable to communicate correctly with native module!");
    }

    // Initialise internal C data-structure for handling different file types
    nativeModule.InitFileTypes();

    /* 
    * Sends config to native side as stringified JSON.
    * InitSettings is declared in `src/native/config.h` and takes a string version of the internal config JSON,
    * this is then parsed by the external cJSON library and turned into a cJSON object using `cJSON_Parse`.
    */
    const ConfigStr = JSON.stringify(config.settings)
    if (!nativeModule.InitSettings(ConfigStr)) {
        throw new Error("Unable to initialise internal config!");
    }

    /*
    * The dependency graph creation step finds all of the files in the entry directory,
    * parses each file to find its imports, adds them to the graph structure, and then connects
    * dependencies and dependents within the graph.
    * createGraph is declared in `src/native/graph/DependencyGraph.h` and defined in `src/native/graph/DependencyGraph.c`
    */
    let dependencyGraphPtr = nativeModule.CreateGraph();

    /*
    * Topologically sort the graph to find the order in which files need to be built/bundled.
    * This orders dependencies before dependents.
    * topological_sort is declared in `src/native/graph/DependencyGraph.h` and defined in `src/native/graph/DependencyGraph.c`
    */
    nativeModule.topological_sort(dependencyGraphPtr);

    /*
    * Main build/codegen step.
    * Concatenates files together, mutates final files, and saves them to disk.
    */
    let success = nativeModule.BundleFiles(dependencyGraphPtr);

    if (!success) {
        throw new Error("An error occured when attempting to bundle files!");
    }

    return {
        config,
        nativeModule,
        dependencyGraphPtr
    }
}

/**
 * Runs a single one-off build of the project.
 * @param {Object} config Arrowpack configuration object.
 */
async function build(config) {
    await buildProject(config);
}

/**
 * Performs an initial build and creates a DevServer instance.
 * 
 * @param {Object} config Arrowpack JSON configuration object
 * @returns {Promise<DevServer>}
 */
async function createServer(config = {}) {

    const devConfig = { ...config, productionMode: false };

    let BundleContext = await buildProject(devConfig);

    const server = new DevServer(BundleContext);

    await server.start();

    return server;

}

module.exports = { build, createServer, DevServer };