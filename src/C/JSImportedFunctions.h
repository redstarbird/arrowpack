#if !defined(JSIMPORTEDFUNCTIONS_H)
#define JSIMPORTEDFUNCTIONS_H
#include <stdbool.h>
#include <emscripten.h>
#include <stdlib.h>

extern EMSCRIPTEN_KEEPALIVE bool IsNodeBuiltInJS(char *name);
extern EMSCRIPTEN_KEEPALIVE void EnsureNodeBuiltinBrowserModuleJS(char *name);
extern EMSCRIPTEN_KEEPALIVE char *NodeModuleBrowserPackageNameJS(char *name);
bool IsNodeBuiltin(char *name);
void EnsureNodeBuiltinBrowserModule(char *name);
char *NodeModuleBrowserPackageName(char *name);
#endif // JSIMPORTEDFUNCTIONS_H
