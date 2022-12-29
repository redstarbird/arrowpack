#include "StringRelatedFunctions.h"

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

    unsigned int lastFullStop = LastOccurenceOfChar(path, '.');
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
        tempHolder = malloc(sizeof(char) * (strlen(path) + strlen(Settings.entry)) + 1);
        printf("starts with /\n");
        strcpy(tempHolder, Settings.entry);
        tempHolder[strlen(tempHolder)] = '\0';
        strcat(tempHolder, path + 1);
        printf("/ path: %s\n", tempHolder);
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
            printf("Found ../ %i times!\n", MatchesNum);
            char *PathCopy = strdup(path);
            char *BasePathCopy = strdup(BasePath);
            printf("BasePath: %s\n", BasePathCopy);
            // Create a copy of the path variable so it doesn't get overwritten by strtok()

            int PathSeperatorsFound = 0;
            int LocationFound = -1;
            int BasePathLength = strlen(BasePathCopy);
            if (BasePathCopy[BasePathLength - 1] == '/')
            {
                BasePathCopy[BasePathLength - 1] = '\0';
                BasePathLength--;
            }
            for (int i = BasePathLength; i > 0; i--)
            {
                if (BasePathCopy[i] == '/')
                {
                    PathSeperatorsFound++;
                    if (PathSeperatorsFound == MatchesNum)
                    {
                        LocationFound = i;
                        break;
                    }
                }
            }

            RemoveSubstring(PathCopy, "../");
            BasePathCopy = ReplaceSectionOfString(BasePathCopy, LocationFound + 1, strlen(BasePathCopy), PathCopy);
            /*
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
            char *TempEntry = strdup(Settings.entry);

            strcat(TempEntry, FinalString);
            strcat(TempEntry, path);
            printf("done\n");
            printf("%s\n", TempEntry);*/
            printf("%s\n", BasePathCopy);
            return BasePathCopy;
        }
        else
        {
            if (strstr(path, Settings.entry) != NULL) // path is already full path (might accidentally include paths with entry name in folder path)path o
            {
                return path;
            }
            char *TempPath = strdup(BasePath); // Very messy code
            int TempPathLength = strlen(TempPath);
            realloc(TempPath, (TempPathLength + strlen(path) + 1) * sizeof(char));
            char *TempPath2 = strdup(path);
            if (TempPath2[0] == '.' && TempPath2[1] == '/')
            {
                strcat(TempPath, TempPath2 + 2);
            }
            else
            {
                strcat(TempPath, TempPath2);
            }
            return TempPath;
        }
    }
    return path; // Stops compiler from throwing exception
}

char *EMSCRIPTEN_KEEPALIVE GetBasePath(const char *filename)
{
    int LastPathChar = LastOccurenceOfChar(filename, '/') + 1;
    char *BasePath = (char *)malloc((LastPathChar + 1) * sizeof(char));
    for (unsigned int i = 0; i < LastPathChar; i++)
    {
        BasePath[i] = filename[i];
    }
    BasePath[LastPathChar] = '\0';
    return BasePath;
}

