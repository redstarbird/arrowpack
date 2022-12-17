#ifndef FILETYPESHANDLER_H
#define FILETYPESHANDLER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "emscripten.h"
#include "./StringRelatedFunctions.h"

typedef struct FileType
{
    char FileExtensions[8][16];
    char ShortName[10];
    unsigned int id;

} FileType;

#define CSSFILETYPE_ID 0 // Can probably turn this into an enum
#define JSFILETYPE_ID 1
#define HTMLFILETYPE_ID 2
#define SCSSFILETYPE_ID 3
#define TYPESCRIPTFILETYPE_ID 4

#define FILETYPESIDNUM 5

void EMSCRIPTEN_KEEPALIVE InitFileTypes();

int GetFileTypeID(const char *path);

#endif // !FILETYPESHANDLER_H