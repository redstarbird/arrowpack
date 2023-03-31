module.exports = function (FileContents) {
    return FileContents.replace(/\/\/[^\n]*\n/g, "");
}