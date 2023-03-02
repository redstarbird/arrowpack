import ESMTest, { GRAVITY as GRAVY, TestGRAVITY } from "../JS/ESMTest";

if (TestGRAVITY()) {
    console.log("Logging user in with regular earth gravity: " + GRAVY + "...");
}

ESMTest();