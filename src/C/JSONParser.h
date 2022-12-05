#ifndef JSONPARSER_H
#define JSONPARSER_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char * ParseJSON(const char * json);

struct FileRule * ParseJSONToStructArray(char * RawJSON);


#endif