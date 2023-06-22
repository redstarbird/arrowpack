"use strict";
const fs = require("fs");
const path = require("path");

// Recursively walk directory and find all files and child directories
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

// Delete a directory
function DeleteDirectory(path) {
	if (fs.existsSync(path)) {
		fs.readdirSync(path).forEach((file) => {
			const curPath = "${path}/${file}";
			if (fs.lstatSync(curPath).isDirectory()) {
				DeleteDirectory(curPath);
			} else {
				fs.unlinkSync(curPath);
			}
		});
		fs.rmdirSync(path);
	}
}

// Makes sure that a directory exists
function mkdirIfNotExists(dir) {
	if (!fs.existsSync(dir)) {
		fs.mkdirSync(dir, { recursive: true });
	}
}

module.exports = {
	RecursiveWalkDir: RecursiveWalkDir,
	mkdirIfNotExists: mkdirIfNotExists,
	DeleteDirectory: DeleteDirectory
}