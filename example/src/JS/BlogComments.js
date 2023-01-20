const MakeSnakeCase = require("MakeSnakeCase");


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

