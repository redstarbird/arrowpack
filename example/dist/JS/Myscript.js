function HelloWorldIfy(str) {
    return "Hello World " + str;
}
let helloworldify_ARROWPACK = {};helloworldify_ARROWPACK = HelloWorldIfy;function ThisIsAModuleTestFunction() {
    console.log("CommonJS module!");
}let MyModulecjs_ARROWPACK = {};MyModulecjs_ARROWPACK.CJSModuleFunction = ThisIsAModuleTestFunction;const HelloWorldify = helloworldify_ARROWPACK;
const CJSModuleTest = MyModulecjs_ARROWPACK;

CJSModuleTest.CJSModuleFunction();


console.log(HelloWorldify("This is javascript!"));