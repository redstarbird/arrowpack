#ifndef BUNDLEFILES_H
#define BUNDLEFILES_H

#include <stdbool.h>
#include <stdio.h>

#include "../bridge/JSImportedFunctions.h"
#include "../graph/DependencyGraph.h"
#include "../graph/FileTypesHandler.h"
#include "../minify/HTMLMinifier.h"
#include "../minify/JSMinifier.h"
#include "../util/FileUtil.h"
#include "../util/ProblemHandler.h"
#include "../util/ProgressBar.h"
#include "../util/RegexUtil.h"
#include "../util/StringUtil.h"
#include "StringShiftHandler.h"

bool EMSCRIPTEN_KEEPALIVE BundleFiles(struct Graph *graph);

void BundleFile(struct Node *GraphNode);

#endif  // !BUNDLEFILES_H