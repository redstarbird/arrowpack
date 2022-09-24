#ifndef FINDDEPENDENCIES_H
#define FINDDEPENDENCIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

char EMSCRIPTEN_KEEPALIVE **BasicRegexDependencies(const char *filename, const char *pattern, unsigned int Startpos, unsigned int Endpos);

char EMSCRIPTEN_KEEPALIVE **FindHTMLDependencies(const char *filename);

char EMSCRIPTEN_KEEPALIVE **FindCSSDependencies(const char *filename);

#endif // !FINDDEPENENCIESH
