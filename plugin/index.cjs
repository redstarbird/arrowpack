function ValidatorResult(result, errors, warnings) { // Function prototype for the result of validating a file
    this.result = result; // Holds the result as a bool of whether there are any errors
    this.errors = errors || []; // Holds the errors for the file
    this.warnings = warnings || [] // Holds the warnings for the file
}

class Validator {
    constructor(ValidateFunction, validatorName) {
        this.ValidateFunction = ValidateFunction; // The function that is run to validate files
        this.Result = new ValidatorResult(true, [], []); // Initialises the result function prototype
        this.name = validatorName || ""; // The name of the validator plugin
    }

    AddError(problems) { // Add an error to the validator result
        if (!this.Result.errors) { // Initialises errors array if it doesn't exist
            if (typeof problems === "object") {
                this.Result.errors = problems;
            }
            else {
                this.Result.errors = [problems];

            }
        }
        else if (typeof problems === "object") {
            for (let i = 0; i < problems.length; i++) {
                this.Result.errors.push(problems[i]); // Adds each error to the array if an array is input to the function
            }
        }
        else {
            this.Result.errors.push(problems); // Adds the error to the error array
        }
    }
    AddWarning(problems) { // Add a warning to the validator result
        if (!this.Result.warnings) { // Initialises warnings array if it doesn't exist
            if (typeof problems === "object") {
                this.Result.warnings = problems;
            }
            else {
                this.Result.warnings = [problems];

            }
        }
        else if (typeof problems === "object") {
            for (let i = 0; i < problems.length; i++) {
                this.Result.warnings.push(problems[i]); // Adds each warning to the array if an array is input to the function
            }
        }
        else {
            this.Result.warnings.push(problems); // Adds the warning to the warning array
        }
    }
    AddResult(result) { // Adds the final result
        this.result = result;
    }
    Validate(filecontents, filepath) { // Validates the current file
        const result = this.asyncValidate(filecontents, filepath); // Runs the function for the validator plugin
        if (result !== undefined && result !== null) {
            return result; // Return the result if it is returned by the function 
        } else {
            console.log("Validator " + this.name + "does not return a ValidatorResult object, it is best pratice to insure it does!");
            return this.Result; // Returns the result object if not returned by the plugin
        }
    }


}

module.exports = { Validator: Validator };