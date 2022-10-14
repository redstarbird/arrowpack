#include "../C/ReadFile.h"
#include "../Regex/RegexFunctions.h"
#include "../C/StringRelatedFunctions.h"
#include "FindDependencies.h"

char EMSCRIPTEN_KEEPALIVE **BasicRegexDependencies(char *filename, const char *pattern, unsigned int Startpos, unsigned int Endpos)
{ // Allows any function that only needs basic regex to easily be run
    char *FileContents = ReadDataFromFile(filename);
    if (FileContents == NULL)
    {
        return NULL;
    }

    printf("Num of matches: %d\n", GetNumOfRegexMatches(FileContents, pattern));
    char **RegexMatches = GetAllRegexMatches(FileContents, pattern, Startpos, Endpos);

    if (RegexMatches == NULL)
    {
        return NULL;
    }
    free(FileContents);
    return RegexMatches;
}

char EMSCRIPTEN_KEEPALIVE **FindHTMLDependencies(char *filename)
{

    return BasicRegexDependencies(filename, "<include src=\"[^<]*\">", 14, 2); // Abstraction
}

char EMSCRIPTEN_KEEPALIVE **FindCSSDependencies(char *filename)
{
    char **Dependencies = BasicRegexDependencies(filename, "@import .*;", 9, 2);
    for (int i = 0; i < sizeof(Dependencies) / sizeof(char *); i++)
    {
        if (HasRegexMatch(Dependencies[i], "rl\\(*.\"")) // Removes "url()" CSS function from string
        {
            strcpy(Dependencies[i], getSubstring(Dependencies[i], 4, 1));
        }
    }
    return Dependencies;
}