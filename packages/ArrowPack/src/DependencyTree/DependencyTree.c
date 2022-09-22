#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include "DependencyTree.h"
#include "ReadFile.h"
#include <emscripten.h>
#include "./cJSON/cJSON.h" // https://github.com/DaveGamble/cJSON
#include "../Regex/RegexFunctions.h"

void EMSCRIPTEN_KEEPALIVE SortDependencyTree(struct Node *tree, int treeLength)
{
    struct Node tempHolder;
    for (int i = 0; i < treeLength - 1; i++)
    {
        if (tree[i].DependenciesInTree > tree[i + 1].DependenciesInTree)
        {
            tempHolder = tree[i];
            tree[i] = tree[i + 1];
            tree[i + 1] = tempHolder;
        }
        else if (tree[i].DependenciesInTree == tree[i + 1].DependenciesInTree)
        {
            if (tree[i].DependentsInTree < tree[i + 1].DependentsInTree)
            {
                tempHolder = tree[i];
                tree[i] = tree[i + 1];
                tree[i + 1] = tempHolder;
            }
        }
    }
    if (tree[0].DependenciesInTree > 0)
    {
        printf("Error: Dependency loop found in file %s.\n", tree[0].path);
        exit(1);
    }
}

char *GetFileExtension(const char *path) // Returns the file extension for the given path without the dot char
{
    unsigned int pathLen = strlen(path);

    unsigned int lastFullStop = 0;
    for (unsigned int i = 0; i < pathLen; i++)
    {
        if (path[i] == '.')
        {
            lastFullStop = i;
        }
    }
    if (lastFullStop == 0)
    {
        printf("Error: could not file character \".\" in path %s", path);
        exit(1);
    }

    lastFullStop++;                            // Stops fullstop character being included
    const int length = pathLen - lastFullStop; // gets length of file ext
    char *extension = malloc(length + 1);
    for (int i = 0; i < length; i++)
    {
        extension[i] = path[lastFullStop + i]; // string doesn't include fullstop
    }
    return extension;
}

char **SplitStringByChar(char *str, const char delimiter)
{
    unsigned int NumOfTokens = 0;
    char **Result = malloc(sizeof(char *));
    char *token = strtok(str, &delimiter);

    while (token != NULL)
    {
        NumOfTokens++;
        Result = (char **)malloc(sizeof(char *) * NumOfTokens + 1);
        Result[NumOfTokens - 1] = (char *)malloc(sizeof(char) * strlen(token));

        strcpy(Result[NumOfTokens - 1], token);
        token = strtok(NULL, &delimiter);
    }
    return Result;
}

bool EMSCRIPTEN_KEEPALIVE containsCharacter(char *string, char character) // Checks if string contains a certain character
{
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == character)
            return true;
    }
    return false;
}

char *entryPath; // global variable for entry (base) path

/*if (pathLen < charLen) {return false;}
unsigned short int o = 0;
for (unsigned int i = charLen; i >= 0; i--) {
    if (path[*pathLen-i] !=  extension[o]) {return false;}
    o++;
}
return true;*/

struct FileRule GetFileRuleFromPath(const char *path, struct FileRule *fileRules)
{

    unsigned int pathLen = strlen(path);

    int lastFullStop = 0;
    for (unsigned int i = 0; i < pathLen; i++)
    {
        if (path[i] == '.')
        {
            lastFullStop = i;
        }
    }
    if (lastFullStop == 0)
    {
        printf("Error: could not file character \".\" in path %s", path);
        exit(1);
    }

    lastFullStop++;
    const int length = pathLen - lastFullStop; // gets length of file ext
    char extension[length];
    for (int i = 0; i < length; i++)
    {
        extension[i] = path[lastFullStop + i]; // string includes fullstop
    }

    for (int i = 0; i < sizeof(*fileRules) / sizeof(struct FileRule); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%s\n", fileRules[i].FileExtensions[j]);
            if (fileRules[i].FileExtensions[j] == NULL || *fileRules[i].FileExtensions[j] == '\0')
            {
                break;
            }
            if (fileRules[i].FileExtensions[j] == extension)
            {
                return fileRules[i];
            }
        }
    }
    printf("Could not find rule for processing file %s\n", path);
    exit(1);
}

char EMSCRIPTEN_KEEPALIVE *getSubstring(char *Text, int StartIndex, int EndIndex) // Returns substring between start and end indexes
{
    const int substringLength = EndIndex - StartIndex + 1;
    char *substring = malloc(sizeof(char) * substringLength);
    for (int i = 0; i < EndIndex - StartIndex; i++)
    {
        substring[i] = Text[StartIndex + i];
    }
    substring[substringLength] = '\0'; // terminate string with null terminator
    return substring;
}

