#ifndef JSONPARSER_H
#define JSONPARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *ParseJSON(const char *json);

struct FileRule *ParseJSONToStructArray(char *RawJSON);

#endif