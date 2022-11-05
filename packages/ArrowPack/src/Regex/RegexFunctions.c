#include "RegexFunctions.h"

int EMSCRIPTEN_KEEPALIVE GetNumOfRegexMatches(const char *Text, const char *Pattern)
{
    // printf("Getting num of regex matches for text %s\n", Text);
    return 0; // Temp for debugging need to remember to remove
    regex_t regexp;
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

    while (error != REG_NOMATCH && match[NumOfStrings].rm_eo != 0)
    {
        error = regexec(&regexp, Text + match[NumOfStrings].rm_eo, 0, match, 0);
        NumOfStrings++;
    }
    regfree(&regexp);

    return NumOfStrings;
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *GetAllRegexMatches(char *Text, const char *Pattern, unsigned int StartPos, unsigned int EndPos)
{
    const int N_MATCHES = 512;
    size_t RegexMatchesSize = sizeof(struct RegexMatch);
    RegexMatch *matches = (RegexMatch *)malloc(sizeof(char *));
    regex_t regexp;

    char *TextStartPointer = Text; //  points to start of string after match

    regmatch_t match[N_MATCHES]; // Contains all matches

    unsigned int matchesCompleted = 0;

    if (regcomp(&regexp, Pattern, 0) != 0) // compiles regex
    {
        fprintf(stderr, "Could not compile regex");
        exit(1); // Exits if an error occured during regex compilation
    };
    while (1)
    {
        int error = regexec(&regexp, TextStartPointer, N_MATCHES, match, 0);
        if (error == 0)
        {
            matches = (RegexMatch *)realloc(matches, (matchesCompleted + 2) * RegexMatchesSize);                                           // Reallocates memory for matches array
            matches[matchesCompleted].Text = (char *)malloc((int)match[0].rm_eo - (int)match[0].rm_so + 1);                                // Allocates memory for match
            matches[matchesCompleted].Text = getSubstring(TextStartPointer, (int)match[0].rm_so + StartPos, (int)match[0].rm_eo - EndPos); // Adds substring to matchs array
            TextStartPointer += (int)match[0].rm_eo;                                                                                       // Uses pointer arithmetic to set textStartPointer to end of match
            matchesCompleted++;                                                                                                            // Increments count of matches
        }
        else
        {
            break;
        }
    }
    regfree(&regexp);

    return matches;
}

bool EMSCRIPTEN_KEEPALIVE HasRegexMatch(const char *text, const char *pattern)
{
    regex_t regexp;

    int Error;

    Error = regcomp(&regexp, pattern, 0);

    if (Error != 0) // Throws an error if regex doesn't compile
    {
        regfree(&regexp);
        printf("Error compiling regex: %s\n", pattern);
        exit(1);
    }

    Error = regexec(&regexp, text, 0, NULL, 0);

    if (!Error)
    {
        regfree(&regexp);
        return true;
    }
    else if (Error == REG_NOMATCH)
    {
        regfree(&regexp);
        return false;
    }
    else
    {
        regfree(&regexp);
        printf("Error when running regex on %s with pattern of %s\n", text, pattern);
        exit(1);
    }
}

void ReplaceStrBetweenIndexes(char *str, char *InsertString, unsigned int start, unsigned int end)
{
}

void EMSCRIPTEN_KEEPALIVE regextest(char *Text, const char *Pattern)
{
}