char EMSCRIPTEN_KEEPALIVE *TurnToFullRelativePath(char *path, char *BasePath)
{ // Turns a relative path into absolute path
    /*if (!containsCharacter(path, ':')) {
        if (PATH_SEPARATOR) {}
    }
    if (!containsCharacter(path,':') || PATH_SEPARATOR == '/') {
        if (path[0] == '/' || path[0] == '\\') {
            *path += 1;
            strcat(entryPath, path);
            return entryPath;
        }
    }*/
    printf("turning %s to absolute path", path);
    char *tempHolder; // Buffer to hold the absolute path

    if (path[0] == '/' || path[0] == '\\')
    {
        tempHolder = malloc(sizeof(char *) * (strlen(path) + strlen(entryPath)) + 1);
        strcpy(tempHolder, entryPath);
        strcat(tempHolder, path);
        return tempHolder;
    }
    else
    {
        int MatchesNum = GetNumOfRegexMatches(path, "\\.\\./");

        if (MatchesNum > 0)
        { // Handles paths containing ../

            if (BasePath == NULL)
            { // BasePath is only needed for paths with ../
                printf("Error no base path specified");
                exit(1);
                char *NeedVariableForNoError = malloc(sizeof(char));
                return NeedVariableForNoError;
            }
            char *PathCopy;
            strcpy(PathCopy, BasePath); // Create a copy of the path variable so it doesn't get overwritten by strtok()

            char **SplitString = SplitStringByChar(PathCopy, '/');

            char *FinalString = malloc(sizeof(char) * (strlen(path) + strlen(BasePath) + strlen(entryPath) + 1)); // Probably very inefficient
            int ArrayIndex = 0;
            for (int i = 0; i < (sizeof(SplitString) / sizeof(char *)) - MatchesNum; i++) // loops through array except for elements that need to be removed
            {
                for (int j = 0; j < sizeof(*SplitString[i]); j++) // Need to implement better way to do this
                {
                    if (!SplitString[i][j])
                    {
                        break;
                    }
                    if (SplitString[i][j] == '\0')
                    {
                        break;
                    }
                    else
                    {
                        FinalString[ArrayIndex] = SplitString[i][j];
                        printf("SplitString[i][j] = '%c'\n", SplitString[i][j]);
                    }
                    ArrayIndex++;
                }
                FinalString[ArrayIndex] = '/';
                ArrayIndex++;
            }
            // FinalString[ArrayIndex] = '\0';
            char *TempEntry;
            strcpy(TempEntry, entryPath);
            strcat(TempEntry, FinalString);
            strcat(TempEntry, path);
            return TempEntry;
        }
        else
        {
            if (strstr(path, entryPath) == NULL) // path is already full path (might accidentally include paths with entry name in folder path)path o
            {
                printf("Already full path: %s\n", path);
                return path;
            }
            char *TempPath; // Very messy code
            strcpy(TempPath, BasePath);
            strncat(TempPath, "/", 1);
            strcat(TempPath, path);
            return TempPath;
        }
    }
    char *NeedVariableForNoError = malloc(sizeof(char) * 2);
    NeedVariableForNoError[1] = '\0';
    return NeedVariableForNoError;
}

char EMSCRIPTEN_KEEPALIVE **GetDependencies(char *Path)
{
    char *FileExtension = GetFileExtension(Path);
    printf("File extension: %s\n", FileExtension);
    // Very unfortunate that C doesn't support switch statements for strings
    if (strcmp(FileExtension, "html") == 0 || strcmp(FileExtension, "htm") == 0)
    {
    }
    else if (strcmp(FileExtension, "css") == 0)
    {
        printf("CSS not implemented yet");
    }
    else if (strcmp(FileExtension, "js") == 0)
    {
        printf("JS not implemented yet");
    }
    else if (strcmp(FileExtension, "scss") == 0)
    {
        printf("SCSS not implemented yet");
    }

    char **temp = malloc(sizeof(char *) * 5); // just here temporarily so compiler doesnt throw an error
    return temp;
}

void EMSCRIPTEN_KEEPALIVE FatalInvalidFile(const char *filename)
{ // Throws a fatal error with the given filename
    printf("Fatal error: %s is invalid\n", filename);
    exit(1);
}

struct FileRule *InitFileRules() // Gets file rules from FileTypes.json file
{

    char *rawJSON = ReadDataFromFile("src/FileTypes.json"); // string containing raw JSON from FileTypes.json file

    struct FileRule *fileRules = (struct FileRule *)malloc(sizeof(struct FileRule) * 2);

    cJSON *json = cJSON_Parse(rawJSON);
    cJSON *currentArray;
    cJSON *currentElement;
    cJSON *fileTypesArray;

    unsigned short int currentTimesIterated = 0; // resets for every file rule
    unsigned int arrayElementCount = 0;
    unsigned short int currentFileTypesArrayIndex = 0;

    // char tempFileTypesBuffer[4][32] = {"\0", "\0", "\0", "\0"};

    if (cJSON_IsInvalid(json))
    {
        FatalInvalidFile("FileTypes.json");
    }

