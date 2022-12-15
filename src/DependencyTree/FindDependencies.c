#include "FindDependencies.h"

unsigned int RegexMatchArrayLength(struct RegexMatch *Array)
{
    struct RegexMatch *IteratePointer = &Array[0];
    unsigned int ArrayLength = 0;
    while (IteratePointer->IsArrayEnd == false)
    {
        // printf("loop\n");
        ArrayLength++;
        IteratePointer++;
    }
    printf("ArrayLength = %i\n", ArrayLength);
    return ArrayLength;
}

void EMSCRIPTEN_KEEPALIVE CombineRegexMatchArrays(struct RegexMatch **Array1, struct RegexMatch **Array2)
{

    if ((*Array2)[0].IsArrayEnd == false)
    {
        if ((*Array1)[0].IsArrayEnd == false)
        {
            unsigned int Array1Length = RegexMatchArrayLength(*Array1);
            printf("here?\n");
            unsigned int Array2Length = RegexMatchArrayLength(*Array2);
            printf("realloc near\n");
            *Array1 = realloc(*Array1, sizeof(struct RegexMatch) * (Array1Length + Array2Length + 2));

            printf("realloc done\n");
            for (unsigned int i = 0; i < Array2Length; i++)
            {
                printf("0: %i, \n", (*Array1)[0].IsArrayEnd);
                // memcpy(&Array1[i + Array1Length], &Array2[i], sizeof(struct RegexMatch));

                (*Array1)[i + Array1Length].EndIndex = (*Array2)[i].EndIndex;
                (*Array1)[i + Array1Length].StartIndex = (*Array2)[i].StartIndex;
                (*Array1)[i + Array1Length].Text = strdup((*Array2)[i].Text);
                (*Array1)[i + Array1Length].IsArrayEnd = false;
                printf("00: %i\n", (*Array1)[0].IsArrayEnd);
                printf("Array[%i + %i] = %s, Array2[%i] = %s, eoa: %i\n", i, Array1Length, (*Array1)[i + Array1Length].Text, i, (*Array2)[i].Text, (*Array2)->IsArrayEnd == true);
            }
            (*Array1)[Array1Length + Array2Length - 1].IsArrayEnd = true;
        }
        else
        {
            *Array1 = *Array2;
        }
    }
    printf("is this 1: %i\n", (*Array1)[0].IsArrayEnd);
}

