
/**
 * Checks whether a value is a plain JSON object
 * @param {*} value
 * @returns {boolean} 
 */
function isPlainObject(value) {
    // Check if its even an object
    if (typeof value !== "object" || value == null) {
        return false;
    }

    /* A plain object should either have the Object prototype or no prototype */
    const prototype = Object.getPrototypeOf(value);
    return (prototype === null || prototype === Object.prototype);
}

/**
 * Returns whether an object is empty or not
 * @param {Object} object 
 * @returns {boolean}
 */
function isObjectEmpty(object) {
    for (var Property in object) {
        if (object.hasOwnProperty(Property)) {
            return false;
        }
    }
    return true;
}

module.exports = { isPlainObject, isObjectEmpty }