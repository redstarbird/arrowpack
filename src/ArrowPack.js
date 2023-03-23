#!/usr/bin/env node
"use strict";
// js wrapper for arrowpack for NPM
const fs = require("fs");
const path = require("path");
const chalk = require("chalk");
const settingsSingleton = require("./SettingsSingleton/settingsSingleton");
const DirFunctions = require("./js/DirFunctions");
// const wasm_exec = require("../Build/wasm_exec.js");
const CFunctionFactory = require("../Build/CFunctions.js");
// const go = new Go();
const Sleep = require("../src/js/Sleep");
const { mkdirIfNotExists } = require("./js/DirFunctions.js");
const chokidar = require('chokidar');
const ArrowSerializer = require("./js/StringConversion.cjs");
const { ArrowDeserialize } = require("./js/StringConversion.cjs");


var StartTime = performance.now();

const argv = require("arrowargs")(process.argv.slice(2))
	.option("c", {
		alias: "config-path",
		describe: "Path to config file if not in working directory",
		type: "string"
	}).option("dev", { alias: "dev-server", describe: "Starts the arrowpack dev server", type: "boolean", default: false })
	.help().argv;

var CONFIG_FILE_NAME = "ArrowPack-config.json";
if (argv.c) { CONFIG_FILE_NAME = path.join(argv.c, CONFIG_FILE_NAME); }
var rawconfigData = null;
if (fs.existsSync(CONFIG_FILE_NAME)) { rawconfigData = fs.readFileSync(CONFIG_FILE_NAME, "utf8"); }
const Settings = new settingsSingleton(rawconfigData);

process.on("SIGTERM", () => {
	print("Exiting due to SIGTERM, deleting temp directory...");
	DeletePreprocessDir();
})
process.on("exit", () => { DeletePreprocessDir(); });
process.on("SIGINT", () => { var DelResult = DeletePreprocessDir(); process.exit(DelResult); });
function DeletePreprocessDir() {
	fs.rm("ARROWPACK_TEMP_PREPROCESS_DIR", { recursive: true }, (err) => {

		if (err) { console.error(err); } else {
			console.log("Sucessfully removed temporary preprocess directory");
		} return 0;
	});
}

let CFunctions;
let StructsPointer;

(async () => {
	CFunctions = await CFunctionFactory();

	if (argv.dev === true) {
		console.log("Entering dev mode");
		const DevServer = require("./js/DevServer.js");
		DevServer.StartServer(Settings);

		const watcher = chokidar.watch(Settings.getValue("entry"));
		watcher.on("change", (FilePath) => {
			console.log("File " + FilePath + " has changed, rebuilding...");
			var StartTime = performance.now();
			var RebuiltFiles = CFunctions.ccall(
				"RebuildFiles",
				"string",
				["number", "string", "number"],
				[StructsPointer, FilePath, 1]);
			RebuiltFiles = ArrowDeserialize(RebuiltFiles);
			DevServer.SendUpdatedPage(RebuiltFiles, Settings);
			console.log(chalk.magentaBright("\n\nBundling files completed in " + (performance.now() - StartTime) / 1000 + " seconds\n\n"));
		});
	} Bundle();
})();


function Bundle() {
	var temp;

	if (Settings.getValue("largeProject") === false) {
		temp = DirFunctions.RecursiveWalkDir(Settings.getValue("entry")); // eventually add pluginAPI event here
	} else {
		let RecursiveWalkDirWASM = fs.readFileSync("../Build/RecursiveWalkDir.wasm"); const { WebAssembly } = require("wasi");
		let compiledWalkDirWASM = WebAssembly.compile(wasm);
		let InstanceWalkDirWASM = WebAssembly.instantiate(compiledWalkDirWASM);
		const { InstanceWalkDirWASMExports } = instance;
		temp = InstanceWalkDirWASMExports.walk_dir(Settings.getValue("entry"));
	}

	var WalkedFiles = temp.Files;
	var WalkedDirs = temp.Directories;
	console.log(WalkedDirs);
	if (WalkedDirs) {
		WalkedDirs.forEach(Dir => {
			console.log(chalk.red(Dir));
			var tempDir = Settings.getValue("exit") + Dir.substring(Settings.getValue("entry").length);

			console.log(chalk.yellowBright(tempDir));
			DirFunctions.mkdirIfNotExists(tempDir);
			//DirFunctions.mkdirIfNotExists(Dir);
		});
	}
	DirFunctions.mkdirIfNotExists("ARROWPACK_TEMP_PREPROCESS_DIR");
	var AbsoluteFilesCharLength = 0;
	var WrappedWalkedFiles = "";
	if (WalkedFiles && WalkedFiles.length > 0) { // Paths are wrapped into one string because passing array of strings from JS to C is complicated
		WalkedFiles.forEach(FilePath => { WrappedWalkedFiles += FilePath + "::"; console.log(chalk.bold.blue(FilePath)); AbsoluteFilesCharLength += FilePath.length; });



		CFunctions._CheckWasm();
		CFunctions._InitFileTypes();
		for (let k in Settings.settings) {
			if (CFunctions.ccall(
				"SendSettingsString",
				"number",
				["string"],
				[k]
			) != 1) {
				throw "Error sending Wasm settings string: " + k;
			}
			console.log(chalk.bold.blue(k));
			// Gives time to apply settings
			if (CFunctions.ccall(
				"SendSettingsString",
				"number",
				["string"],
				[Settings.settings[k].toString()]
			) != 1) {
				throw "Error sending Wasm settings string: " + Settings.settings[k];
			}
			console.log(chalk.bold.blue(Settings.settings[k]));
			// Also gives time to apply settings
		}
		var Success;
		// StructsPointer = CFunctions._CreateTree(allocateUTF8(WrappedWalkedFiles), WalkedFiles.length, AbsoluteFilesCharLength); // Need to get this working eventually for faster speed but couldn't work out allocateUTF8
		StructsPointer = CFunctions.ccall(
			"CreateGraph",
			"number",
			["string", "number"],
			[WrappedWalkedFiles, WalkedFiles.length]
		);

		Success = CFunctions.ccall(
			"BundleFiles",
			"number",
			["number"],
			[StructsPointer]
		);

		if (Success === 1 || Success === 0) {

			//CFunctions.ccall("PrintTimeTaken", "void", ["number", "number"], [StartTime, performance.now()]); // Not working for some reason
			console.log(chalk.magentaBright("\n\nBundling files completed in " + (performance.now() - StartTime) / 1000 + " seconds\n\n"));
			if (argv.dev) { console.log("Dev server running..."); } else {
				DeletePreprocessDir();
			}

		}


		// console.log("\n\nBuild completed in" + (EndTime - StartTime) / 1000 + " seconds!\n\n\n"); // Need to get Wasm code to run this because Wasm code seems to be non-blocking
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
}



