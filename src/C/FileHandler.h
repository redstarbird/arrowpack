#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "StringRelatedFunctions.h"
#include "ProblemHandler.h"

void CreateFileWrite(char *path, char *text);
char *ReadDataFromFile(char *path);
void CopyFile(char *FileToCopy, char *FileToCopyTo);
bool FileExists(char *FilePath);
void EnsureDirectory(const char *DirectoryPath);
bool DirectoryExists(const char *path);
char **GetAllFilesInDirectory(char *directoryPath, bool recursive, int *fileCount);

#endif // !FILEHANDLERH
