


mergeInto(LibraryManager.library, {
    IsNodeBuiltInJS: function (name) {
        name = Module.UTF8ToString(name);
        const NodeBrowserModules = require("../src/js/NodeBrowserModules.js");
        return NodeBrowserModules.IsNodeBuiltin(name);
    },
    EnsureNodeBuiltinBrowserModuleJS: function (name) {
        name = Module.UTF8ToString(name);
        const NodeBrowserModules = require("../src/js/NodeBrowserModules.js");
        NodeBrowserModules.EnsureInstalled(name);
    },
    NodeModuleBrowserPackageNameJS: function (name) {
        name = Module.UTF8ToString(name);
        const NodeBrowserModules = require("../src/js/NodeBrowserModules.js");
        const string = NodeBrowserModules.NodeModuleBrowserPackageName(name);
        console.log(string);
        var lengthBytes = Module.lengthBytesUTF8(string) + 1;
        var stringOnWasmHeap = Module._malloc(string);

        return Module.stringToNewUTF8(string); // Return the encoded string
    }
})