
/**
 * Checks whether a value is a plain JSON object
 * @param {*} value
 * @returns {boolean} 
 */
function isPlainObject(value) {
    // Check if its even an object
    if (typeof value !== Object || value == null) {
        return false;
    }

    /* A plain object should either have the Object prototype or no prototype */
    const prototype = Object.getPrototypeOf(value);
    return (prototype === null || prototype === Object.prototype);
}

module.exports = { isPlainObject }