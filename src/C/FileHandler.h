#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

void CreateFileWrite(char *path, char *text);
char *ReadDataFromFile(char *path);
void CopyFile(char *FileToCopy, char *FileToCopyTo);
bool FileExists(char *FilePath);
#endif // !FILEHANDLERH
