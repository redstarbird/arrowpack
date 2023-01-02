function MakeSnakeCase(str) {
    str = str.replace(/ /g, '_');

    // Replace uppercase characters with lowercase versions preceded by an underscore
    str = str.replace(/[A-Z]/g, function (char) {
        return '_' + char.toLowerCase();
    });

    return str;

}
module.exports = { SnakeCase: MakeSnakeCase };