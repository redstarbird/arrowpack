"use strict";
// js wrapper for arrowpack for NPM
const fs = require("fs");
const yargs = require("yargs");
const chalk = require("chalk");
const boxen = require("boxen");
const settingsSingleton = require("./SettingsSingleton/settingsSingleton");
const RecursiveWalkDir = require("./js/recursiveWalkDir ");
import { WASI } from "wasi";
const wasm_exec = require("./js/wasm_exec.js");
const go = new wasm_exec.Go();
import { version } from "../package.json";

if (process.argv[2]) {
	if (
		process.argv[2] == "--version" ||
		process.argv[2] == "-version" ||
		process.argv[2] == "-v" ||
		process.argv[2] == "--v" ||
		process.argv[2] == "version"
	) {
		console.log(version);
	}
} else if (process.argv[2] == "build") {
	const settings = new settingsSingleton(fs.readFileSync("ArrowPack-config.json", "utf8"));

	var WalkedFiles, WalkedDirs = RecursiveWalkDir(settings.getValue("entry")); // eventually add pluginAPI event here

	var WrappedWalkedFiles = "";
	WalkedFiles.forEach(FilePath => { WrappedWalkedFiles += "::" + FilePath });

	const DependencyTreeWasmBuffer = fs.readFileSync("../Build/DependencyTree.wasm");
	var DependencyTreeMemory = new WebAssembly.Memory({
		initial: 10,
		maximum: 4096,
		shared: true,
	});

	var StructsPointer;
	WebAssembly.instantiateStreaming(DependencyTreeWasmBuffer, DependencyTreeMemory).then((instance) => {
		StructsPointer = instance.ccall(
			"CreateTree",
			"number",
			["string", "number", "string"],
			[WrappedWalkedFiles, WalkedFiles.length, settings.getValue("entry")]
		)
	});

	var GoWASMFileHandler;
	const goWASM = fs.readFileSync("../Build/FileHandler.wasm");

	WebAssembly.instantiate(goWASM, go.importObject).then(function (obj) {
		GoWASMFileHandler = obj.instance;
		go.run(GoWASMFileHandler);
		GoWASMFileHandler.exports.HandleFiles(StructsPointer, settings.ToStringFormat);
	});
} else {
	console.log('Please specify a command-line arg, such as "ArrowPack build"');
}
