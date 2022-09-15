"use strict";
const fs = require("fs");
const path = require("path");

function RecursiveWalkDir(FolderPath, Files, Directories) {
	var InitialFiles = fs.readdirSync(FolderPath);
	Files = Files || [];
	Directories = Directories || [];

	InitialFiles.forEach(function (file) {
		if (fs.statSync(path.join(FolderPath, file)).isDirectory()) {
			Directories.push(file);
			let temp = RecursiveWalkDir(path.join(FolderPath, file), Files, Directories);
			Files = temp.Files;              //  Destructuring was not working for some reason
			Directories = temp.Directories;  //  :(

		} else {
			Files.push(path.join(FolderPath, file));

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