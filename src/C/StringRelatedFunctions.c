/* This file contains all of the functions used for strings */
#include "StringRelatedFunctions.h"

// Returns the index of the last occurence of a character or -1 if the character isn't present
int EMSCRIPTEN_KEEPALIVE
LastOccurenceOfChar(const char *text, char character)
{
    int LastOccurence = -1; // Default value if the character isn't present

    for (unsigned int i = 0; i < strlen(text); i++)
    {
        if (text[i] == character)
        {
            LastOccurence = i; // Set LastOcccurence to the occurence index
        }
    }
    return LastOccurence;
}

// Returns the number of times a character is found in a string
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

// Returns whether a string contains a given character
bool EMSCRIPTEN_KEEPALIVE containsCharacter(const char *string, char character)
{
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == character)
            return true; // Returns true if the character is found
    }
    return false;
}

// Returns the file extension for the given path without the '.' char
char *GetFileExtension(const char *PATH)
{
    unsigned int pathLen = strlen(PATH);
    int lastPathSeperator = LastOccurenceOfChar(PATH, '/'); // Finds the location of the final path seperator
    if (lastPathSeperator == -1)
    {
        lastPathSeperator = LastOccurenceOfChar(PATH, '\\'); // Also detect path seperators on windows
    }

    char *path = (char *)PATH + (lastPathSeperator + 1); // Store the path after the '/'
    int lastFullStop = LastOccurenceOfChar(path, '.');   // Finds the location of the final '.'
    if (lastFullStop == -1)
    {
        return strdup(path); // If no '.' character is found, return the path itself for file such as "LICENSE"
    }
    lastFullStop++;             // Stops fullstop character being included
    const int length = pathLen; // gets length of file extension
    char *extension = malloc(length + 1);
    strncpy(extension, path + lastFullStop, length); // Copies file extension to new string
    extension[length] = '\0';
    return extension;
}

// Returns substring between start and end indexes
char EMSCRIPTEN_KEEPALIVE *getSubstring(char *Text, int StartIndex, int EndIndex)
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

// Converts a string to a bool
bool StringToBool(const char *str)
{
    if (str[0] == '1')
        return true;                     // Count "1" as true
    return strcasecmp(str, "true") == 0; // Return whether the string is "true"
}

// Splits a string by a specified character into an array of strings
char **SplitStringByChar(char *str, const char delimiter)
{
    unsigned int NumOfTokens = 0;
    char **Result = malloc(sizeof(char *));
    char *token = strtok(str, &delimiter);

    while (token != NULL) // Loop through all occurences of character
    {
        NumOfTokens++;
        Result = (char **)malloc(sizeof(char *) * NumOfTokens + 1);
        Result[NumOfTokens - 1] = (char *)malloc(sizeof(char) * strlen(token));

        strcpy(Result[NumOfTokens - 1], token);
        token = strtok(NULL, &delimiter);
    }
    return Result;
}

