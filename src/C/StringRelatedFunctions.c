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

int NumOfCharOccurences(const char *text, const char character)
{
    int Occurences = 0;
    for (unsigned int i = 0; i < strlen(text); i++)
    {
        if (text[i] == character)
        {
            Occurences++;
        }
    }
    return Occurences;
}

bool EMSCRIPTEN_KEEPALIVE containsCharacter(const char *string, char character) // Checks if string contains a certain character
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
        return strdup(path);
    }
    lastFullStop++;             // Stops fullstop character being included
    const int length = pathLen; // gets length of file ext
    char *extension = malloc(length + 1);
    strncpy(extension, path + lastFullStop, length);
    extension[length] = '\0';

    /*for (int i = 0; i < length; i++)
    {
        extension[i] = path[lastFullStop + i]; // string doesn't include fullstop
    }
    extension[length] = '\0';*/
    return extension;
}

char EMSCRIPTEN_KEEPALIVE *getSubstring(char *Text, int StartIndex, int EndIndex) // Returns substring between start and end indexes
{
    const int substringLength = EndIndex - StartIndex + 1;          // Gets the length of the substring
    char *substring = malloc(sizeof(char) * (substringLength + 2)); // Allocates memory for substring
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

char EMSCRIPTEN_KEEPALIVE *TurnToFullRelativePath(const char *PATH, char *BasePath) // Turns a relative path into full path
{
    char *NewPath; // Buffer to hold the absolute path
    char *path = strdup(PATH);

    if (containsCharacter(path, '\\')) // Turn \ in path on windows into / for compatibility with other functions
    {
        for (int i = 0; i < strlen(path); i++)
        {
            if (path[i] == '\\')
            {
                path[i] = '/';
            }
        }
    }
    if (path[0] == '/')
    {
        NewPath = malloc(sizeof(char) * (strlen(path) + strlen(GetSetting("entry")->valuestring)) + 1);
        strcpy(NewPath, GetSetting("entry")->valuestring);
        strcat(NewPath, path + 1);
        return NewPath;
    }
    else
    {

        int MatchesNum = GetNumOfRegexMatches(path, "\\.\\./");
        if (MatchesNum > 0) // Handles paths containing ../
        {
            if (BasePath[0] == '\0' || BasePath == NULL) // BasePath is only needed for paths with ../
            {
                ThrowFatalError("Error no base path specified for path: %s\n", path);
            }

            char *NewPath = strdup(BasePath); // Create a copy of the BasePath variable so it doesn't get overwritten by strtok()

            int PathSeperatorsFound = 0;
            int LocationFound = -1;
            int BasePathLength = strlen(BasePath);
            if (NewPath[BasePathLength - 1] == '/')
            {
                NewPath[BasePathLength - 1] = '\0';
                BasePathLength--;
            }
            for (int i = BasePathLength; i > 0; i--)
            {
                if (NewPath[i] == '/')
                {
                    PathSeperatorsFound++;
                    if (PathSeperatorsFound == MatchesNum)
                    {
                        LocationFound = i;
                        break;
                    }
                }
            }

            path = RemoveSubstring(path, "../");
            NewPath = realloc(NewPath, strlen(NewPath) + strlen(path) + 3);

            NewPath[LocationFound + 1] = '\0';
            NewPath[LocationFound] = '/';
            strcat(NewPath, path);
            return NewPath;
        }
        else
        {
            if (strstr(path, GetSetting("entry")->valuestring) != NULL || BasePath[0] == '\0') // path is already full path (might accidentally include paths with entry name in folder path)path o
            {
                return path;
            }
            if (StringStartsWith(path, PREPROCESS_DIR))
            {
                return path;
            }
            else
            {
                NewPath = malloc((strlen(BasePath) + strlen(path) + 1) * sizeof(char));
                strcpy(NewPath, BasePath);
                char *TempPath2 = strdup(path);
                if (TempPath2[0] == '.' && TempPath2[1] == '/')
                {
                    strcat(NewPath, TempPath2 + 2);
                }
                else
                {
                    strcat(NewPath, TempPath2);
                }
            }
            return NewPath;
        }
    }
    return path; // Stops compiler from complaining
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
    char *BasePath = (char *)malloc((LastPathChar + 1) * sizeof(char) + strlen(GetSetting("entry")->valuestring));

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
    char *BasePath = (char *)malloc((LastPathChar + 1) * sizeof(char) + strlen(GetSetting("entry")->valuestring));

    memcpy(BasePath, filename, LastPathChar);

    BasePath[LastPathChar] = '\0';
    if (IsPreprocessDir(filename))
    {
        BasePath = ReplaceSectionOfString(BasePath, 0, 30, GetSetting("entry")->valuestring);
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
    char *newString = malloc(sizeof(char) * (stringLen + shiftNum + 2));
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
            /*string = realloc(string, stringLen + 2);
            if (string == NULL)
            // {
                return NULL;
        }*/
        }
        //  Copy the original string up to the start index
        newString = strncpy(newString, string, start);
        newString[start] = '\0';
        //  Copy the replacement string
        strncpy(newString + start, ReplaceString, replaceLen);
        //  Copy the rest of the original string after the end index
        strcpy(newString + start + replaceLen, string + end);
        // Free the old string and update the pointer

        // newString[stringLen] = '\0';
    }
    else
    {
        // If the replacement string has the same length as the original
        // section, simply copy it into the string
        strncpy(string + start, ReplaceString, end - start);
        return strdup(string);
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
        return ReplaceSectionOfString(PathCopy, 0, 13, GetSetting("exit")->valuestring);
    }
    if (strncasecmp(PathCopy, PREPROCESS_DIR, 29) == 0)
    {
        return ReplaceSectionOfString(PathCopy, 0, 30, GetSetting("exit")->valuestring);
    }
    else
    {
        return ReplaceSectionOfString(PathCopy, 0, strlen(GetSetting("entry")->valuestring), GetSetting("exit")->valuestring);
    }
}

