#ifndef BUNDLEFILES_H
#define BUNDLEFILES_H

#include <stdio.h>
#include <stdbool.h>
#include "StringRelatedFunctions.h"
#include "../DependencyTree/DependencyTree.h"
#include "ProblemHandler.h"
#include "./FileHandler.h"
#include "../Minifiers/HTMLMinifier.h"
#include "FileTypesHandler.h"

bool EMSCRIPTEN_KEEPALIVE BundleFiles(struct Graph *graph);

#endif // !BUNDLEFILES_H5