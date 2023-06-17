#include <iostream>
#include "emscripten.h"

extern "C" {
void EMSCRIPTEN_KEEPALIVE CPPTESTINIT() {
        std::cout << "c++" << std::endl;
}}