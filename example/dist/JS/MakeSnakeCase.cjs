function Test() {
    console.log("Test!");
}
function MakeSnakeCase(str) {
    Test();
    str = str.replace(/ /g, '_');

    // Replace uppercase characters with lowercase versions preceded by an underscore using the MakeSnakeCase function
    str = str.replace(/[A-Z]/g, function (char) {
        return '_' + char.toLowerCase();
    });

    return str;

}
