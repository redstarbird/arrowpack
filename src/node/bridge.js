"use strict";

const fs = require("fs");
const path = require("path");

// Stores the current native module instance, null until the first call of `loadNative`
let nativeInstance = null;

// Creates a mapping between the native function calls and the Wasm function calls
function createWasmAdapter(wasmModule) {
    return {
        backendType: "wasm",
        CheckNative: () => wasmModule.ccall("CheckNative", "number", [], []),
        InitFileTypes: () => wasmModule.ccall("InitFileTypes", null, [], []),
        InitSettings: (StringifiedJSON) => wasmModule.ccall("InitSettings", "number", ["string"], [StringifiedJSON]),
        CreateGraph: () => wasmModule.ccall("CreateGraph", "number", [], []),
        topological_sort: (GraphPtr) => wasmModule.ccall("topological_sort", "number", [], [GraphPtr]),
        BundleFiles: (GraphPtr) => wasmModule.ccall("BundleFiles", "number", ["number"], [GraphPtr]),
        RebuildFiles: (GraphPtr, FilePath) => wasmModule.ccall("RebuildFiles", "string", ["number", "string", "number"], [GraphPtr, FilePath, 1])
    };
}

// Returns an abstracted interfact to the internal native module
// This will either use a N-API (default) or Wasm (fallback) module under the hood
async function loadNative() {
    if (nativeInstance) {
        return nativeInstance;
    }

    /*
    arrowpack.node should be in `build/native/arrowpack.node` normally
    but could possibly be in "Release" or "Debug" dir depending on local build process
    */
    const pathOptions = [
        path.resolve(__dirname, "../../build/native/arrowpack.node"),
        path.resolve(__dirname, "../../build/native/Release/arrowpack.node"),
        path.resolve(__dirname, "../../build/native/Debug/arrowpack.node")
    ];

    // Attempt to load N-API module from the possibly paths
    for (const pathOption of pathOptions) {
        if (fs.existsSync(pathOption)) {
            try {
                const nativeModule = require(pathOption);
                nativeModule.backendType = "napi";
                return nativeModule;
            } catch (err) {
                console.warn("Native N-API module failed to load " + err.message + ". Falling back to WebAssembly...");
            }
        }
    }

    // If N-API fails to load, attempt to load the fallback Wasm module instead
    const WasmJSPath = path.resolve(__dirname, "../../build/wasm/CFunctions.js");
    if (fs.existsSync(WasmJSPath)) {
        const WasmModuleFactory = require(WasmJSPath);
        const WasmModule = await WasmModuleFactory();
        return createWasmAdapter(WasmModule);
    }

    throw new Error("Unable to load either the native N-API addon (arrowpack.node) or WASM Module (CFunctions.js)!")
}

module.exports = loadNative;