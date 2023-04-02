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
const { createRequire } = require("module");
const { exit } = require("process");
const { performance } = require('perf_hooks');

var StartTime = performance.now();

function requireModule(modulePath, exportName) {
	try {
		const imported = require(modulePath);
		return exportName ? imported[exportName] : imported;
	} catch (err) {
		return err.code;
	}
}

function ObjectIsEmpty(object) {
	for (var Property in object) {
		if (object.hasOwnProperty(Property))
			return false;
	}
	return true;
}

const argv = require("arrowargs")(process.argv.slice(2))
	.option("c", {
		alias: "config-path",
		describe: "Path to config file if not in working directory",
		type: "string"
	}).option("dev", { alias: "dev-server", describe: "Starts the arrowpack dev server", type: "boolean", default: false })
	.help().argv;

var CONFIG_FILE_NAME = "arrowpack.config.js";
if (argv.c) { if (fs.lstatSync(argv.c).isDirectory()) { CONFIG_FILE_NAME = path.join(argv.c, CONFIG_FILE_NAME); } else { CONFIG_FILE_NAME = argv.c; } }
var rawconfigData = null;
if (!fs.existsSync(CONFIG_FILE_NAME)) {
	var CJSName = CONFIG_FILE_NAME.substring(0, CONFIG_FILE_NAME.length - 3) + ".cjs";
	var EMJSName = CONFIG_FILE_NAME.substring(0, CONFIG_FILE_NAME.length - 3) + ".mjs";
	if (fs.existsSync(CJSName)) {
		CONFIG_FILE_NAME = CJSName;
	} else if (fs.existsSync(EMJSName)) {
		CONFIG_FILE_NAME = EMJSName;
	} else { CONFIG_FILE_NAME = ""; }
}
if (CONFIG_FILE_NAME !== "") {
	CONFIG_FILE_NAME = path.join(process.cwd(), CONFIG_FILE_NAME);
	/*if (CONFIG_FILE_NAME.endsWith(".mjs")) {
		rawconfigData = await(async () => { (await import(CONFIG_FILE_NAME)) })();
	}*/
	rawconfigData = require(CONFIG_FILE_NAME);
} else {
	rawconfigData = {};
}



console.log(rawconfigData);
if (argv.c) { if (!argv.c.endsWith("/")) { argv.c += "/"; } rawconfigData["INTERNAL_CONFIG_DIR"] = argv.c, rawconfigData["INTERNAL_FULL_CONFIG_PATH"] = path.join(process.cwd(), argv.c) }
const Settings = new settingsSingleton(rawconfigData);
process.exit(0);
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


const PluginsCache = {};

function JSTransformFiles(EncodedOriginalContents, PluginPath) {
	const OriginalFileContents = CFunctions.UTF8ToString(EncodedOriginalContents);
	PluginPath = CFunctions.UTF8ToString(PluginPath);

	if (!PluginsCache[PluginPath]) {
		try {
			PluginsCache[PluginPath] = require(PluginPath);
		} catch (error) {
			throw "Error loading plugin: " + PluginPath + "\n\n" + error;
		}
	}
	const Transformer = PluginsCache[PluginPath];
	const FileContents = Transformer(OriginalFileContents);
	if (FileContents === OriginalFileContents) {

		return null;
	}
	var lengthBytes = CFunctions.lengthBytesUTF8(FileContents) + 1;
	var stringOnWasmHeap = CFunctions._malloc(lengthBytes);
	CFunctions.stringToUTF8(FileContents, stringOnWasmHeap, lengthBytes);
	return stringOnWasmHeap;
}

