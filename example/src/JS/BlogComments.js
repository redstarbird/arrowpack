const MakeSnakeCaseModule = require("MakeSnakeCase");
let Comments = ["This is a comment", "This is another comment", "This is yet another comment"];
function LogComments(params) {
    for (let i = 0; i < params.length; i++) {
        MakeSnakeCaseModule.SnakeCase(console.log(params[i]));
    }
}

LogComments(Comments);
module.exports = { LogComments: LogComments };