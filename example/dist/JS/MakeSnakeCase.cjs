function Test() {
    console.log("Test!");
}
function MakeSnakeCase(str) {
    Test();
    str = str.replace(/ /g, '_');

        str = str.replace(/[A-Z]/g, function (char) {
        return '_' + char.toLowerCase();
    });

    return str;

}
module.exports = { SnakeCase: MakeSnakeCase };