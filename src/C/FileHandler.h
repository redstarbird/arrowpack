#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void CreateFileWrite(char *path, char *text);
char *ReadDataFromFile(char *path);
void CopyFile(char* FileToCopy, char* FileToCopyTo);

#endif // !FILEHANDLERH
