module.exports = RecursiveWalkDir;
const fs = require("fs");
const path = require("path");

const RecursiveWalkDir = function (path, Files, Directories) {
	InitialFiles = fs.readdirSync(path);

	Files = Files || [];
	Directories = Directories || [];

	InitialFiles.forEach(function (file) {
		if (fs.statSync(file).isDirectory()) {
			Directories.push(file);
			RecursiveWalkDir(path + "/" + file, Files, Directories);
		} else {
			Files.push(path.join(path, file));
		}
	});
	return Files, Directories;
};
