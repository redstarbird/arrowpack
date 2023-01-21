#ifndef BUNDLEFILES_H
#define BUNDLEFILES_H

#include <stdio.h>
#include <stdbool.h>
#include "StringRelatedFunctions.h"
#include "../DependencyGraph/DependencyGraph.h"
#include "ProblemHandler.h"
#include "./FileHandler.h"
#include "../Minifiers/HTMLMinifier.h"
#include "FileTypesHandler.h"
#include "ProgressBar.h"
#include "../Regex/RegexFunctions.h"
#include "StringShiftHandler.h"
#include "../Minifiers/JSMinifier.h"

bool EMSCRIPTEN_KEEPALIVE BundleFiles(struct Graph *graph);

#endif // !BUNDLEFILES_H