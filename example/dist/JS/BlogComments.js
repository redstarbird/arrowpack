function TestA2() {    console.log("Test!");}function MakeSnakeCaseA3(str) {    TestA2();    str = str.replace(/ /g, '_');        str = str.replace(/[A-Z]/g, function (char) {        return '_' + char.toLowerCase();    });    return str;}let MakeSnakeCase_ARROWPACK = {};MakeSnakeCase_ARROWPACK = { SnakeCase: MakeSnakeCaseA3 };const MakeSnakeCase = MakeSnakeCase_ARROWPACK;


let Comments = ["This is a comment", "This is another comment", "This is yet another comment"];
function Test(TestString) {
    console.log("Testing " + TestString);
}
function LogComments(params) {
    for (let i = 0; i < params.length; i++) {
        console.log(MakeSnakeCase.SnakeCase(params[i]));
    }
}

LogComments(Comments);
module.exports = { LogComments: LogComments };

