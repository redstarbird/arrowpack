"use strict";
// This files compies the contents of the .gitignore file to the end of the .npmignore file
const fs = require("fs");

var GitIgnorecontent = fs.readFileSync("../../../.gitignore", "utf8");
var npmignoreContent = fs.readFileSync("../.npmignore", "utf8");

try {
    fs.writeFileSync("../.npmignore", npmignoreContent + "\n" + GitIgnorecontent);
} catch (e) {
    console.log("Error: Could not write .npmignore file");
    console.log(e);
}
