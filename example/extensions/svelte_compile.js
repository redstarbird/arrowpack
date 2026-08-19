const fs = require('fs');

module.exports = function svelte_compile(fileName) {
    const fileContent = fs.readFileSync(fileName, 'utf8');
    const { js, css } = processSvelteFile(fileName, fileContent);
}