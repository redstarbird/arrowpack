// This file is used to compile all the C WASM functions into one file to minimize file size and WASM performance.

#include <stdio.h>
#include "./C/ReadFile.h"
#include "./C/DependencyTree.h"
#include <emscripten.h>

void EMSCRIPTEN_KEEPALIVE CheckWasm()
{
    printf("Wasm started Successfully...\n");
}