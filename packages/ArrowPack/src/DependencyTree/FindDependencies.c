#include "FindDependencies.h"

struct RegexMatch EMSCRIPTEN_KEEPALIVE *BasicRegexDependencies(char *filename, const char *pattern, unsigned int Startpos, unsigned int Endpos)
{ // Allows any function that only needs basic regex to easily be run
    printf("filename: %s\n", filename);
    char *FileContents = ReadDataFromFile(filename);
    if (FileContents == NULL)
    {
        return NULL;
    }
    // printf("file contents: %s\n", FileContents);
    // printf("num of regex mathces %d\n", GetNumOfRegexMatches(FileContents, pattern));
    struct RegexMatch *RegexMatches = GetAllRegexMatches(FileContents, pattern, Startpos, Endpos);
    if (RegexMatches == NULL)
    {
        printf("No dependencies found for file: %s\n", filename);
        return NULL;
    }
    struct RegexMatch *IteratePointer = &RegexMatches[0];
    while (strlen(IteratePointer->Text) > 0)
    {
        IteratePointer->Text = TurnToFullRelativePath(IteratePointer->Text, GetBasePath(filename));
        printf("DIfferent loop technique: %s\n", IteratePointer->Text);
        IteratePointer++;
    }
    /*
        printf("Size of regex matches: %i, size of regex matches[0]: %i\n", (int)sizeof(RegexMatches), (int)sizeof(RegexMatches));
        printf("Looping %i times\n", (int)sizeof(&RegexMatches) / (int)sizeof(RegexMatches[0]));
        for (int i = 0; i < sizeof(&RegexMatches) / sizeof(RegexMatches[0]) + 1; i++)
        {
            printf("test1245\n");
            printf("hmm: %s\n", RegexMatches[i].Text);
            RegexMatches[i].Text = strdup(TurnToFullRelativePath(RegexMatches[i].Text, ""));
            // RegexMatches[i] = TurnToFullRelativePath(RegexMatches[i], "");
            printf("hmmv2: %s\n", RegexMatches[i].Text);
            if (i == 1)
            {
                printf("this is interesting: %s\n", RegexMatches[i - 1].Text);
            }
        }*/

    printf("Got all regex matches\n");

    free(FileContents);
    return RegexMatches;
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindHTMLDependencies(char *filename)
{

    return BasicRegexDependencies(filename, "<include src=\"[^>]*\"", 14, 2); // Abstraction
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindCSSDependencies(char *filename)
{
    struct RegexMatch *Dependencies = BasicRegexDependencies(filename, "@import .*;", 9, 2);
    for (int i = 0; i < sizeof(Dependencies) / sizeof(char *); i++)
    {
        if (HasRegexMatch(Dependencies[i].Text, "rl\\(*.\"")) // Removes "url()" CSS function from string
        {
            strcpy(Dependencies[i].Text, getSubstring(Dependencies[i].Text, 4, 1));
        }
    }
    return Dependencies;
}