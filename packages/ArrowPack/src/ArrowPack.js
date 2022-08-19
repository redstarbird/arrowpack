// js wrapper for arrowpack for NPM
const fs = require("fs");
const wasm_exec = require("js/wasm_exec.js");
const go = new wasm_exec.Go();

const wasmBuffer = fs.readFileSync("main.wasm");
// do stuff eventually
WebAssembly.instantiate(wasmBuffer, go.importObject).then((wasmModule) => {});
