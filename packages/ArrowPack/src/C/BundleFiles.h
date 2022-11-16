#ifndef BUNDLEFILES_H
#define BUNDLEFILES_H

#include <stdio.h>
#include <stdbool.h>
#include "StringRelatedFunctions.h"
#include "ReadFile.h"
#include "../DependencyTree/DependencyTree.h"
#include "ProblemHandler.h"
#include "ReadFile.h"
#include "FileHandler.h"

bool EMSCRIPTEN_KEEPALIVE BundleFiles(struct Node *DependencyTree);

#endif // !BUNDLEFILES_H5