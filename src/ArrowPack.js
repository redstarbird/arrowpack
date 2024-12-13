#!/usr/bin/env node
"use strict";
const fs = require("fs");
const path = require("path");
const chalk = require("chalk");
const settingsSingleton = require("./SettingsSingleton/settingsSingleton");
const DirFunctions = require("./js/DirFunctions");
const CFunctionFactory = require("../Build/CFunctions.js");
const Sleep = require("../src/js/Sleep");
const { mkdirIfNotExists } = require("./js/DirFunctions.js");
const chokidar = require('chokidar');
const ArrowSerializer = require("./js/StringConversion.cjs");
const { ArrowDeserialize } = require("./js/StringConversion.cjs");
const { createRequire } = require("module");
const { exit } = require("process");
const { performance } = require('perf_hooks');

var StartTime = performance.now(); // Track the start time to track bundle time

function requireModule(modulePath, exportName) {
	try {
		const imported = require(modulePath);
		return exportName ? imported[exportName] : imported;
	} catch (err) {
		return err.code;
	}
}

// Returns whether an object is empty or not
function ObjectIsEmpty(object) {
	for (var Property in object) {
		if (object.hasOwnProperty(Property))
			return false;
	}
	return true;
}

const argv = require("arrowargs")(process.argv.slice(2)) // Handle command line arguments
	.option("c", {
		alias: "config-path",
		describe: "Path to config file if not in working directory",
		type: "string"
	}).option("dev", {
		alias: "dev-server", describe: "Starts the arrowpack dev server", type: "boolean", default: false
	}).option("v", { alias: "version", describe: "Display version information", type: "boolean", default: false })
	.help().argv;


// Get configuration file path
var CONFIG_FILE_NAME = "arrowpack.config.js"; //
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

// Get configuration file data
if (CONFIG_FILE_NAME !== "") {
	CONFIG_FILE_NAME = path.join(process.cwd(), CONFIG_FILE_NAME);
	rawconfigData = require(CONFIG_FILE_NAME);
} else {
	rawconfigData = {};
}

// Configure internal settings
if (argv.c) { if (!argv.c.endsWith("/")) { argv.c += "/"; } rawconfigData["INTERNAL_CONFIG_DIR"] = argv.c, rawconfigData["INTERNAL_FULL_CONFIG_PATH"] = path.join(process.cwd(), argv.c) }
const Settings = new settingsSingleton(rawconfigData); // Initialize settings singleton


const PluginsCache = {}; // Caches used plugins so they don't need to be reloaded every time they are used

// Function for transforming files that is called from C code
function JSTransformFiles(EncodedOriginalContents, PluginPath) {
	const OriginalFileContents = CFunctions.UTF8ToString(EncodedOriginalContents);
	PluginPath = CFunctions.UTF8ToString(PluginPath);

	if (!PluginsCache[PluginPath]) { // Make sure that the plugin is in the cache
		try {
			PluginsCache[PluginPath] = require(PluginPath);
		} catch (error) {
			throw "Error loading plugin: " + PluginPath + "\n\n" + error;
		}
	}
	const Transformer = PluginsCache[PluginPath];
	const FileContents = Transformer(OriginalFileContents); // Run the transformation on the file contents
	if (FileContents === OriginalFileContents) { // Don't waste time encoding strings if the file contents haven't changed
		return null;
	}
	// Encode string into UTF8 encoding
	var lengthBytes = CFunctions.lengthBytesUTF8(FileContents) + 1;
	var stringOnWasmHeap = CFunctions._malloc(lengthBytes);
	CFunctions.stringToUTF8(FileContents, stringOnWasmHeap, lengthBytes);
	return stringOnWasmHeap; // Return the encoded string
}

