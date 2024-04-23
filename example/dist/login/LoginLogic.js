function TestESMImports() {    console.log("Ecmascript imports are working");}const GRAVITY = 9.8;function TestGRAVITY() {    console.log("Gravity is: " + GRAVITY + "!");    return GRAVITY === 9.8;}let GRAVY=GRAVITY;

if (TestGRAVITY()) {
    console.log("Logging user in with regular earth gravity: " + GRAVY + "...");
}

ESMTest();