    cJSON_ArrayForEach(currentArray, json)
    {
        arrayElementCount++;
        if (cJSON_IsInvalid(currentArray))
        {
            FatalInvalidFile("FileTypes.json");
        }
        if (cJSON_IsArray(currentArray))
        {
            currentTimesIterated = 0;
            currentFileTypesArrayIndex = 0; // reset variables to zero

            fileRules = (struct FileRule *)realloc(fileRules, arrayElementCount * sizeof(struct FileRule));

            cJSON_ArrayForEach(currentElement, currentArray)
            {
                if (cJSON_IsInvalid(currentArray))
                {
                    FatalInvalidFile("FileTypes.json");
                }
                switch (currentTimesIterated)
                {

                case 0:
                    if (cJSON_IsArray(currentElement))
                    {
                        cJSON_ArrayForEach(fileTypesArray, currentElement)
                        {
                            strncpy(fileRules[arrayElementCount].FileExtensions[currentFileTypesArrayIndex], fileTypesArray->valuestring, 32);
                            currentFileTypesArrayIndex++;
                        }
                    }
                    else if (cJSON_IsString(currentElement))
                    {
                        strncpy(fileRules[arrayElementCount].FileExtensions[0], currentElement->valuestring, 32);
                    }
                    break;

                case 1:
                    if (!cJSON_IsString(currentElement))
                    {
                        FatalInvalidFile("FileTypes.json");
                    }
                    else
                    {
                        strncpy(fileRules[arrayElementCount].regexPattern, currentElement->valuestring, 64);
                    }
                    break;

                case 2:
                    if (!cJSON_IsNumber(currentElement))
                    {
                        FatalInvalidFile("FileTypes.json");
                    }
                    else
                    {
                        fileRules[arrayElementCount].StartPos = currentElement->valueint;
                    }
                    break;

                case 3:
                    if (!cJSON_IsNumber(currentElement))
                    {
                        FatalInvalidFile("File");
                    }
                    else
                    {
                        fileRules[arrayElementCount].EndPos = currentElement->valueint;
                    }
                    break;

                default:
                    break;
                }
                currentTimesIterated++;
            }
        }
        else
        {
            FatalInvalidFile("FileTypes.json");
        }
    }
    cJSON_Delete(json); // free memory allocated by cJSON
    return fileRules;
}

struct Node EMSCRIPTEN_KEEPALIVE *CreateTree(char *Wrapped_paths, int ArrayLength)
{
    int CurrentWrappedArrayIndex = 0;
    int lastNewElement = 0;
    int ElementsUnwrapped = 0;

    char **paths = malloc((ArrayLength) * sizeof(char *)); // Allocates memory for array of strings

    printf("%s\n", Wrapped_paths);

    while (ElementsUnwrapped <= ArrayLength && CurrentWrappedArrayIndex < strlen(Wrapped_paths))
    {

        if (Wrapped_paths[CurrentWrappedArrayIndex] == ':') // Checks for paths divider character thing
        {
            paths[ElementsUnwrapped] = (char *)malloc((CurrentWrappedArrayIndex - lastNewElement + 1) * sizeof(char));

            for (int i = 0; i < CurrentWrappedArrayIndex - lastNewElement; i++)
            {
                paths[ElementsUnwrapped][i] = Wrapped_paths[lastNewElement + i];
            }
            printf("this path is %s\n", paths[ElementsUnwrapped]);
            printf("This file extension is %s\n", GetFileExtension(paths[ElementsUnwrapped]));
            lastNewElement = CurrentWrappedArrayIndex + 2;
            ElementsUnwrapped++;
            CurrentWrappedArrayIndex++;
        }
        CurrentWrappedArrayIndex++;
    }
    free(Wrapped_paths);
    printf("test\n");
    struct Node *Tree = malloc(sizeof(struct Node) * ArrayLength);

    for (unsigned int i = 0; i < ArrayLength; i++)
    {

        //*Tree[i].path = *TurnToFullRelativePath(paths[i], NULL);
        strcpy(Tree[i].path, TurnToFullRelativePath(paths[i], NULL));
        Tree[i].DependenciesInTree = 0;
        Tree[i].DependentsInTree = 0;
    }

    struct FileRule *fileRules = InitFileRules();

    for (int i = 0; i < ArrayLength; i++)
    {

        char *data = ReadDataFromFile(paths[i]);
        printf("Data %s\n", data);
        if (data == NULL)
        {
            continue;
        }

        char **Dependencies = FindAllRegexMatches(data, GetFileRuleFromPath(paths[i], fileRules));
        char *iteratePointer = Dependencies[0];
        while (iteratePointer++ != NULL)
        {
            for (int j = 0; j < ArrayLength; j++)
            {
                if (*Tree[j].path == *iteratePointer)
                {
                    Tree[i].Dependencies[Tree[i].DependenciesInTree] = Tree[j];
                    Tree[j].Dependents[Tree[j].DependentsInTree] = Tree[i];
                    Tree[i].DependenciesInTree++;
                    Tree[j].DependentsInTree++;
                    free(iteratePointer);
                    break;
                }
            }
        }
        free(Dependencies);
        free(iteratePointer);
    }

    return Tree;
}
