// This file is used to compile all the C WASM functions into one file to minimize file size and WASM performance.

#include <stdio.h>
#include "./C/FileHandler.h"
#include "./DependencyGraph/DependencyGraph.h"
#include <emscripten.h>

void EMSCRIPTEN_KEEPALIVE CheckWasm()
{
    printf("Wasm started Successfully...\n");
}

void EMSCRIPTEN_KEEPALIVE PrintTimeTaken(int StartTime, int EndTime)
{
    printf("Time taken to complete(wasm): %d", (EndTime - StartTime) / 1000);
}