void MakeMatchesFullPath(struct RegexMatch *matches, char *BaseFilePath)
{
    struct RegexMatch *IteratePointer = &matches[0];
    while (IteratePointer->IsArrayEnd != true)
    {
        // printf("Path: %s, FullRelativePath: %s\n", IteratePointer->Text, TurnToFullRelativePath(IteratePointer->Text, NULL));
        printf("strdup(%s, %s)\n", IteratePointer->Text, BaseFilePath);
        IteratePointer->Text = TurnToFullRelativePath(IteratePointer->Text, GetBasePath(BaseFilePath));
        printf("Creating HTML element: %s\n", IteratePointer->Text);
        printf("0: %s\n", matches[0].Text);
        IteratePointer++;
    }
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *BasicRegexDependencies(char *filename, const char *pattern, unsigned int Startpos, unsigned int Endpos)
{ // Allows any function that only needs basic regex to easily be run
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
        printf("No dependencies found for file: %s with pattern: %s\n", filename, pattern);
        return NULL;
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
    /*
        ╭╮ ╭╮╭━━━━╮╭━╮╭━╮╭╮
        ┃┃ ┃┃┃╭╮╭╮┃┃┃╰╯┃┃┃┃
        ┃╰━╯┃╰╯┃┃╰╯┃╭╮╭╮┃┃┃
        ┃╭━╮┃  ┃┃  ┃┃┃┃┃┃┃┃ ╭╮
        ┃┃ ┃┃  ┃┃  ┃┃┃┃┃┃┃╰━╯┃
        ╰╯ ╰╯  ╰╯  ╰╯╰╯╰╯╰━━━╯*/
    struct RegexMatch *HTMLIncludeMatches = BasicRegexDependencies(filename, "<include src=\"[^>]*\"", 14, 2);

    MakeMatchesFullPath(HTMLIncludeMatches, filename);

    struct RegexMatch *IteratePointer = &HTMLIncludeMatches[0];
    char *TempStringPointer;
    /*
        ╭━━━╮╭━━━╮╭━━━╮
        ┃╭━╮┃┃╭━╮┃┃╭━╮┃
        ┃┃╱╰╯┃╰━━╮┃╰━━╮
        ┃┃╱╭╮╰━━╮┃╰━━╮┃
        ┃╰━╯┃┃╰━╯┃┃╰━╯┃
        ╰━━━╯╰━━━╯╰━━━╯*/
    // struct RegexMatch *CSSDependencies = BasicRegexDependencies(filename, "<link\\s+rel\\s*=\\s*(\"')?stylesheet(\"')?\\s*(type\\s*=\\s*(\"')?text/css(\"')?)?\\s+href\\s*=\\s*(\"')?(.*?)(\"')?\\s*\\/>", 5, 0);
    struct RegexMatch *CSSDependencies = BasicRegexDependencies(filename, "<link[^>$]*stylesheet[^>$]*", 0, 0);
    if (CSSDependencies != NULL)
    {
        IteratePointer = &CSSDependencies[0];
        while (IteratePointer->IsArrayEnd == false) // Gets link from CSS element
        {
            printf("CSS loop\n"); //
            int HREFLocation = -1;
            TempStringPointer = &IteratePointer->Text[0];
            int TextLength = strlen(IteratePointer->Text);
            for (unsigned int i = 0; i < TextLength; i++)
            {
                if (strncasecmp(TempStringPointer + i, "href", 4) == 0)
                {
                    HREFLocation = i + 4;
                    break;
                }
            }
            int StartLocation = -1;
            int EndLocation = -1;
            if (HREFLocation != -1)
            {
                bool EqualsFound = false;
                for (unsigned int i = HREFLocation; i < TextLength; i++)
                {
                    if (!EqualsFound)
                    {
                        if (TempStringPointer[i] == '=')
                        {
                            EqualsFound = true;
                        }
                    }
                    else
                    {
                        if (TempStringPointer[i] != ' ' && TempStringPointer[i] != '\"' && TempStringPointer[i] != '\'')
                        {
                            StartLocation = i;
                            break;
                        }
                    }
                }
                EndLocation = TextLength;
                if (StartLocation != -1)
                {
                    for (unsigned int i = StartLocation; i <= TextLength; i++)
                    {
                        if (TempStringPointer[i] == ' ' || TempStringPointer[i] == '\'' || TempStringPointer[i] == '\"' || TempStringPointer[i] == '>')
                        {
                            EndLocation = i;
                            break;
                        }
                    }
                }
            }
            printf("FileName: %s, Basepath: %s\n", filename, GetBasePath(filename));
            IteratePointer->Text = strdup(TurnToFullRelativePath(getSubstring(IteratePointer->Text, StartLocation, EndLocation - 1), GetBasePath(filename)));
            IteratePointer++;
        }
    }
    printf("Code reaches here\n");

    CombineRegexMatchArrays(&HTMLIncludeMatches, &CSSDependencies);
    printf("Code doesn't reach here\n");
    IteratePointer = &HTMLIncludeMatches[0];
    printf("hmm\n");
    printf("HMM: %i\n", HTMLIncludeMatches[0].IsArrayEnd);
    printf("hmm2\n");
    while (IteratePointer->IsArrayEnd != true)
    {
        printf("Final Iterate pointer: %s\n", IteratePointer->Text);
        IteratePointer++;
    }

    printf("finished printing\n");
    return HTMLIncludeMatches;
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