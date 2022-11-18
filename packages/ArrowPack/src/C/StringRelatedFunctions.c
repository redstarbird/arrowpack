#include "StringRelatedFunctions.h"

int EMSCRIPTEN_KEEPALIVE GetLastFullstop(const char *text)
{
    int LastFullStop = -1;

    for (unsigned int i = 0; i < strlen(text); i++)
    {
        if (text[i] == '.')
        {
            LastFullStop = i;
        }
    }
    return LastFullStop;
}

int EMSCRIPTEN_KEEPALIVE LastOccurenceOfChar(const char *text, char character)
{
    int LastOccurence = -1;

    for (unsigned int i = 0; i < strlen(text); i++)
    {
        if (text[i] == character)
        {
            LastOccurence = i;
        }
    }
    return LastOccurence;
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
char *GetFileExtension(const char *path) // Returns the file extension for the given path without the dot char
{
    unsigned int pathLen = strlen(path);

    unsigned int lastFullStop = GetLastFullstop(path);
    if (lastFullStop == -1)
    {
        return NULL;
    }
    lastFullStop++;                            // Stops fullstop character being included
    const int length = pathLen - lastFullStop; // gets length of file ext
    char *extension = malloc(length + 1);
    for (int i = 0; i < length; i++)
    {
        extension[i] = path[lastFullStop + i]; // string doesn't include fullstop
    }
    extension[length] = '\0';
    return extension;
}

char EMSCRIPTEN_KEEPALIVE *getSubstring(char *Text, int StartIndex, int EndIndex) // Returns substring between start and end indexes
{
    const int substringLength = EndIndex - StartIndex + 1;    // Gets the length of the substring
    char *substring = malloc(sizeof(char) * substringLength); // Allocates memory for substring
    for (int i = 0; i < substringLength; i++)
    {
        substring[i] = Text[StartIndex + i];
    }
    substring[substringLength] = '\0'; // terminate string with null terminator
    return substring;
}

bool StringToBool(const char *str)
{
    return strcasecmp(str, "true") == 0;
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
    char *tempHolder; // Buffer to hold the absolute path

    if (path[0] == '/' || path[0] == '\\')
    {
        // tempHolder = malloc(sizeof(char *) * (strlen(path) + strlen(Settings.entry)) + 1);

        strcpy(tempHolder, Settings.entry);
        tempHolder[strlen(tempHolder) - 1] = '\0';
        strcat(tempHolder, path);
        return tempHolder;
    }
    else
    {

        int MatchesNum = GetNumOfRegexMatches(path, "\\.\\./"); // This is adding broken char to end of BasePath for some reason
        if (MatchesNum > 0)
        { // Handles paths containing ../
            if (BasePath[0] == '\0')
            { // BasePath is only needed for paths with ../
                printf("Error no base path specified");
                exit(1);
            }
            char *PathCopy;
            strcpy(PathCopy, BasePath); // Create a copy of the path variable so it doesn't get overwritten by strtok()

            char **SplitString = SplitStringByChar(PathCopy, '/');

            char *FinalString = malloc(sizeof(char) * (strlen(path) + strlen(BasePath) + strlen(Settings.entry) + 1)); // Probably very inefficient
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
                    }
                    ArrayIndex++;
                }
                FinalString[ArrayIndex] = '/';
                ArrayIndex++;
            }
            // FinalString[ArrayIndex] = '\0';
            char *TempEntry;
            strcpy(TempEntry, Settings.entry);
            strcat(TempEntry, FinalString);
            strcat(TempEntry, path);
            return TempEntry;
        }
        else
        {
            if (strstr(path, Settings.entry) != NULL) // path is already full path (might accidentally include paths with entry name in folder path)path o
            {
                return path;
            }
            char *TempPath = strdup(BasePath); // Very messy code
            printf("TempPath: %s, path: %s\n", TempPath, path);
            realloc(TempPath, (strlen(TempPath) + strlen(path) + 1) * sizeof(char));
            strcat(TempPath, path);
            return TempPath;
        }
    }
    return path; // Stops compiler from throwing exception on higher optimization levels
}

char *EMSCRIPTEN_KEEPALIVE GetBasePath(const char *filename)
{
    int LastPathChar = LastOccurenceOfChar(filename, '/') + 1;
    char *BasePath = (char *)malloc(LastPathChar * sizeof(char));
    for (unsigned int i = 0; i < LastPathChar; i++)
    {
        BasePath[i] = filename[i];
    }
    BasePath[LastPathChar] = '\0';
    return BasePath;
}

void ReplaceSectionOfString(char *string, int start, int end, const char *ReplaceString)
{
    /*
         if (end - start > strlen(ReplaceString))
         {
             printf("option 1\n");
             ShiftNum = strlen(ReplaceString) - (end - start);
         }
         else
         {
             printf("option 2\n");
             ShiftNum = strlen(ReplaceString);
         }*/
    ColorCyan();
    printf("String replace debug start\n");
    ColorNormal();
    int ShiftNum = strlen(ReplaceString) - (end - start);
    printf("Shift num: %i, replace string: %s, string: %s, end: %i\n", ShiftNum, ReplaceString, string, end);

    size_t stringlen = (ShiftNum + (int)strlen(string) + 1) * sizeof(char);
    if (ShiftNum > 0)
    {
        string = (char *)realloc(string, stringlen);
        string[stringlen] = '\0';
        for (unsigned int i = (int)stringlen - ShiftNum - 1; i >= end; i--)
        {                                     // Loops through all characters that need to be shifted to the right
            string[i + ShiftNum] = string[i]; // Shift the character the correct amount to the right
        }
        printf("Shifted string: %s\n", string);
        for (unsigned int i = start; i < strlen(ReplaceString); i++)
        {
            string[i] = ReplaceString[i - start];
        }
        printf("Final string: %s\n", string);
    }
    else if (ShiftNum < 0)
    {
        printf("not implemented yet\n");
        for (unsigned int i = end; i < strlen(string); i++) //
        {                                                   // Loops through all characters that need to be shifted to the left
            string[i + ShiftNum] = string[i];               // Shift the character the correct amount to the left
        }
        printf("Shifted string: %s\n", string);
        for (unsigned int i = 0; i < strlen(ReplaceString); i++)
        {
            string[start + i] = ReplaceString[i];
        }
        string[strlen(string) + ShiftNum] = '\0';
        printf("Finished string: %s\n", string);
    }
    ColorCyan();
    printf("String replace debug end\n");
    ColorNormal();
}

bool EMSCRIPTEN_KEEPALIVE StringStartsWith(const char *string, const char *substring)
{
    if (string[0] == '\0' || substring[0] == '\0')
    {
        return false;
    }
    return strncasecmp(substring, string, strlen(string)) == 0;
}

char *EntryToExitPath(const char *path)
{
    char *PathCopy = strdup(path);
    ReplaceSectionOfString(PathCopy, 0, strlen(Settings.entry), Settings.exit);
    return PathCopy;
}

void StringFormatInsert(char *string, const char *InsertString)
{
    // todo
}