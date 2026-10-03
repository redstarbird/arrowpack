#ifndef FINDDEPENDENCIES_H
#define FINDDEPENDENCIES_H

#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../bridge/JSImportedFunctions.h"
#include "../config.h"
#include "../external/cJSON/cJSON.h"  // https://github.com/DaveGamble/cJSON
#include "../util/FileUtil.h"
#include "../util/ProblemHandler.h"
#include "../util/RegexUtil.h"
#include "../util/StringUtil.h"
#include "./DependencyGraph.h"

RegexMatch EMSCRIPTEN_KEEPALIVE *BasicRegexDependencies(char *filename, const char *pattern, unsigned int Startpos,
                                                        unsigned int Endpos, struct RegexMatch *CommentLocations);

RegexMatch EMSCRIPTEN_KEEPALIVE *FindHTMLDependencies(struct Node *vertex, struct Graph **DependencyGraph);

RegexMatch EMSCRIPTEN_KEEPALIVE *FindCSSDependencies(char *filename);

struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindJSDependencies(char *filename);

#endif  // !FINDDEPENENCIESH
