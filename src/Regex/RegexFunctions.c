#include "RegexFunctions.h"

// Removes a regex match from an array and destroys it
void RemoveRegexMatch(struct RegexMatch *match)
{
    struct RegexMatch *LastMatch = match;
    while (1) // Go through the array from after the match
    {
        match++;
        if (match->IsArrayEnd == true) // Stop at the end of the array
        {
            LastMatch->IsArrayEnd = true;
            break;
        }
        else // Shift all data to the previous match
        {
            LastMatch->Text = match->Text;
            LastMatch->EndIndex = match->EndIndex;
            LastMatch->StartIndex = match->StartIndex;
            LastMatch++;
        }
    }
}

// Returns the length of a regex match array
unsigned int RegexMatchArrayLength(struct RegexMatch *Array)
{
    struct RegexMatch *IteratePointer = &Array[0];
    unsigned int ArrayLength = 0;
    while (IteratePointer->IsArrayEnd == false)
    {
        ArrayLength++;
        IteratePointer++;
    }
    return ArrayLength;
}

// Combines two regex match arrays together
void EMSCRIPTEN_KEEPALIVE CombineRegexMatchArrays(struct RegexMatch **Array1, struct RegexMatch **Array2)
{
    // Check if Array2 is empty
    if ((*Array2)[0].IsArrayEnd == false)
    {
        // Check if Array1 is empty
        if ((*Array1)[0].IsArrayEnd == false)
        {
            // Calculate the lengths of the arrays
            unsigned int Array1Length = RegexMatchArrayLength(*Array1);
            unsigned int Array2Length = RegexMatchArrayLength(*Array2);

            // Allocate new memory for the combined array
            struct RegexMatch *NewArray = malloc(sizeof(struct RegexMatch) * (Array1Length + Array2Length + 2));

            // Copy the elements of Array1 into the new array
            memcpy(NewArray, *Array1, sizeof(struct RegexMatch) * Array1Length);

            // Copy the elements of Array2 into the new array
            for (unsigned int i = 0; i < Array2Length; i++)
            {
                NewArray[i + Array1Length].EndIndex = (*Array2)[i].EndIndex;
                NewArray[i + Array1Length].StartIndex = (*Array2)[i].StartIndex;
                NewArray[i + Array1Length].Text = strdup((*Array2)[i].Text);
                NewArray[i + Array1Length].IsArrayEnd = false;
            }

            // Set the end-of-array flag for the last element of the new array
            NewArray[Array1Length + Array2Length].IsArrayEnd = true;

            // Free the memory used by the original Array1
            free(*Array1);

            // Update Array1 to point to the new array
            *Array1 = NewArray;
        }
        else
        {
            // Array1 is empty, so just set it equal to Array2
            *Array1 = *Array2;
        }
    }
}

// Returns the number of times a pattern is matched in a string
int EMSCRIPTEN_KEEPALIVE GetNumOfRegexMatches(char *Text, const char *Pattern)
{
    const int N_MATCHES = 512;
    regex_t regexp;

    char *TextCopy = strdup(Text);

    char *TextStartPointer = &TextCopy[0]; //  points to start of string after match

    regmatch_t match[N_MATCHES]; // Contains all matches

    unsigned int matchesCompleted = 0;
    if (regcomp(&regexp, Pattern, 0) != 0) // compiles regex
    {
        ThrowFatalError("Could not compile regex pattern: %s", Pattern); // Exits if an error occured during regex compilation
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

// Gets the text of the first match of a pattern in a string
struct RegexMatch EMSCRIPTEN_KEEPALIVE *GetRegexMatch(char *Text, char *Pattern)
{
    struct RegexMatch *Match = malloc(sizeof(struct RegexMatch) * 2);
    regex_t regexp;

    const int N_MATCHES = 512;

    regmatch_t match[N_MATCHES];

    int flags = REG_EXTENDED | REG_ICASE;
    if (regcomp(&regexp, Pattern, flags) != 0) // compiles regex
    {
        ThrowFatalError("Could not compile regex pattern: %s", Pattern); // Throws a fatal error if an error occured during regex compilation
    };

    int error = regexec(&regexp, Text, N_MATCHES, match, flags);
    if (error == 0) // Assigns variables in regex match struct is a match is found
    {
        Match[0].IsArrayEnd = false;
        Match[0].StartIndex = match[0].rm_so;
        Match[0].EndIndex = match[0].rm_eo;
        Match[0].Text = getSubstring(Text, match[0].rm_so, match[0].rm_eo);
        Match[1].IsArrayEnd = true;
        return Match;
    }
    return NULL;
}

// Returns the text of all regex matches in a regex match array
struct RegexMatch EMSCRIPTEN_KEEPALIVE *GetAllRegexMatches(char *Text, const char *Pattern, unsigned int StartPos, unsigned int EndPos)
{
    const int N_MATCHES = 512;
    size_t RegexMatchesSize = sizeof(struct RegexMatch);
    struct RegexMatch *matches = malloc(RegexMatchesSize);
    regex_t regexp;

    char *TextStartPointer = Text; //  points to start of string after match
    int AmountShifted = 0;

    regmatch_t match[N_MATCHES]; // Contains all matches

    unsigned int matchesCompleted = 0;
    int flags = REG_EXTENDED | REG_ICASE;
    if (regcomp(&regexp, Pattern, flags) != 0) // compiles regex
    {
        ThrowFatalError("Could not compile regex pattern: %s", Pattern); // Exits if an error occured during regex compilation
    };
    while (1)
    {
        int error = regexec(&regexp, TextStartPointer, N_MATCHES, match, 0);
        if (error == 0)
        {
            matches = (struct RegexMatch *)realloc(matches, (matchesCompleted + 2) * RegexMatchesSize); // Reallocates memory for matches array

            matches[matchesCompleted].Text = getSubstring(TextStartPointer, (int)match[0].rm_so + StartPos, (int)match[0].rm_eo - EndPos); // Adds substring to matchs array
            matches[matchesCompleted].IsArrayEnd = false;
            matches[matchesCompleted].StartIndex = (unsigned int)match[0].rm_so + AmountShifted; // Saves start index so it doesn't need to be recalculated later
            matches[matchesCompleted].EndIndex = (unsigned int)match[0].rm_eo + AmountShifted;   // Does the same as above but for the end index
            TextStartPointer += (int)match[0].rm_eo;                                             // Uses pointer arithmetic to set textStartPointer to end of match
            AmountShifted += (int)match[0].rm_eo;
            matchesCompleted++; // Increments count of matches
        }
        else
        {
            break;
        }
    }
    regfree(&regexp);
    matches[matchesCompleted].IsArrayEnd = true; // Allows array to be reliably iterated over
    return matches;
}

// Returns whether a string matches a given regular expression pattern
bool EMSCRIPTEN_KEEPALIVE HasRegexMatch(const char *text, const char *pattern)
{
    regex_t regexp;

    int Error;

    Error = regcomp(&regexp, pattern, 0);

    if (Error != 0) // Throws an error if regex doesn't compile
    {
        regfree(&regexp);
        ThrowFatalError("Could not compile regex pattern: %s", pattern); // Exits if an error occured during regex compilation
    }

    Error = regexec(&regexp, text, 0, NULL, 0); // Check for a match with the pattern

    if (!Error) // A regex match has been found
    {
        regfree(&regexp);
        return true;
    }
    else if (Error == REG_NOMATCH) // A regex match has not been found
    {
        regfree(&regexp);
        return false;
    }
    else
    {
        regfree(&regexp);
        ThrowFatalError("Error when running regex on %s with pattern of %s\n", text, pattern);
    }
    return false;
}