// Turns a relative path into a full path relative to the CWD
char EMSCRIPTEN_KEEPALIVE *TurnToFullRelativePath(const char *PATH, char *BasePath)
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
    if (path[0] == '/') // Path starts from entry point of application
    {
        NewPath = malloc(sizeof(char) * (strlen(path) + strlen(GetSetting("entry")->valuestring)) + 1); // Allocate new entry path string
        strcpy(NewPath, GetSetting("entry")->valuestring);                                              // Put entry point infront of path
        strcat(NewPath, path + 1);                                                                      // Add original path to new string
        return NewPath;
    }
    else
    {

        int MatchesNum = GetNumOfRegexMatches(path, "\\.\\./"); // Finds how many directories up the path is
        if (MatchesNum > 0)                                     // Handles paths containing ../
        {
            if (BasePath[0] == '\0' || BasePath == NULL) // BasePath is needed for paths with ../
            {
                ThrowFatalError("Error no base path specified for path: %s\n", path);
            }

            char *NewPath = strdup(BasePath); // Create a copy of the BasePath variable so it doesn't get overwritten by strtok()

            int PathSeperatorsFound = 0;
            int LocationFound = -1;
            int BasePathLength = strlen(BasePath);
            if (NewPath[BasePathLength - 1] == '/') // Remove '/' from end of string
            {
                NewPath[BasePathLength - 1] = '\0';
                BasePathLength--;
            }

            for (int i = BasePathLength; i > 0; i--) // Search backwards through to find the index of how many directories to cut off of the base path
            {
                if (NewPath[i] == '/')
                {
                    PathSeperatorsFound++;
                    if (PathSeperatorsFound >= MatchesNum) // Check if enough directories have been found in the path
                    {
                        LocationFound = i; // The index to cut off from
                        break;
                    }
                }
            }

            path = RemoveSubstring(path, "../"); // Removes ../ characters from final path
            NewPath = realloc(NewPath, strlen(NewPath) + strlen(path) + 3);

            NewPath[LocationFound + 1] = '\0'; // Cuts off base path at index
            NewPath[LocationFound] = '/';
            strcat(NewPath, path); // adds path to basepath
            return NewPath;
        }
        else // Path is relative to basepath
        {
            if (strstr(path, GetSetting("entry")->valuestring) != NULL || BasePath[0] == '\0') // path is already full path (might accidentally include paths with entry name in folder path)
            {
                return path;
            }
            if (StringStartsWith(path, PREPROCESS_DIR)) // Path is in preprocess temp directory
            {
                return path;
            }
            else
            {
                NewPath = malloc((strlen(BasePath) + strlen(path) + 1) * sizeof(char)); // Allocate new path with length of basepath, path and space for a null terminator
                strcpy(NewPath, BasePath);                                              // Copy basepath to the start of the new path
                char *TempPath2 = strdup(path);

                strcat(NewPath, TempPath2 + 2 * (TempPath2[0] == '.' && TempPath2[1] == '/')); // Add path to new path and remove "./" if present
            }
            return NewPath;
        }
    }
    return path; // Stops compiler from complaining
}

// Returns whether a given filepath is in the preprocess directory
bool IsPreprocessDir(const char *path)
{
    if (strlen(path) < 29) // Check if filepath is shorter than preprocess directory
    {
        return false;
    }
    return strncmp(path, PREPROCESS_DIR, 29) == 0; // Use strncmp to check for preprocess path at the start of the string
}

// Gives the true base path without adjusting for preprocess directory, mainly useful for file operations
char *EMSCRIPTEN_KEEPALIVE GetTrueBasePath(const char *filename)
{
    int LastPathChar = LastOccurenceOfChar(filename, '/') + 1; // Find the last path seperator
    if (LastPathChar == -1)                                    // Check if file path contains '/'
    {
        return ""; // Return empty string if there is no base path
    }
    char *BasePath = (char *)malloc(sizeof(char) * ((LastPathChar + 1) + strlen(GetSetting("entry")->valuestring)));

    memcpy(BasePath, filename, LastPathChar); // Copy filepath up to the last past seperator to get the base path

    BasePath[LastPathChar] = '\0'; // Null terminate the new string
    return BasePath;
}

// Returns the base path/directory for a given file path
char *EMSCRIPTEN_KEEPALIVE GetBasePath(const char *filename)
{
    int LastPathChar = LastOccurenceOfChar(filename, '/') + 1; // Find the last path seperator
    if (LastPathChar == -1)                                    // Check if file path contains '/'
    {
        return ""; // Return empty string if there is no base path
    }
    char *BasePath = (char *)malloc(sizeof(char) * ((LastPathChar + 1) + strlen(GetSetting("entry")->valuestring)));

    memcpy(BasePath, filename, LastPathChar); // Copy filepath up to the last past seperator to get the base path

    BasePath[LastPathChar] = '\0'; // Null terminate the new string
    if (IsPreprocessDir(filename)) // Turn preprocess base path into entry base path
    {
        BasePath = ReplaceSectionOfString(BasePath, 0, 30, GetSetting("entry")->valuestring);
    }

    return BasePath;
}

// Replaces a section of a string with a substring regardless of section and substring length
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
        }
        //  Copy the original string up to the start index
        newString = strncpy(newString, string, start);
        newString[start] = '\0';
        //  Copy the replacement string
        strncpy(newString + start, ReplaceString, replaceLen);
        //  Copy the rest of the original string after the end index
        strcpy(newString + start + replaceLen, string + end);
        // Free the old string and update the pointer
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

