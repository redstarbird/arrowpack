/*

    This file is WIP
    Will eventually be a general lightweight JSON parser

*/

#include "JSONParser.h"
#include "../DependencyGraph/DependencyGraph.h"
#include "FileHandler.h"
#include <stdbool.h>

char *ParseJSON(const char *json)
{
}

char *removeWhitespace(char *str)
{
    int index = 0;
    int newStringIndex = 0;

    size_t len = strlen(str);
    bool inParanthesis = false;
    char paranthesisUsed;
    char newJSON[len];

    while (str[index] != '\0')
    {
        switch (str[index])
        {
        case '\"' ... '\'':
            if (!inParanthesis)
            {
                inParanthesis = true;
                paranthesisUsed = str[index];
            }
            else if (str[index] == paranthesisUsed)
            {
                inParanthesis = false;
            }
            newJSON[newStringIndex] = str[index];
            newStringIndex++;
            index++;
            break;

        case '\n':
            index++;
            break;

        case ' ':
            if (inParanthesis)
            {
                newJSON[newStringIndex] = str[index];
                newStringIndex++;
            }
            index++;
            break;

        default:
            newJSON[newStringIndex] = str[index];
            newStringIndex++;
            index++;
            break;
        }
        newJSON[len] = '\0';
    }
    return *newJSON;
}

struct FileRule getFileRuleFromSubstring(char *string)
{
}

struct FileRule *ParseJSONToStructArray(char *RawJSON)
{ // gets array of file rules from json
    // char *JSON = json + 1; // removes first character with pointer arithmetic
    // JSON[strlen(JSON)-1] = '\0'; // removes last character

    char *json = removeWhitespace(RawJSON);

    struct FileRule *StructArray = malloc(2 * sizeof(int));

    unsigned int startOfCurrentFileRule = 0;
    unsigned int index = 0;
    unsigned int AmountOfFileRules = 0;

    char *substring;

    int squareBracketDepth = 0;

    while (1)
    {
        if (json[index] == '\0')
        {
            break;
        } // stop at end of file
        else if (json[index] == '[')
        {
            squareBracketDepth++;
            if (squareBracketDepth == 2)
            { // detects if start of new file rule
                startOfCurrentFileRule = index;
            }

            StructArray = realloc(StructArray, AmountOfFileRules * sizeof(struct FileRule *)); // might be allocating too much memory :(
        }
        else if (json[index] == ',')
        {
            substring = malloc((index - startOfCurrentFileRule + 1) * sizeof(char)); // allocate memory for substring
            strncpy(*substring, json + startOfCurrentFileRule, index - 1);           // gets substring by using pointer arithmetic for start pos and current index for end pos
            substring[index - startOfCurrentFileRule] = '\0';                        // end substring
            getFileRuleFromSubstring(substring);
        }
        index++;
    }
}