// Function for validating files
async function JSValidateFiles(FileContents, PluginPath, FilePath) {
	// Decode UTF8 string into JS string
	PluginPath = CFunctions.UTF8ToString(PluginPath);
	FilePath = CFunctions.UTF8ToString(FilePath);
	FileContents = CFunctions.UTF8ToString(FileContents);

	let imported = false;
	if (!PluginsCache[PluginPath]) { // Ensure the plugin is in the cache
		try {
			await (async () => {
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
		const Results = await Validator.Validate(FileContents, FilePath); // Run validator
		if (Results.warnings) { // Check for warnings
			if (Results.warnings.length > 0) {
				for (let i = 0; i < Results.warnings.length; i++) {
					console.log("Warning: " + Results.warnings[i]);
				}
			}
		}
		if (Results.errors) { // Check for errors
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

if (argv.v) {
	const version = require("../package.json").version;
	console.log(version);
} else {
(async () => { // Dev server code
	CFunctions = await CFunctionFactory();

	if (argv.dev === true) { // Check if dev server is enabled
		console.log("Entering dev mode");
		const DevServer = require("./js/DevServer.js");
		DevServer.StartServer(Settings);

		const watcher = chokidar.watch(Settings.getValue("entry")); // Watch file system of CWD
		watcher.on("change", (FilePath) => { // Wait for files to be changed
			console.log("File " + FilePath + " has changed, rebuilding...");
			var StartTime = performance.now();
			var RebuiltFiles = CFunctions.ccall( // Call C function to rebuild files
				"RebuildFiles",
				"string",
				["number", "string", "number"],
				[StructsPointer, FilePath, 1]);
			RebuiltFiles = ArrowDeserialize(RebuiltFiles); // Deserialise serialised string of changed files
			DevServer.SendUpdatedPage(RebuiltFiles, Settings); // Send the updated pages to clients
			console.log(chalk.magentaBright("\n\nBundling files completed in " + (performance.now() - StartTime) / 1000 + " seconds\n\n"));
		});
	} Bundle();
})();}


// Main function for bundling files
function Bundle() {
	var temp;
	console.log(chalk.yellow("Here!"));

	// Find all files in CWD and all subdirectories
	if (Settings.getValue("largeProject") === false) {
		temp = DirFunctions.RecursiveWalkDir(Settings.getValue("entry")); // eventually add pluginAPI event here
	} else { // Use Wasm module to recusively find all files (doesn't currently work)
		let RecursiveWalkDirWASM = fs.readFileSync("../Build/RecursiveWalkDir.wasm"); const { WebAssembly } = require("wasi");
		let compiledWalkDirWASM = WebAssembly.compile(wasm);
		let InstanceWalkDirWASM = WebAssembly.instantiate(compiledWalkDirWASM);
		const { InstanceWalkDirWASMExports } = instance;
		temp = InstanceWalkDirWASMExports.walk_dir(Settings.getValue("entry"));
	}
	var WalkedFiles = temp.Files;
	var WalkedDirs = temp.Directories;

	// Create exit directories for all entry directories
	if (WalkedDirs) {
		WalkedDirs.forEach(Dir => {
			var tempDir = Settings.getValue("exit") + Dir.substring(Settings.getValue("entry").length);

			DirFunctions.mkdirIfNotExists(tempDir);
		});
	}

	DirFunctions.mkdirIfNotExists("ARROWPACK_TEMP_PREPROCESS_DIR"); // Create temporary directory for temp files

	if (WalkedFiles && WalkedFiles.length > 0) {
		var WrappedWalkedFiles = ArrowSerializer.ArrowSerialize(WalkedFiles);
		CFunctions._CheckWasm(); // Check that Wasm has been initialized correctly
		CFunctions._InitFileTypes(); // Initialize file types structs

		const StringifiedJSON = JSON.stringify(Settings.settings) // Convert settings to a JSON string

		var Success = CFunctions.ccall("InitSettings", "number", ["string"], [StringifiedJSON]); // Initialize settings on the Wasm side
		if (Success !== 1) { throw "Error setting up Wasm settings"; }

		StructsPointer = CFunctions.ccall( // Find all dependencies and create the dependency graph
			"CreateGraph",
			"number",
			["string"],
			[WrappedWalkedFiles]
		);

		if (!ObjectIsEmpty(Settings.settings.validators)) { // Run validators on the Wasm side if any exist
			Success = false;
			const ValidateJSFunctionPointer = CFunctions.addFunction(JSValidateFiles, "iiii");
			Success = CFunctions.ccall(
				"ExecutePlugin", "number", ["number", "number", "number"], [StructsPointer, ValidateJSFunctionPointer, 2]
			)

		}
		let TransformJSFunctionPointer = Success;

		if (!ObjectIsEmpty(Settings.settings.transformers)) { // Run transformers on the Wasm side if any exist
			Success = false;
			TransformJSFunctionPointer = CFunctions.addFunction(JSTransformFiles, "iiii");
			Success = CFunctions.ccall(
				"TransformFiles", "number", ["number", "number"], [StructsPointer, TransformJSFunctionPointer]
			)
			if (Success !== 1) { throw "Error transforming files!"; }
		}

		Success = false;
		CFunctions._topological_sort(StructsPointer); // Sort the dependency graph topologically

		Success = 0;
		Success = CFunctions.ccall( // Bundle all the files in the graph
			"BundleFiles",
			"number",
			["number"],
			[StructsPointer]
		);

		if (!ObjectIsEmpty(Settings.settings.postProcessors)) { // Run the post processors on the Wasm side if any exist
			Success = false;
			Success = CFunctions.ccall(
				"ExecutePlugin", "number", ["number", "number", "number"], [StructsPointer, TransformJSFunctionPointer, 3]
			);
		}

		if (Success === 1 || Success === 0) {
			console.log(chalk.magentaBright("\n\nBundling files completed in " + (performance.now() - StartTime) / 1000 + " seconds\n\n")); // Print the bundle time

			if (argv.dev) {
				console.log("Dev server running...");
			}
			else {
				DeletePreprocessDir();
			}

		}
	} else {
		console.log(chalk.yellowBright("Could not find any files to be bundled")); // No files were found in the entry directory
	}
}

// Clean up the preprocess directory on exit
process.on("SIGTERM", () => { // Delete the preprocess directory on sigterm
	print("Exiting due to SIGTERM, deleting temp directory...");
	DeletePreprocessDir();
})
process.on("exit", () => { DeletePreprocessDir(); }); // Delete the preprocess directory on exit
process.on("SIGINT", () => { var DelResult = DeletePreprocessDir(); process.exit(DelResult); }); // Delete the preprocess directory on force stop from user

function DeletePreprocessDir() { // Function to delete the preprocess directory
	fs.rm("ARROWPACK_TEMP_PREPROCESS_DIR", { recursive: true }, (err) => {

		if (err) { console.error(err); return 1; } else {
			console.log("Sucessfully removed temporary preprocess directory");
		} return 0;
	});
}