// Inserts a string at a position into another string
char *InsertStringAtPosition(char *OriginalString, char *ReplaceString, int position)
{
    if (OriginalString == NULL || ReplaceString == NULL) // Check strings aren't NULL
    {
        return NULL;
    }
    unsigned int OriginalLen = strlen(OriginalString);
    unsigned int ReplaceLen = strlen(ReplaceString);
    char *NewString = malloc(OriginalLen + ReplaceLen + 1);               // Allocate a new string with the length of the origininal string, new string a null terminator
    strncpy(NewString, OriginalString, position);                         // Copy the start of the original string, up to the insert index, into the new string
    strcpy(NewString + position, ReplaceString);                          // Copy the insert string into the new string at the insert index
    strcpy(NewString + position + ReplaceLen, OriginalString + position); // Copy the rest of the original string into the new string
    return NewString;
}

// Returns whether a string starts with a given substring
bool EMSCRIPTEN_KEEPALIVE StringStartsWith(const char *string, const char *substring)
{
    if (string[0] == '\0' || substring[0] == '\0') // Check both strings aren't empty
    {
        return false;
    }
    return strncasecmp(substring, string, strlen(substring)) == 0; // Compare the start of the string with strncasecmp
}

// Takes an entry path and returns the exit path equivalent
char *EntryToExitPath(const char *path)
{
    char *PathCopy = strdup(path);
    if (strncasecmp(PathCopy, "node_modules/", 13) == 0)
    {
        return ReplaceSectionOfString(PathCopy, 0, 13, GetSetting("exit")->valuestring); // Returns the exit path if the entry path if in node_modules
    }
    if (strncasecmp(PathCopy, PREPROCESS_DIR, 29) == 0)
    {
        return ReplaceSectionOfString(PathCopy, 0, 30, GetSetting("exit")->valuestring); // Returns the exit path if the entry path is in the preprocess directory
    }
    else
    {
        return ReplaceSectionOfString(PathCopy, 0, strlen(GetSetting("entry")->valuestring), GetSetting("exit")->valuestring); // Replaces the entry path in the string with the exit path
    }
}

// Turns an entry path into the corresponding preprocess path
char *EntryToPreprocessPath(char *path)
{
    return ReplaceSectionOfString(path, 0, strlen(GetSetting("entry")->valuestring) - 1, PREPROCESS_DIR); // Replaces entry string with preprocess
}

/* Removes all occurences of a given subtring from a string */
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

/* Returns whether a given string ends with a given substring */
bool StringEndsWith(char *str, char *substr)
{
    int str_len = strlen(str);
    int substr_len = strlen(substr);

    if (substr_len > str_len) // If the substring length is more than the string length then the string can't end with it
    {
        return false;
    }

    return (strcmp(str + str_len - substr_len, substr) == 0);
}

/* Returns whether a given string is a URL */
bool IsURL(char *str)
{
    if (str == NULL)
    {
        return false;
    }
    return HasRegexMatch(str, "((http|https):\\\\/\\\\/)?[\\\\w\\\\-_]+(\\\\.[\\\\w\\\\-_]+)+([\\\\w\\\\-\\\\.,@?^=%&:/~\\\\+#]*[\\\\w\\\\-\\\\@?^=%&/~\\\\+#])?");
}

/* Removes all occurences of a character from a string */
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

/* Returns whether a given string contains a given substring */
bool StringContainsSubstring(const char *string, const char *substring)
{
    // Check if the string or the substring is NULL
    if (string == NULL || substring == NULL)
    {
        return false;
    }

    // Check if the substring is an empty string
    if (strlen(substring) == 0 || strlen(string) == 0)
    {
        return false;
    }

    // Check if the string contains the substring using strstr
    return strstr(string, substring) != NULL;
}

/* Turns an integer to a string */
char *IntToString(int Integer)
{
    if (Integer == 0)
    {
        return "0"; // 0 Doesn't work with this algorithm so it is hardcoded in here
    }
    int IntStringLength = 1;
    int Temp2 = 1;

    while (Temp2 <= Integer) // Find the length of the string
    {
        IntStringLength++;
        Temp2 *= 10;
    }
    printf("String length: %d\n", IntStringLength);
    char *String = malloc(sizeof(char) * (IntStringLength + 1)); // Allocates the string using the length calculated earlier
    char *ptr = &String[0];
    int count = 0, temp;
    if (Integer < 0) // Easy way to make negative numbers work
    {
        Integer *= (-1); // Make number positive
        *ptr++ = '-';    // Add negative sign to start of string
        count++;
    }
    for (temp = Integer; temp > 0; temp /= 10, ptr++)
        ;
    *ptr = '\0';
    for (temp = Integer; temp > 0; temp /= 10) // Divide integer by 10 each loop
    {
        *--ptr = temp % 10 + '0'; // Add the character code of the number to the string
        count++;
    }
    printf("String %s\n", String);
    return String;
}

