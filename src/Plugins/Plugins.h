#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <emscripten.h>
#include <stdbool.h>
#include "../DependencyGraph/DependencyGraph.h"
#include "../C/StringRelatedFunctions.h"
#include "../SettingsSingleton/settingsSingleton.h"
#include "../C/FileHandler.h"
#include "../C/ProblemHandler.h"

bool EMSCRIPTEN_KEEPALIVE ExecutePlugin(struct Graph *DependencyGraph, char *(*functionPTR)(char *, char *, char *), int pluginIndex);

#endif