#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <emscripten.h>
#include <stdbool.h>

#include "../C/FileHandler.h"
#include "../C/StringRelatedFunctions.h"
#include "../DependencyGraph/DependencyGraph.h"
#include "../SettingsSingleton/settingsSingleton.h"

bool EMSCRIPTEN_KEEPALIVE TransformFiles(struct Graph *DependencyGraph, char *(*functionPTR)(char *, char *, char *));

#endif