static int LASTUNUSEDNAMENUM = 0; // Internal variable used to create unique names
/* Creates a unique unused name for preventing name collisions */
char *CreateUnusedName()
{
    LASTUNUSEDNAMENUM++;                        // Increment  LASTUNUSEDNAMENUM
    char *Num = IntToString(LASTUNUSEDNAMENUM); // Turn int to string
    char *Name = malloc(strlen(Num) + 2);       // Allocate string
    strcpy(Name, "A");                          // Start new string with A
    strcat(Name, Num);                          // Add unique number to new string
    return Name;
}

/* Returns a given filepath as a preprocess path equivalent */
char *AddPreprocessDIR(char *Path)
{
    char *NewString = malloc(strlen(Path) + 31); // Allocate new string with extra length for preprocess path
    strcpy(NewString, PREPROCESS_DIR);
    NewString[29] = '/'; // Add trailing slash after preprocess dir
    NewString[30] = '\0';
    strcat(NewString, Path); // Add original path to new string
    free(Path);
    return NewString;
}

/**

ArrowDeserialize is a function that deserializes a string that has been serialized using the ArrowSerialize function from Javascript

@param serialized - the serialized string

@param n_strings - a pointer to an integer that will be set to the number of strings found in the serialized string

@return char ** - a pointer to an array of strings
*/
char **ArrowDeserialize(const char *serialized, int *n_strings)
{
    // Define custom separator and escape characters
    const char separator = '\x1f';
    const char escape = '\x1e';

    // If the serialized string only contains one string, return it as an array
    if (!containsCharacter(serialized, separator))
    {
        char **returnString = malloc(sizeof(char *));
        returnString[0] = strdup(serialized);
        *n_strings = 1;
        return returnString;
    };

    int count = 0;
    char **strings = NULL;
    char *str = NULL;
    int str_len = 0;

    // Loop through the serialized string to find the total number of strings
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

    // If there is a string that has not been added to the array, add it
    if (str != NULL)
    {
        count++;
        strings = realloc(strings, count * sizeof(char *));
        strings[count - 1] = str;
    }

    // Set the number of strings and return the array of strings
    *n_strings = count;
    return strings;
}

// Serialise an array of strings into a single string to be sent to JS side
char *ArrowSerialize(const char *strings[], size_t count)
{
    size_t i, j, k, len;
    char *buffer;
    const char escape_char = '\x1f'; // The character used to seperate strings

    // Compute the total length of the serialized string
    len = 0;
    for (i = 0; i < count; i++) // Loops through each given string
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
    for (i = 0; i < count; i++) // Loops through the length of the string
    {
        for (j = 0; j < strlen(strings[i]); j++) // Loops through each given string
        {
            if (strings[i][j] == escape_char)
            {
                buffer[k++] = escape_char;
            }
            buffer[k++] = strings[i][j]; // Copies the contents of the string into the serialised string
        }
        if (i < count - 1)
        {
            buffer[k++] = escape_char; // Adds escape char if at the end of the current string
        }
    }
    buffer[k] = '\0'; // Null terminate the final string`

    return buffer;
}

/* Turns a string to an integer e.g, "543" to int 543 */
int StringToInt(const char *string)
{
    return atoi(string); // Abstracting the atoi function into a more understandable function name
}

// Internal function used by match glob to expand braces in a glob pattern into a string with | delimeters
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

// Returns whether a file path string matches a given glob pattern
bool MatchGlob(const char *STRING, const char *PATTERN)
{
    char *string = strdup(STRING);
    char *pattern = strdup(PATTERN);

    // Determine the maximum length of the expanded pattern
    int max_pattern_len = strlen(pattern) + 1;

    for (int i = 0; i < strlen(pattern); i++)
    {
        if (pattern[i] == '{') // Find any brace expansions
        {

            char *endbrace = strchr(pattern + i, '}');

            if (endbrace == NULL)
            {
                ThrowFatalError("Invalid brace expansion: %s\n", pattern);
            }

            max_pattern_len += (endbrace - pattern) * 10;
        }
    }

    // Expand any braces in the pattern because fnmatch doesn't support brace expansion syntax
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

    return false; // Return false because a match has not been found
}