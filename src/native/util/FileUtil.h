#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ProblemHandler.h"
#include "StringUtil.h"

void CreateFileWrite(char *path, char *text);
char *ReadDataFromFile(char *path);
void CopyFile(char *FileToCopy, char *FileToCopyTo);
bool FileExists(char *FilePath);
void EnsureDirectory(const char *DirectoryPath);
bool DirectoryExists(const char *path);
char **GetAllFilesInDirectory(char *directoryPath, bool recursive, int *fileCount);

#endif  // !FILEUTIL_H
