const fs = require('fs');

// This function takes the path to a directory as an argument, along with an array of directory names to exclude, and prints the total number of lines of code in all the files in that directory and its subdirectories, excluding the directories in the exclude list
function countLines(directoryPath, excludeList) {
    // Read the contents of the directory
    const directoryContents = fs.readdirSync(directoryPath);

    // Initialize the total number of lines to 0
    let totalLines = 0;

    // Iterate over the contents of the directory
    for (const fileName of directoryContents) {
        // Construct the full path to the file
        const filePath = `${directoryPath}/${fileName}`;

        // Check if the file is a regular file (not a directory or symlink, etc.)
        if (fs.statSync(filePath).isFile()) {
            // If the file is a regular file, check if it is a code file by checking its file extension
            if (filePath.endsWith('.js') || filePath.endsWith('.py') || filePath.endsWith('.java') || filePath.endsWith('.cpp') || filePath.endsWith('.c')) {
                // Read the contents of the file
                const fileContents = fs.readFileSync(filePath, 'utf8');

                // Split the contents of the file on the newline character to get an array of lines
                const lines = fileContents.split('\n');

                // Print the number of lines in the file
                console.log(`${filePath}: ${lines.length}`);

                // Add the number of lines in the file to the total number of lines
                totalLines += lines.length;
            }
        } else if (fs.statSync(filePath).isDirectory() && !excludeList.includes(fileName)) {
            // If the file is a directory, and it is not in the exclude list, recursively count the lines of code in the directory
            totalLines += countLines(filePath, excludeList);
        }
    }

    // Return the total number of lines
    return totalLines;

}

// Example usage
console.log(`Total: ${countLines('./', ['node_modules', '.git', 'cJSON', '.vs', 'Build', "OldFiles"])}`);