async function JSValidateFiles(FileContents, PluginPath, FilePath) {
	PluginPath = CFunctions.UTF8ToString(PluginPath);
	FilePath = CFunctions.UTF8ToString(FilePath);
	console.log("JS validating\n");
	FileContents = CFunctions.UTF8ToString(FileContents);
	let imported = false;
	if (!PluginsCache[PluginPath]) {
		console.log("Not in cache!\n");
		try {
			console.log("trying!");
			await (async () => {
				console.log("async!\n");
				PluginsCache[PluginPath] = (await import(PluginPath)).default;
				imported = true;
				console.log(PluginsCache[PluginPath]);
			})();

		} catch (error) {
			throw "Error loading plugin: " + PluginPath + "\n\n" + error;
		}
	} else { imported = true; }

	const Validator = PluginsCache[PluginPath];
	await (async () => {
		const Results = await Validator.Validate(FileContents, FilePath); if (Results.warnings) {
			if (Results.warnings.length > 0) {
				for (let i = 0; i < Results.warnings.length; i++) {
					console.log("Warning: " + Results.warnings[i]);
				}
			}
		}
		if (Results.errors) {
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

let CFunctions;
let StructsPointer;

(async () => { // Dev server code
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

	if (WalkedDirs) {
		WalkedDirs.forEach(Dir => {
			var tempDir = Settings.getValue("exit") + Dir.substring(Settings.getValue("entry").length);

			DirFunctions.mkdirIfNotExists(tempDir);
			//DirFunctions.mkdirIfNotExists(Dir);
		});
	}
	DirFunctions.mkdirIfNotExists("ARROWPACK_TEMP_PREPROCESS_DIR");
	var AbsoluteFilesCharLength = 0;
	var WrappedWalkedFiles = "";
	if (WalkedFiles && WalkedFiles.length > 0) { // Paths are wrapped into one string because passing array of strings from JS to C is complicated
		WalkedFiles.forEach(FilePath => { WrappedWalkedFiles += FilePath + "::"; AbsoluteFilesCharLength += FilePath.length; });



		CFunctions._CheckWasm();
		CFunctions._InitFileTypes();
		/*for (let k in Settings.settings) {
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
		}*/const StringifiedJSON = JSON.stringify(Settings.settings)

		var Success = CFunctions.ccall("InitSettings", "number", ["string"], [StringifiedJSON]); if (Success !== 1) { throw "Error setting up Wasm settings"; } console.log("Setup settings");
		console.log("Success: " + Success);
		// StructsPointer = CFunctions._CreateTree(allocateUTF8(WrappedWalkedFiles), WalkedFiles.length, AbsoluteFilesCharLength); // Need to get this working eventually for faster speed but couldn't work out allocateUTF8
		StructsPointer = CFunctions.ccall(
			"CreateGraph",
			"number",
			["string", "number"],
			[WrappedWalkedFiles, WalkedFiles.length]
		); if (!ObjectIsEmpty(Settings.settings.validators)) {
			Success = false;
			const ValidateJSFunctionPointer = CFunctions.addFunction(JSValidateFiles, "iiii");
			Success = CFunctions.ccall(
				"ExecutePlugin", "number", ["number", "number", "number"], [StructsPointer, ValidateJSFunctionPointer, 2]
			)

		} let TransformJSFunctionPointer = Success;
		if (!ObjectIsEmpty(Settings.settings.transformers)) {
			Success = false;
			TransformJSFunctionPointer = CFunctions.addFunction(JSTransformFiles, "iiii");
			Success = CFunctions.ccall(
				"TransformFiles", "number", ["number", "number"], [StructsPointer, TransformJSFunctionPointer]
			)
			if (Success !== 1) { throw "Error transforming files!"; }
		}

		Success = false;
		CFunctions._topological_sort(StructsPointer);
		Success = 0;
		Success = CFunctions.ccall(
			"BundleFiles",
			"number",
			["number"],
			[StructsPointer]
		);

		if (!ObjectIsEmpty(Settings.settings.postProcessors)) {
			Success = false;
			Success = CFunctions.ccall(
				"ExecutePlugin", "number", ["number", "number", "number"], [StructsPointer, TransformJSFunctionPointer, 3]
			);
		}
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