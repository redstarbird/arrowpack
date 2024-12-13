const fs = require('fs');
const path = require('path');

/**
 * Recursively count lines of code for each programming language in the given directory.
 * 
 * @param {string} dir - Directory to search in.
 * @param {Object} options - Options for the search.
 * @param {Set<string>} options.excludedDirs - Directories to exclude.
 * @param {Object} options.languageExtensions - Map of programming languages to file extensions.
 * @returns {Object} - An object containing lines of code for each language.
 */
function countLinesOfCodeByLanguage(dir, { excludedDirs = new Set(), languageExtensions = {} } = {}) {
    const lineCounts = {}; // To store line counts for each language

    const files = fs.readdirSync(dir, { withFileTypes: true });

    for (const file of files) {
        const filePath = path.join(dir, file.name);

        if (file.isDirectory()) {
            if (!excludedDirs.has(file.name)) {
                const subDirCounts = countLinesOfCodeByLanguage(filePath, { excludedDirs, languageExtensions });
                for (const [language, lines] of Object.entries(subDirCounts)) {
                    lineCounts[language] = (lineCounts[language] || 0) + lines;
                }
            }
        } else {
            const ext = path.extname(file.name);
            const language = Object.keys(languageExtensions).find(lang => languageExtensions[lang].has(ext));

            if (language) {
                const fileContent = fs.readFileSync(filePath, 'utf8');
                const lineCount = fileContent.split('\n').length;
                lineCounts[language] = (lineCounts[language] || 0) + lineCount;
            }
        }
    }

    return lineCounts;
}

const excludedDirs = new Set(['node_modules', '.git', 'cJSON', '.vs', 'Build', 'OldFiles']);
const languageExtensions = {
    JavaScript: new Set(['.js', '.jsx']),
    TypeScript: new Set(['.ts', '.tsx']),
    C: new Set(['.c', '.h']),
    CPlusPlus: new Set(['.cpp', '.hpp']),
    Python: new Set(['.py']),
    Go: new Set(['.go']),
    Fortran: new Set(['.for']),
    Rust: new Set(['.rs']),
};

const cwd = process.cwd();
const lineCounts = countLinesOfCodeByLanguage(cwd, { excludedDirs, languageExtensions });

console.log('\x1b[1mLines of code by programming language:\x1b[0m');

// Determine the maximum width for aligned output
const maxLanguageLength = Math.max(...Object.keys(lineCounts).map(lang => lang.length));
const maxLineCountLength = Math.max(...Object.values(lineCounts).map(lines => lines.toString().length));

// Sort entries by line count in descending order
const sortedEntries = Object.entries(lineCounts).sort(([, a], [, b]) => b - a);

for (const [language, lines] of sortedEntries) {
    const languageFormatted = language.padEnd(maxLanguageLength, ' ');
    const linesFormatted = lines.toString().padStart(maxLineCountLength, ' ');
    console.log(`\x1b[36m${languageFormatted}\x1b[0m: \x1b[33m${linesFormatted}\x1b[0m`);
}
