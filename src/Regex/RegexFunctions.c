#include "RegexFunctions.h"

int EMSCRIPTEN_KEEPALIVE GetNumOfRegexMatches(char *Text, const char *Pattern)
{
    // printf("Getting num of regex matches for text %s\n", Text);
    const int N_MATCHES = 512;
    regex_t regexp;

    char *TextCopy = strdup(Text);

    char *TextStartPointer = &TextCopy[0]; //  points to start of string after match

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
            TextStartPointer += (int)match[0].rm_eo; // Uses pointer arithmetic to set textStartPointer to end of match
            matchesCompleted++;                      // Increments count of matches
        }
        else
        {
            break;
        }
    }
    regfree(&regexp);
    return matchesCompleted;
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *GetAllRegexMatches(char *Text, const char *Pattern, unsigned int StartPos, unsigned int EndPos)
{
    const int N_MATCHES = 512;
    size_t RegexMatchesSize = sizeof(struct RegexMatch);
    struct RegexMatch *matches = (struct RegexMatch *)malloc(RegexMatchesSize);
    regex_t regexp;

    char *TextStartPointer = Text; //  points to start of string after match
    int AmountShifted = 0;

    regmatch_t match[N_MATCHES]; // Contains all matches

    unsigned int matchesCompleted = 0;
    int flags = REG_EXTENDED | REG_ICASE;
    if (regcomp(&regexp, Pattern, flags) != 0) // compiles regex
    {
        fprintf(stderr, "Could not compile regex");
        exit(1); // Exits if an error occured during regex compilation
    };
    while (1)
    {
        int error = regexec(&regexp, TextStartPointer, N_MATCHES, match, 0);
        if (error == 0)
        {
            matches = (struct RegexMatch *)realloc(matches, (matchesCompleted + 1) * RegexMatchesSize);                                            // Reallocates memory for matches array
            matches[matchesCompleted].Text = (char *)malloc((int)match[0].rm_eo - (int)match[0].rm_so + 1);                                        // Allocates memory for match
            matches[matchesCompleted].Text = strdup(getSubstring(TextStartPointer, (int)match[0].rm_so + StartPos, (int)match[0].rm_eo - EndPos)); // Adds substring to matchs array
            printf("Matched regex: %s\n", getSubstring(TextStartPointer, (int)match[0].rm_so, (int)match[0].rm_eo));
            matches[matchesCompleted].IsArrayEnd = false;
            matches[matchesCompleted].StartIndex = (unsigned int)match[0].rm_so + AmountShifted; // Saves start index so it doesn't need to be recalculated later
            matches[matchesCompleted].EndIndex = (unsigned int)match[0].rm_eo + AmountShifted;   // Does the same as above but for the end index
            printf("Adjusted regex substring: %s, so %i, eo %i\n", getSubstring(Text, matches[matchesCompleted].StartIndex, matches[matchesCompleted].EndIndex), matches[matchesCompleted].StartIndex, matches[matchesCompleted].EndIndex);
            TextStartPointer += (int)match[0].rm_eo; // Uses pointer arithmetic to set textStartPointer to end of match
            AmountShifted += (int)match[0].rm_eo;
            matchesCompleted++; // Increments count of matches
        }
        else
        {
            break;
        }
    }
    regfree(&regexp);
    matches = (struct RegexMatch *)realloc(matches, (matchesCompleted + 1) * RegexMatchesSize);
    matches[matchesCompleted].IsArrayEnd = true; // Allows array to be reliably iterated over
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
