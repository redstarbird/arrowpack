#ifndef FILETYPESHANDLER_H
#define FILETYPESHANDLER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "emscripten.h"

typedef struct FileType
{
    char FileExtensions[8][16];
    char ShortName[10];
    unsigned int id;

} FileType;

#define CSSFILETYPE_ID 0
#define JSFILETYPE_ID 1
#define HTMLFILETYPE_ID 2
#define SCSSFILETYPE_ID 3
#define TYPESCRIPTFILETYPE_ID 4

void EMSCRIPTEN_KEEPALIVE InitFileTypes();

#endif // !FILETYPESHANDLER_H