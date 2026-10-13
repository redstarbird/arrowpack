/*
 * @file The main arrowpack API for Node.js.
 * This file calls and orchestrates the internal native C bundler.
 * The public build and createServer functions are exposed from this file.
 */

const loadNativeModule = require("./bridge.js")
const DirFunctions = require("../src/node/util/FSUtil.js");
const Config = require("./config.js")

async function build(config) {
    // Preprocess directory
    DirFunctions.mkdirIfNotExists("ARROWPACK_TEMP_PREPROCESS_DIR");

    let nativeModule = loadNativeModule();

    /*
    * Check that native functions can be called correctly.
    * checkNative returns 42 to ensure that values are 
    * properly being translated between JS and C.
    */
    if (nativeModule.checkNative() != 42) {
        throw new Error("Unable to communicate correctly with native module!");
    }

    // Initialise internal C data-structure for handling different file types
    nativeModule.InitFileTypes();


    /* 
    * Creates config and sends it to the native side.
    InitSettings is declared in `src/native/config.h` and takes a string version of the internal config JSON,
    this is then parsed by the external cJSON library and turned into a cJSON object using `cJSON_Parse`.
    */
    const config = Config(json);
    const ConfigStr = JSON.stringify(Settings.settings)
    if (!nativeModule.InitSettings(ConfigStr)) {
        // Todo Handle error
    }

    /*
    * The dependency graph creation step finds all of the files in the entry directory,
    * parses each file to find its imports, adds them to the graph structure, and then connects
    * dependencies and dependents within the graph.
    * createGraph is declared in `src/native/graph/DependencyGraph.h` and defined in `src/native/graph/DependencyGraph.c`
    */
    let dependencyGraphPtr = nativeModule.createGraph();

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
    let success = nativeModule.bundleFiles(dependencyGraphPtr);

    if (!success) {
        // Todo handle error
    }
}

module.exports = { build: build };