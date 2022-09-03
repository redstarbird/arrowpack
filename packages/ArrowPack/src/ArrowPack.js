// js wrapper for arrowpack for NPM
const fs = require("fs");
const yargs = require("yargs");
const chalk = require("chalk");
const boxen = require("boxen");
const settingsSingleton = require("./js/settingsSingleton");
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
	const settings = new settingsSingleton(
		fs.readFileSync("ArrowPack-config.json", "utf8")
	);

	var wasm;
	const goWASM = fs.readFileSync("../Build/FileHandler.wasm");

	WebAssembly.instantiate(goWASM, go.importObject).then(function (obj) {
		wasm = obj.instance;
		go.run(wasm);
	});
} else {
	console.log('Please specify a command-line arg, such as "ArrowPack build"');
}
