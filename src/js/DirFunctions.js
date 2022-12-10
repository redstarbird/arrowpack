"use strict";
const fs = require("fs");
const path = require("path");

function RecursiveWalkDir(FolderPath, Files, Directories) {
	var InitialFiles = fs.readdirSync(FolderPath);
	Files = Files || [];
	Directories = Directories || [];

	InitialFiles.forEach(function (file) {
		let fullPath = path.join(FolderPath, file);
		if (fs.statSync(fullPath).isDirectory()) {
			// Push the full directory path to the Directories array
			Directories.push(fullPath);
			let temp = RecursiveWalkDir(fullPath, Files, Directories);
			Files = temp.Files;              //  Destructuring was not working for some reason
			Directories = temp.Directories;  //  :(

		} else {
			Files.push(fullPath);

		}
	});

	return { Files, Directories };
};


function mkdirIfNotExists(dir) {
	if (!fs.existsSync(dir)) {
		fs.mkdirSync(dir, { recursive: true });
	}
}

module.exports = {
	RecursiveWalkDir: RecursiveWalkDir,
	mkdirIfNotExists: mkdirIfNotExists
}