char *EntryToPreprocessPath(char *path)
{
    return ReplaceSectionOfString(path, 0, strlen(GetSetting("entry")->valuestring) - 1, PREPROCESS_DIR);
}
void StringFormatInsert(char *string, const char *InsertString)
{
    // todo
}

char *RemoveSubstring(char *string, const char *substring)
{
    // Check for NULL input
    if (string == NULL || substring == NULL)
    {
        return string;
    }
    // Get the lengths of the strings
    int stringLen = strlen(string);
    int substringLen = strlen(substring);
    // Check if the substring is empty
    if (substringLen == 0)
    {
        return string;
    }
    // Allocate memory for the new string
    char *newString = malloc(sizeof(char) * (stringLen + 1));
    if (newString == NULL)
    {
        return string;
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

    // Free the memory allocated for the new string
    free(string);
    return newString;
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

char **ArrowDeserialize(const char *serialized, int *n_strings)
{

    const char separator = '\x1f'; // custom separator character
    const char escape = '\x1e';    // custom escape character
    if (!containsCharacter(serialized, separator))
    {
        char **returnString = malloc(sizeof(char *));
        returnString[0] = strdup(serialized);
        return returnString;
    };
    int count = 0;
    char **strings = NULL;
    char *str = NULL;
    int str_len = 0;
    for (int i = 0; i < strlen(serialized); i++)
    {
        char c = serialized[i];
        if (c == separator)
        {
            count++;
            strings = realloc(strings, count * sizeof(char *));
            strings[count - 1] = str;
            str = NULL;
            str_len = 0;
        }
        else if (c == escape)
        {
            char next_c = serialized[i + 1];
            if (next_c == separator || next_c == escape)
            {
                str = realloc(str, str_len + 1);
                str[str_len] = next_c;
                str_len++;
                i++;
            }
            else
            {
                str = realloc(str, str_len + 2);
                str[str_len] = c;
                str[str_len + 1] = next_c;
                str_len += 2;
                i++;
            }
        }
        else
        {
            str = realloc(str, str_len + 1);
            str[str_len] = c;
            str_len++;
        }
    }
    if (str != NULL)
    {
        count++;
        strings = realloc(strings, count * sizeof(char *));
        strings[count - 1] = str;
    }
    *n_strings = count;
    return strings;
}
char *ArrowSerialize(const char *strings[], size_t count)
{
    size_t i, j, k, len;
    char *buffer;
    const char escape_char = '\x1f';

    // Compute the total length of the serialized string
    len = 0;
    for (i = 0; i < count; i++)
    {
        len += strlen(strings[i]);
        for (j = 0; j < strlen(strings[i]); j++)
        {
            if (strings[i][j] == '\n')
            {
                len++;
            }
        }
    }

    // Allocate a buffer to hold the serialized string
    buffer = (char *)malloc(len + count + 1);
    if (buffer == NULL)
    {
        return NULL;
    }

    // Serialize each string into the buffer
    k = 0;
    for (i = 0; i < count; i++)
    {
        for (j = 0; j < strlen(strings[i]); j++)
        {
            if (strings[i][j] == escape_char)
            {
                buffer[k++] = escape_char;
            }
            buffer[k++] = strings[i][j];
        }
        if (i < count - 1)
        {
            buffer[k++] = escape_char;
        }
    }
    buffer[k] = '\0';

    return buffer;
}

int StringToInt(const char *string)
{
    return atoi(string);
}

static void expandBraces(char *pattern, char *buffer, size_t buffer_size)
{
    // Find the position of the first brace in the pattern
    char *brace = strchr(pattern, '{');

    if (brace == NULL)
    {
        // No brace expansion needed
        strncpy(buffer, pattern, buffer_size);
        return;
    }

    // Copy the part of the pattern before the brace into a buffer
    char *prefix = strndup(pattern, brace - pattern);

    // Find the position of the closing brace
    char *endbrace = strchr(brace, '}');

    if (endbrace == NULL)
    {
        ThrowFatalError("Invalid brace expression\n");
    }

    // Copy the part of the pattern after the closing brace into another buffer
    char *suffix = strdup(endbrace + 1);

    // Calculate the maximum length of the expanded pattern
    size_t max_pattern_len = 0;
    char *value = strtok(brace + 1, ",");
    while (value != NULL)
    {
        unsigned int ValueLength = strlen(value);
        if (value[ValueLength - 1] == '}')
        {
            value[ValueLength - 1] = '\0';
        }
        // Construct the new pattern by substituting the value inside the braces
        char *new_pattern = NULL;
        if (asprintf(&new_pattern, "%s%s%s|", prefix, value, suffix) == -1)
        {
            ThrowFatalError("Memory allocation failed for expanding braces\n");
        }

        // Recursively expand any remaining braces
        char *expanded_pattern = (char *)malloc(buffer_size * sizeof(char));
        expandBraces(new_pattern, expanded_pattern, buffer_size);

        // Add the expanded pattern to the buffer
        size_t len = strlen(expanded_pattern);
        if (len > 0)
        {
            if (max_pattern_len > 0)
            {
                max_pattern_len++;
            }
            max_pattern_len += len;
        }

        strcat(buffer, expanded_pattern);

        free(new_pattern);
        free(expanded_pattern);

        // Move to the next value inside the braces
        value = strtok(NULL, ",");
    }

    free(prefix);
    free(suffix);

    if (max_pattern_len > buffer_size)
    {
        ThrowFatalError("Expanded glob pattern is too long\n");
    }
}

bool MatchGlob(const char *STRING, const char *PATTERN)
{
    char *string = strdup(STRING);
    char *pattern = strdup(PATTERN);
    /*
        int status;

        if (containsCharacter(GlobPattern, '{'))
        {
            unsigned int BraceExpansions = NumOfCharOccurences(GlobPattern, '{');
            unsigned int ExpansionStringsNum[BraceExpansions];
            char **ExpansionString = malloc(BraceExpansions * sizeof(char *));
            bool InExpansion = false;
            unsigned int CurrentExpansion = 0;
            size_t CurrentExpansionSize = 2;
            unsigned int CurrentSubExpansion = 0;
            unsigned int LastSubExpansionStart = 0;
            for (int i = 0; i < strlen(GlobPattern); i++)
            {
                if (InExpansion)
                {
                    bool FoundEnd = GlobPattern[i] == '}';
                    if (GlobPattern[i] == ',' || FoundEnd)
                    {

                        if (CurrentSubExpansion + 2 >= CurrentExpansionSize)
                        {
                            if (FoundEnd)
                            {
                                CurrentExpansionSize++;
                                InExpansion = false;
                            }
                            else
                            {
                                CurrentExpansionSize *= 2;
                            }
                            ExpansionString[CurrentExpansion] = realloc(ExpansionString[CurrentExpansion], CurrentExpansionSize * sizeof(char));
                            ExpansionString[CurrentExpansion][CurrentSubExpansion] = *getSubstring((char *)GlobPattern, LastSubExpansionStart, i - 1);
                            printf("Expansion thing thing thing: %s\n", ExpansionString[CurrentExpansion][CurrentSubExpansion]);
                        }
                    }
                }
                else
                {
                    if (GlobPattern[i] == '{')
                    {
                        InExpansion = true;
                        CurrentSubExpansion = 0;
                        CurrentExpansionSize = 2;
                        CurrentExpansion++;
                    }
                }
            }

            exit(0);
        }
        else
        {
            status = fnmatch(GlobPattern, FilePath, 0);
            if (status == 0) // The filename matches the glob pattern
            {

                return true;
            }
            else if (status == FNM_NOMATCH) // The filename does not match the glob pattern
            {

                return false;
            }
            else // An error occurred
            {

                ThrowFatalError("Error matching glob!\n");
            }
        }

        return false;*/

    // Determine the maximum length of the expanded pattern
    int max_pattern_len = strlen(pattern) + 1;

    for (int i = 0; i < strlen(pattern); i++)
    {
        if (pattern[i] == '{')
        {

            char *endbrace = strchr(pattern + i, '}');

            if (endbrace == NULL)
            {
                ThrowFatalError("Invalid brace expansion: %s\n", pattern);
            }

            max_pattern_len += (endbrace - pattern) * 10;
        }
    }

    // Expand any braces in the pattern
    char *expanded_pattern = (char *)malloc(max_pattern_len * sizeof(char));
    memset(expanded_pattern, 0, max_pattern_len);
    expandBraces(pattern, expanded_pattern, max_pattern_len);

    // Loop over each expanded pattern
    char *pat = strtok(expanded_pattern, "|");

    while (pat != NULL)
    {
        // Check if the string matches the current pattern
        if (fnmatch(pat, string, 0) == 0)
        {
            return true;
        }

        // Move to the next expanded pattern
        pat = strtok(NULL, "|");
    }

    free(expanded_pattern);

    return false;
}