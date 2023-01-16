#include "StringRelatedFunctions.h"

static int LASTUNUSEDNAMENUM = 0;

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

char EMSCRIPTEN_KEEPALIVE *TurnToFullRelativePath(const char *PATH, char *BasePath)
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
    char *path = strdup(PATH);
    if (path[0] == '/' || path[0] == '\\')
    {
        tempHolder = malloc(sizeof(char) * (strlen(path) + strlen(Settings.entry)) + 1);
        strcpy(tempHolder, Settings.entry);
        tempHolder[strlen(tempHolder)] = '\0';
        strcat(tempHolder, path + 1);
        return tempHolder;
    }
    else
    {

        int MatchesNum = GetNumOfRegexMatches(path, "\\.\\./"); // This is adding broken char to end of BasePath for some reason
        if (MatchesNum > 0)
        { // Handles paths containing ../
            if (BasePath[0] == '\0' || BasePath == NULL)
            { // BasePath is only needed for paths with ../
                ThrowFatalError("Error no base path specified");
            }
            char *PathCopy = strdup(path);
            char *BasePathCopy = strdup(BasePath);
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
            return BasePathCopy;
        }
        else
        {
            if (strstr(path, Settings.entry) != NULL || BasePath[0] == '\0') // path is already full path (might accidentally include paths with entry name in folder path)path o
            {
                return path;
            }

            char *TempPath = strdup(BasePath); // Very messy code
            int TempPathLength = strlen(TempPath);
            TempPath = realloc(TempPath, (TempPathLength + strlen(path) + 1) * sizeof(char));
            char *TempPath2 = strdup(path);
            if (TempPath2[0] == '.' && TempPath2[1] == '/')
            {
                strcat(TempPath, TempPath2 + 2);
            }
            else
            {
                strcat(TempPath, TempPath2);
            }
            ColorCyan();
            ColorNormal();
            return TempPath;
        }
    }
    return path; // Stops compiler from throwing exception
}

bool IsPreprocessDir(const char *path)
{
    if (strlen(path) < 29)
    {
        return false;
    }
    return strncmp(path, PREPROCESS_DIR, 29) == 0;
}

char *EMSCRIPTEN_KEEPALIVE GetTrueBasePath(const char *filename)
{
    int LastPathChar = LastOccurenceOfChar(filename, '/') + 1;
    if (LastPathChar == 0)
    {
        return "";
    }
    char *BasePath = (char *)malloc((LastPathChar + 1) * sizeof(char) + strlen(Settings.entry));

    memcpy(BasePath, filename, LastPathChar);

    BasePath[LastPathChar] = '\0';
    return BasePath;
}

char *EMSCRIPTEN_KEEPALIVE GetBasePath(const char *filename)
{
    int LastPathChar = LastOccurenceOfChar(filename, '/') + 1;
    if (LastPathChar == 0)
    {
        return "";
    }
    char *BasePath = (char *)malloc((LastPathChar + 1) * sizeof(char) + strlen(Settings.entry));

    memcpy(BasePath, filename, LastPathChar);

    BasePath[LastPathChar] = '\0';
    if (IsPreprocessDir(filename))
    {
        BasePath = ReplaceSectionOfString(BasePath, 0, 29, Settings.entry);
    }
    return BasePath;
}

char *ReplaceSectionOfString(char *string, int start, int end, const char *ReplaceString)
{
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
    return strncasecmp(substring, string, strlen(substring)) == 0;
}

char *EntryToExitPath(const char *path)
{
    char *PathCopy = strdup(path);
    if (strncasecmp(PathCopy, "node_modules/", 13) == 0)
    {
        return ReplaceSectionOfString(PathCopy, 0, 13, Settings.exit);
    }
    if (strncasecmp(PathCopy, PREPROCESS_DIR, 29) == 0)
    {
        return ReplaceSectionOfString(PathCopy, 0, 30, Settings.exit);
    }
    else
    {
        return ReplaceSectionOfString(PathCopy, 0, strlen(Settings.entry), Settings.exit);
    }
}

char *EntryToPreprocessPath(char *path)
{
    return ReplaceSectionOfString(path, 0, strlen(Settings.entry) - 1, PREPROCESS_DIR);
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

    return (strcmp(str + str_len - substr_len, substr) == 0);
}

bool IsURL(char *str)
{
    if (str == NULL)
    {
        return false;
    }
    return HasRegexMatch(str, "((http|https):\\\\/\\\\/)?[\\\\w\\\\-_]+(\\\\.[\\\\w\\\\-_]+)+([\\\\w\\\\-\\\\.,@?^=%&:/~\\\\+#]*[\\\\w\\\\-\\\\@?^=%&/~\\\\+#])?");
}
void RemoveCharFromString(char *str, char c)
{
    int i, j;
    for (i = j = 0; str[i] != '\0'; i++)
    {
        if (str[i] != c)
        {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}
bool StringContainsSubstring(const char *string, const char *substring)
{
    // Check if the string or the substring is NULL
    if (string == NULL || substring == NULL)
    {
        return false;
    }

    // Check if the substring is an empty string
    if (strlen(substring) == 0)
    {
        return false;
    }

    // Check if the string contains the substring using strstr
    return strstr(string, substring) != NULL;
}

char *IntToString(int Integer)
{
    if (Integer == 0)
    {
        return "0";
    }
    int IntStringLength = 1;
    int Temp2 = 1;

    while (Temp2 <= Integer)
    {
        IntStringLength++;
        Temp2 *= 10;
    }
    char *String = malloc(sizeof(char) * (IntStringLength + 1));
    char *ptr = &String[0];
    int count = 0, temp;
    if (Integer < 0)
    {
        Integer *= (-1);
        *ptr++ = '-';
        count++;
    }
    for (temp = Integer; temp > 0; temp /= 10, ptr++)
        ;
    *ptr = '\0';
    for (temp = Integer; temp > 0; temp /= 10)
    {
        *--ptr = temp % 10 + '0';
        count++;
    }
    return String;
}

char *CreateUnusedName()
{
    LASTUNUSEDNAMENUM++;
    char *Num = IntToString(LASTUNUSEDNAMENUM);
    char *Name = malloc(strlen(Num) + 2);
    strcpy(Name, "A");
    strcat(Name, Num);
    return Name;
}

char *AddPreprocessDIR(char *Path)
{
    char *NewString = malloc(strlen(Path) + 31);
    strcpy(NewString, PREPROCESS_DIR);
    NewString[29] = '/';
    NewString[30] = '\0';
    strcat(NewString, Path);
    free(Path);
    return NewString;
}