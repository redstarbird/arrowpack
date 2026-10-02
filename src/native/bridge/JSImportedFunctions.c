#include "./JSImportedFunctions.h"

bool IsNodeBuiltin(char *name)
{
    return IsNodeBuiltInJS(name);
}

void EnsureNodeBuiltinBrowserModule(char *name)
{
    EnsureNodeBuiltinBrowserModuleJS(name);
}

char *NodeModuleBrowserPackageName(char *name)
{
    return NodeModuleBrowserPackageNameJS(name);
}