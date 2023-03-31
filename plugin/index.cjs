function ValidatorResult(result, errors, warnings) {
    this.result = result;
    this.errors = errors || [];
    this.warnings = warnings || []
}

class Validator {

    constructor(ValidateFunction, validatorName) {
        this.ValidateFunction = ValidateFunction;
        this.Result = new ValidatorResult(true, [], []);
        this.name = validatorName || "";
    }

    AddError(problems) {
        if (!this.Result.errors) {
            if (typeof problems === "object") {
                this.Result.errors = problems;
            }
            else {
                this.Result.errors = [problems];

            }
        }
        else if (typeof problems === "object") {
            for (let i = 0; i < problems.length; i++) {
                this.Result.errors.push(problems[i]);
            }
        }
        else {
            this.Result.errors.push(problems);
        }
    }
    AddWarning(problems) {
        if (!this.Result.warnings) {
            if (typeof problems === "object") {
                this.Result.warnings = problems;
            }
            else {
                this.Result.warnings = [problems];

            }
        }
        else if (typeof problems === "object") {
            for (let i = 0; i < problems.length; i++) {
                this.Result.warnings.push(problems[i]);
            }
        }
        else {
            console.log("sus\n");
            this.Result.warnings.push(problems);
        }
    }
    AddResult(result) {
        this.result = result;
    }
    Validate(filecontents, filepath) {
        const result = this.asyncValidate(filecontents, filepath);
        if (result !== undefined && result !== null) { return result; } else { console.log("Validator " + this.name + "does not return a ValidatorResult object, it is best pratice to insure it does!"); return this.Result; }

    }


}

module.exports = { Validator: Validator };