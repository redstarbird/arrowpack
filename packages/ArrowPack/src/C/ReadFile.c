#include "ReadFile.h"
#include <emscripten.h>

char EMSCRIPTEN_KEEPALIVE *ReadDataFromFile(char *path)
{ // returns contents of file
    printf("Path: %s\n", path);
    FILE *filePtr;
    filePtr = fopen(path, "r");
    char *buffer = 0;
    if (filePtr == NULL)
    {
        printf("%s has returned NULL\n", path);
        return NULL;
    }

    fseek(filePtr, 0, SEEK_END);  // seek to end of file
    long length = ftell(filePtr); // get length of file
    fseek(filePtr, 0, SEEK_SET);  // go back to start of file
    buffer = malloc(length);      // allocate memory for buffer to store file contents

    if (buffer)
    {
        fread(buffer, 1, length, filePtr); // read file into buffer
    }

    fclose(filePtr); // close the file

    return buffer;
}
