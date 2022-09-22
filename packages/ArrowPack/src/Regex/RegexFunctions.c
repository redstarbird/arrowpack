#include "RegexFunctions.h"

int EMSCRIPTEN_KEEPALIVE GetNumOfRegexMatches(const char *Text, const char *Pattern)
{
    regex_t regexp;
    printf("%s\n", Pattern);
    if (regcomp(&regexp, Pattern, 0) != 0)
    {
        fprintf(stderr, "Could not compile regex");
        return 0;
    }; // compiles regex

    const int N_MATCHES = 128;

    regmatch_t match[N_MATCHES];

    int error = regexec(&regexp, Text, 0, match, 0);

    int NumOfStrings = 0;
    if (error == 0)
    {
        NumOfStrings++;
    }
    else
    {
        regfree(&regexp);
        return NumOfStrings;
    }

    while (!error)
    {
        error = regexec(&regexp, Text, 0, match, 0);
        if (error)
        {
            break;
        }
        else
        {
            NumOfStrings++;
        }
    }
    regfree(&regexp);
    printf("%i\n", NumOfStrings);
    return NumOfStrings;
}

char EMSCRIPTEN_KEEPALIVE **GetAllRegexMatches(char *Text, const char *Pattern)
{
    regex_t regexp;

    if (regcomp(&regexp, Pattern, 0) != 0)
    {
        fprintf(stderr, "Could not compile regex");
        exit(1);
    }; // compiles regex

    const int N_MATCHES = 128;

    regmatch_t match[N_MATCHES];

    int error = regexec(&regexp, Text, 0, match, 0);
    char **matches = NULL;
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
        matches[0] = getSubstring(Text, (int)match->rm_so, (int)match->rm_eo);
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

            matches = realloc(*matches, NumOfStrings * sizeof(char *)); // Allocate memory (this might be allocating too much memory)
            matches[NumOfStrings - 1] = malloc(currentAmountOfChars * sizeof(char));
            matches[NumOfStrings - 1] = getSubstring(Text, (int)match->rm_so, (int)match->rm_eo);
        }
    }
    return matches;
}