char *ReplaceSectionOfString(char *string, int start, int end, const char *ReplaceString)
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
         }
    ColorCyan();
    printf("String replace debug start\n");
    ColorNormal();
    int ShiftNum = strlen(ReplaceString) - (end - start);
    printf("Shift num: %i, replace string: %s, string: %s, end: %i\n", ShiftNum, ReplaceString, string, end);

    size_t stringlen = (ShiftNum + (int)strlen(string) + 1) * sizeof(char);
    if (ShiftNum > 0)
    {
        const int OldStringLen = strlen(string);
        string = (char *)realloc(string, stringlen);
        for (int i = OldStringLen + 1; i < stringlen; i++)
        {
            string[i] = 'w';
        }
        string[stringlen] = '\0';

        for (unsigned int i = (int)stringlen - ShiftNum - 1; i >= end; i--)
        {                                     // Loops through all characters that need to be shifted to the right
            string[i + ShiftNum] = string[i]; // Shift the character the correct amount to the right
        }
        ColorGreen();
        printf("OldStringLen: %i, NewStringLen: %i\n", OldStringLen, strlen(string));
        ColorNormal();

        int NumbersToShift = OldStringLen - end;
        bool StopShift = false;
        for (unsigned int i = 0; i < ShiftNum; i++)
        {
            if (i < NumbersToShift)
            {
                string[i + ShiftNum + OldStringLen] = string[OldStringLen - ShiftNum + i]; // Shift the character the correct amount to the right
            }
            else
            {
                StopShift = true;
                break;
            }
        }
        printf("test 2 %s", string);
        if (!StopShift)
        {
            for (unsigned int i = end; i < OldStringLen - ShiftNum; i++)
            {
                string[i + ShiftNum] = string[i]; // Shift the character the correct amount to the right
            }
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
    ColorNormal();*/

    if (string == NULL || ReplaceString == NULL)
    {
        return NULL;
    }
    int stringLen = strlen(string);
    int replaceLen = strlen(ReplaceString);
    int shiftNum = replaceLen - (end - start);
    // Reallocate memory for the string if necessary
    char *newString = malloc(sizeof(char) * (stringLen + shiftNum + 1));
    if (shiftNum != 0)
    {
        if (newString == NULL)
        {
            return NULL;
        }
        // Adjust the length of the string if the replacement string is longer
        if (shiftNum > 0)
        {
            stringLen += shiftNum;
            string = realloc(string, stringLen + 1);
            printf("%s\n", string);
            if (string == NULL)
            {
                return NULL;
            }
        }
        // Copy the original string up to the start index
        strncpy(newString, string, start);

        // Copy the replacement string
        strncpy(newString + start, ReplaceString, replaceLen);

        // Copy the rest of the original string after the end index
        strcpy(newString + start + replaceLen, string + end);
        // Free the old string and update the pointer
        free(string);
        newString[stringLen + 1] = '\0';
    }
    else
    {
        // If the replacement string has the same length as the original
        // section, simply copy it into the string
        strncpy(string + start, ReplaceString, end - start);
        return string;
    }

    return newString;
}

char *InsertStringAtPosition(char *OriginalString, char *ReplaceString, int position)
{
    if (OriginalString == NULL || ReplaceString == NULL)
    {
        return NULL;
    }
    unsigned int OriginalLen = strlen(OriginalString);
    unsigned int ReplaceLen = strlen(ReplaceString);
    char *NewString = malloc(OriginalLen + ReplaceLen + 1);
    strncpy(NewString, OriginalString, position);
    strcpy(NewString + position, ReplaceString);
    strcpy(NewString + position + ReplaceLen, OriginalString + position);
    return NewString;
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

    return ReplaceSectionOfString(PathCopy, 0, strlen(Settings.entry), Settings.exit);
}

void StringFormatInsert(char *string, const char *InsertString)
{
    // todo
}

void RemoveSubstring(char *string, const char *substring)
{
    // Check for NULL input
    if (string == NULL || substring == NULL)
    {
        return;
    }
    // Get the lengths of the strings
    int stringLen = strlen(string);
    int substringLen = strlen(substring);
    // Check if the substring is empty
    if (substringLen == 0)
    {
        return;
    }
    // Allocate memory for the new string
    char *newString = malloc(sizeof(char) * (stringLen + 1));
    if (newString == NULL)
    {
        return;
    }
    // Initialize the new string
    newString[0] = '\0';
    // Loop through the original string
    for (int i = 0; i < stringLen; i++)
    {
        // Check if the current substring matches the given substring
        if (strncmp(string + i, substring, substringLen) == 0)
        {
            // If it does, skip the substring and continue
            i += substringLen - 1;
        }
        else
        {
            // If it doesn't, append the current character to the new string
            char currentChar[2] = {string[i], '\0'};
            strcat(newString, currentChar);
        }
    }
    // Copy the new string back into the original string
    strcpy(string, newString);
    // Free the memory allocated for the new string
    free(newString);
}

/* Function to remove a section of a string */
void RemoveSectionOfString(char *str, int start, int end)
{
    int i;
    int str_len = strlen(str);
    printf("Section being removed %s\n", getSubstring(str, start, end));

    /* Shift characters after the end of the section to the left */
    for (i = end; i < str_len; i++)
    {
        str[i - (end - start)] = str[i];
    }

    /* Add a null terminator to the shortened string */
    str[i - (end - start)] = '\0';
}

bool StringEndsWith(char *str, char *substr)
{
    int str_len = strlen(str);
    int substr_len = strlen(substr);

    if (substr_len > str_len)
    {
        return false;
    }

    return (strcmp(str + str_len - substr_len, substr_len) == 0);
}