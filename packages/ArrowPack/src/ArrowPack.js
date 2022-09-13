"use strict";
// js wrapper for arrowpack for NPM
const fs = require("fs");
const path = require("path");
const chalk = require("chalk");
const boxen = require("boxen");
const settingsSingleton = require("./SettingsSingleton/settingsSingleton");
const RecursiveWalkDir = require("./js/RecursiveWalkDir");
const wasm_exec = require("./js/wasm_exec.js");
const go = new Go();

const argv = require("yargs/yargs")(process.argv.slice(2))
	.option("c", {
		alias: "config-path",
		describe: "Path to config file if not in working directory",
		type: "string"
	})
	.help().argv;

var CONFIG_FILE_NAME = "ArrowPack-config.json"



if (argv.c) { CONFIG_FILE_NAME = path.join(argv.c, CONFIG_FILE_NAME) } else { console.log("no custom file thingy"); }



var rawconfigData = null;
if (fs.existsSync(CONFIG_FILE_NAME)) { rawconfigData = fs.readFileSync(CONFIG_FILE_NAME, "utf8"); }

const settings = new settingsSingleton(rawconfigData);
var WalkedFiles, WalkedDirs = RecursiveWalkDir(settings.getValue("entry")); // eventually add pluginAPI event here

var WrappedWalkedFiles = "";
if (WalkedFiles && WalkedFiles.length > 0) {
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
			[WrappedWalkedFiles, WalkedFiles.length, settings.getValue("entry"), settings.getValue("exit")]
		)
	});

	var GoWASMFileHandler;
	const goWASM = fs.readFileSync("../Build/FileHandler.wasm");

	WebAssembly.instantiate(goWASM, go.importObject).then(function (obj) {
		GoWASMFileHandler = obj.instance;
		go.run(GoWASMFileHandler);
		GoWASMFileHandler.exports.HandleFiles(StructsPointer, settings.getValue("entry"));
	});
}