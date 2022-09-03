#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include "DependencyTree.h"
#include "C/ReadFile.h"
#include <emscripten.h>
#include "C/cJSON/cJSON.h" // https://github.com/DaveGamble/cJSON

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

EXTERN EMSCRIPTEN_KEEPALIVE void SortDependencyTree(struct Node *tree, int treeLength)
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

EXTERN EMSCRIPTEN_KEEPALIVE bool containsCharacter(char *string, char character)
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

EXTERN EMSCRIPTEN_KEEPALIVE struct FileRule GetFileRuleFromPath(const char *path, struct FileRule *fileRules)
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

EXTERN EMSCRIPTEN_KEEPALIVE char *getSubstring(char *Text, int StartIndex, int EndIndex)
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

EXTERN EMSCRIPTEN_KEEPALIVE char *TurnToAbsolutePath(char *path)
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
    char *tempHolder; // Buffer to hold the absolute path
    if (path[0] == '/' || path[0] == '\\')
    {
        tempHolder = malloc(sizeof(char *) * (strlen(path) + strlen(entryPath)) + 1);
        tempHolder = *path + entryPath;
        realpath(tempHolder, path);
        return path;
    }
    else if (PATH_SEPARATOR == '/')
    { // Linux
        realpath(path, tempHolder);
        return tempHolder;
    }
}

EXTERN EMSCRIPTEN_KEEPALIVE char **FindAllRegexMatches(char *Text, struct FileRule rule)
{
    regex_t regexp;

    if (regcomp(&regexp, rule.regexPattern, 0) != 0)
    {
        fprintf(stderr, "Could not compile regex");
        exit(1);
    }; // compiles regex

    const int N_MATCHES = 128;

    regmatch_t match[N_MATCHES];

    int error = regexec(&regexp, Text, 0, match, 0);
    char **matches;
    unsigned int currentAmountOfChars = 0;

    unsigned int NumOfStrings = 0;
    unsigned int NumOfChars = 0;
    if (error == 0)
    {
        NumOfStrings = 1;
        currentAmountOfChars = match->rm_eo - match->rm_so;
        NumOfChars = currentAmountOfChars;

        matches = malloc(NumOfChars * sizeof(char *)); // no need to multiply NumOfChars by NumOfStrings here

        matches[0] = malloc(NumOfChars * sizeof(char));
        matches[0] = TurnToAbsolutePath(getSubstring(Text, (int)match->rm_so + rule.StartPos, (int)match->rm_eo - rule.EndPos));
    }
    else
    {
        return NULL;
    }

    while (1)
    {
        error = regexec(&regexp, Text, 0, match, 0);
        if (error)
        {
            break;
        }
        else
        {
            NumOfStrings++;
            currentAmountOfChars = match->rm_eo - match->rm_so;
            NumOfChars += currentAmountOfChars;

            matches = realloc(*matches, NumOfStrings * sizeof(char **)); // Allocate memory (this might be allocating too much memory)
            matches[NumOfStrings - 1] = malloc(currentAmountOfChars * sizeof(char));
            matches[NumOfStrings - 1] = TurnToAbsolutePath(getSubstring(Text, (int)match->rm_so + rule.StartPos, (int)match->rm_eo - rule.EndPos));
        }
    }
    return matches;
}

EXTERN EMSCRIPTEN_KEEPALIVE void FatalInvalidFile(const char *filename)
{ // Throws a fatal error with the given filename
    printf("Fatal error: %s is invalid\n", filename);
    exit(1);
}

EXTERN EMSCRIPTEN_KEEPALIVE struct FileRule *InitFileRules() // Gets file rules from FileTypes.json file
{
    char *rawJSON = ReadDataFromFile("FileTypes.json"); // string containing raw JSON from FileTypes.json file
    struct FileRule *fileRules = (struct FileRule *)malloc(sizeof(struct FileRule));
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

EXTERN EMSCRIPTEN_KEEPALIVE struct Node *CreateTree(char **paths, unsigned short int ArrayLength, char *entry)
{
    entryPath = entry;
    printf("ArrayLength:%d\n", ArrayLength);

    InitFileRules(); // creates list of file rule structs

    struct Node *Tree = malloc(sizeof(struct Node) * ArrayLength);

    for (unsigned int i = 0; i < ArrayLength; i++)
    {
        *Tree[i].path = *TurnToAbsolutePath(paths[i]);
        Tree[i].DependenciesInTree = 0;
        Tree[i].DependentsInTree = 0;
    }

    struct FileRule *fileRules = InitFileRules();

    for (int i = 0; i < ArrayLength; i++)
    {
        char *data = ReadDataFromFile(paths[i]);
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
