#ifndef FINDDEPENDENCIES_H
#define FINDDEPENDENCIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>
#include "../C/FileHandler.h"
#include "../Regex/RegexFunctions.h"
#include "../C/StringRelatedFunctions.h"
#include "../SettingsSingleton/settingsSingleton.h"
#include "../C/cJSON/cJSON.h" // https://github.com/DaveGamble/cJSON
#include "../C/ProblemHandler.h"
#include "DependencyGraph.h"
#include "../C/JSImportedFunctions.h"

RegexMatch EMSCRIPTEN_KEEPALIVE *BasicRegexDependencies(char *filename, const char *pattern, unsigned int Startpos, unsigned int Endpos, struct RegexMatch *CommentLocations);

RegexMatch EMSCRIPTEN_KEEPALIVE *FindHTMLDependencies(struct Node *vertex, struct Graph **DependencyGraph);

RegexMatch EMSCRIPTEN_KEEPALIVE *FindCSSDependencies(char *filename);

struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindJSDependencies(char *filename);

#endif // !FINDDEPENENCIESH
