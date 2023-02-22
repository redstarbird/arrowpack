
function ArrowSerialize(arr) {
    const separator = '\x1f'; // custom separator character
    const escape = '\x1e'; // custom escape character
    let encoded = '';
    for (let i = 0; i < arr.length; i++) {
        const str = arr[i];
        let encodedStr = '';
        for (let j = 0; j < str.length; j++) {
            const char = str.charAt(j);
            if (char === separator || char === escape) {
                encodedStr += escape + char;
            } else {
                encodedStr += char;
            }
        }
        encoded += encodedStr + separator;
    }
    return encoded;
}
function ArrowDeserialize(encoded) {
    const separator = '\x1f'; // custom separator character
    const escape = '\x1e'; // custom escape character
    let arr = [];
    let str = '';
    for (let i = 0; i < encoded.length; i++) {
        const char = encoded.charAt(i);
        if (char === separator) {
            arr.push(str);
            str = '';
        } else if (char === escape) {
            const nextChar = encoded.charAt(i + 1);
            if (nextChar === separator || nextChar === escape) {
                str += nextChar;
                i++;
            } else {
                str += char;
            }
        } else {
            str += char;
        }
    }
    if (str.length > 0) {
        arr.push(str);
    }
    return arr;
}
module.exports = { ArrowDeserialize: ArrowDeserialize, ArrowSerialize: ArrowSerialize }