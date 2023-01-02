function MakeSnakeCase(str) {
    str = str.replace(/ /g, '_');

    // Replace uppercase characters with lowercase versions preceded by an underscore
    str = str.replace(/[A-Z]/g, function (char) {
        return '_' + char.toLowerCase();
    });

    return str;

}
let MakeSnakeCase_ARROWPACK = {};MakeSnakeCase_ARROWPACK = { SnakeCase: MakeSnakeCase };const MakeSnakeCaseModule = MakeSnakeCase_ARROWPACK;
let Comments = ["This is a comment", "This is another comment", "This is yet another comment"];
function LogComments(params) {
    for (let i = 0; i < params.length; i++) {
        MakeSnakeCaseModule.SnakeCase(console.log(params[i]));
    }
}

LogComments(Comments);
module.exports = { LogComments: LogComments };