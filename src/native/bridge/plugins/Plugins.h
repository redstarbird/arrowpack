#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <stdbool.h>

#include "../../config.h"
#include "../../graph/DependencyGraph.h"
#include "../../util/FileUtil.h"
#include "../../util/ProblemHandler.h"
#include "../../util/StringUtil.h"
#include "../Export.h"

bool EMSCRIPTEN_KEEPALIVE ExecutePlugin(struct Graph *DependencyGraph, char *(*functionPTR)(char *, char *, char *),
                                        int pluginIndex);

#endif