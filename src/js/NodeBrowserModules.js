var child_process = require('child_process');

const BrowserBuiltinModules = { // Map each node module name to its browser module equivalent
    "assert": "assert",
    "buffer": "buffer",
    "console": "console-browserify",
    "constants": "constants-browserify",
    "crypto": "crypto-browserify",
    "domain": "domain-browser",
    "events": "events",
    "http": "stream-http",
    "https": "https-browserify",
    "os": "os-browserify",
    "path": "path-browserify",
    "punycode": "punycode",
    "querystring": "querystring-es3",
    "stream": "stream-browserify",
    "string_decoder": "string_decoder",
    "timers": "timers-browserify",
    "tty": "tty-browserify",
    "url": "url",
    "util": "util",
    "vm": "vm-browserify",
    "zlib": "browserify-zlib",
    "process": "process",
    "buffer": "buffer",
};

const InstallEnsured = [];

function CheckInstalled(PackageName) { // Checks whether a given nodeJS package is installed

    let Package = null;
    try {
        Package = require(BrowserBuiltinModules[PackageName]);
    } catch (error) {
        Package = null;
    }
    return Package !== null;
}

function EnsureInstalled(PackageName) {
    console.log("Package: " + PackageName);
    /*
    if (InstallEnsured.findIndex(PackageName) === -1) {
        InstallEnsured.push(PackageName);
    }*/
    if (!CheckInstalled(PackageName)) {
        console.log("Installing browser compatible node module " + PackageName + "... This is a 1 time install for this package, install all browser packages with 'arrowpack install-browser-modules'");
        child_process.execSync('npm install ' + BrowserBuiltinModules[PackageName], { stdio: [0, 1, 2] });
    }
}

function IsNodeBuiltin(PackageName) {
    return PackageName in BrowserBuiltinModules;
}

function NodeModuleBrowserPackageName(PackageName) {
    return BrowserBuiltinModules[PackageName];
}

module.exports = {
    BrowserBuiltinModules: BrowserBuiltinModules,
    EnsureInstalled: EnsureInstalled,
    IsNodeBuiltin: IsNodeBuiltin,
    NodeModuleBrowserPackageName: NodeModuleBrowserPackageName
};