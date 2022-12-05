// This doesn't work currently :()
const fs = require("fs");

function oneLine(inputPath) {
    // Read the contents of the input file
    let program = fs.readFileSync(inputPath, "utf8");

    // Remove comments
    program = program.replace(/\/\*(.|\s)*?\*\//g, "");
    program = program.replace(/\/\/.*/g, "");

    // Remove newline characters and compress multiple spaces into a single space
    program = program.replace(/\n/g, "").replace(/\s+/g, " ");

    // Split the program into an array of individual statements
    const statements = program.split(";");

    // Re-indent each statement according to its level of nesting
    let indent = 0;
    for (let i = 0; i < statements.length; i++) {
        const statement = statements[i].trim();

        // If the statement is a closing brace, decrease the indent level
        if (statement.endsWith("}")) {
            indent--;
        }
        // Set indent to 0 if it is less than 0
        indent = Math.max(indent, 0);

        // Re-indent the statement with the current indent level
        statements[i] = " ".repeat(indent * 2) + statement;

        // If the statement is an opening brace, increase the indent level
        if (statement.endsWith("{")) {
            indent++;
        }
    }

    // Join the re-indented statements into a single line
    const result = statements.join("; ");

    // Overwrite the input file with the result
    fs.writeFileSync(inputPath, result);
}

oneLine("Build/CFunctions.js");