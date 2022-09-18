"use strict";
// js wrapper for arrowpack for NPM
const fs = require("fs");
const path = require("path");
const chalk = require("chalk");
const boxen = require("boxen");
const settingsSingleton = require("./SettingsSingleton/settingsSingleton");
const DirFunctions = require("./js/DirFunctions");
const wasm_exec = require("./js/wasm_exec.js");
const CFunctionFactory = require("../Build/CFunctions.js");
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
let temp = DirFunctions.RecursiveWalkDir(settings.getValue("entry")); // eventually add pluginAPI event here

var WalkedFiles = temp.Files;
var WalkedDirs = temp.Directories;

if (WalkedDirs) {
	WalkedDirs.forEach(Dir => {

	});
}

var AbsoluteFilesCharLength = 0;
var WrappedWalkedFiles = "";
if (WalkedFiles && WalkedFiles.length > 0) {
	WalkedFiles.forEach(FilePath => { WrappedWalkedFiles += "::" + FilePath; console.log(chalk.bold.blue(FilePath)); AbsoluteFilesCharLength += FilePath.length; });

	var StructsPointer;

	CFunctionFactory().then((CFunctions) => {
		CFunctions._CheckWasm();
		// StructsPointer = CFunctions._CreateTree(allocateUTF8(WrappedWalkedFiles), WalkedFiles.length, AbsoluteFilesCharLength); // Need to get this working eventually for faster speed but couldn't work out allocateUTF8
		StructsPointer = CFunctions.ccall(
			"CreateTree",
			"number",
			["string", "number"],
			[WrappedWalkedFiles, WalkedFiles.length]
		);
	});

	/*
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
		});*/
}