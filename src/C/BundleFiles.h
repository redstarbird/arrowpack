#ifndef BUNDLEFILES_H
#define BUNDLEFILES_H

#include <stdbool.h>
#include <stdio.h>

#include "../DependencyGraph/DependencyGraph.h"
#include "../Minifiers/HTMLMinifier.h"
#include "../Minifiers/JSMinifier.h"
#include "../Regex/RegexFunctions.h"
#include "./FileHandler.h"
#include "./JSImportedFunctions.h"
#include "FileTypesHandler.h"
#include "ProblemHandler.h"
#include "ProgressBar.h"
#include "StringRelatedFunctions.h"
#include "StringShiftHandler.h"

bool EMSCRIPTEN_KEEPALIVE BundleFiles(struct Graph *graph);

void BundleFile(struct Node *GraphNode);

#endif  // !BUNDLEFILES_H