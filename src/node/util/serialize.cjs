
// Serializes multiple strings into a single string
function ArrowSerialize(arr) {
    const separator = '\x1f'; // custom separator character
    const escape = '\x1e'; // custom escape character
    let encoded = '';
    for (let i = 0; i < arr.length; i++) { // Loop through array of strings
        const str = arr[i];
        let encodedStr = '';
        for (let j = 0; j < str.length; j++) { // Check for escape character
            const char = str.charAt(j);
            if (char === separator || char === escape) {
                encodedStr += escape + char;
            } else {
                encodedStr += char;
            }
        }
        encoded += encodedStr + separator; // Add string to encoded string seperated by separator character
    }
    return encoded;
}

// Deserializes a single string into multiple strings
function ArrowDeserialize(encoded) {
    const separator = '\x1f'; // custom separator character
    const escape = '\x1e'; // custom escape character
    let arr = [];
    let str = '';
    for (let i = 0; i < encoded.length; i++) { // Loop through the encoded string
        const char = encoded.charAt(i);
        if (char === separator) { // Start of new string, push built string to array
            arr.push(str);
            str = ''; // Reset built string
        } else if (char === escape) {
            const nextChar = encoded.charAt(i + 1);
            if (nextChar === separator || nextChar === escape) {
                str += nextChar;
                i++;
            } else {
                str += char; // Add character to built string
            }
        } else {
            str += char; // Add character to built string
        }
    }
    if (str.length > 0) { // Push the final built string to the array
        arr.push(str);
    }
    return arr;
}
module.exports = { ArrowDeserialize: ArrowDeserialize, ArrowSerialize: ArrowSerialize }