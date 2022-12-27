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
                printf("new code: %s\n", NewArray[i + Array1Length].Text);
                NewArray[i + Array1Length].IsArrayEnd = false;
            }

            // Set the end-of-array flag for the last element of the new array
            NewArray[Array1Length + Array2Length].IsArrayEnd = true;

            // Free the memory used by the original Array1
            free(*Array1);

            // Update Array1 to point to the new array
            *Array1 = NewArray;
            printf("is 2nd the start: %i\n", NewArray[1].IsArrayEnd);
        }
        else
        {
            // Array1 is empty, so just set it equal to Array2
            *Array1 = *Array2;
        }
    }
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
    // return HTMLIncludeMatches;

    struct RegexMatch *JSDependencies = BasicRegexDependencies(filename, "<script[^>$]*src[^>$]*", 0, 0);
    if (JSDependencies != NULL)
    {
        IteratePointer = &JSDependencies[0];
        while (IteratePointer->IsArrayEnd != true)
        {
            int srcLocation = -1;

            int TextLength = strlen(IteratePointer->Text);
            for (int i = 0; i < TextLength; i++)
            {
                if (strncasecmp(IteratePointer->Text + i, "src", 3) == 0)
                {
                    srcLocation = i + 3;
                    break;
                }
            }
            int StartLocation = -1;
            int EndLocation = -1;
            if (srcLocation != -1)
            {
                bool EqualsFound = false;
                for (unsigned int i = srcLocation; i < TextLength; i++)
                {
                    if (!EqualsFound)
                    {
                        if (IteratePointer->Text[i] == '=')
                        {
                            EqualsFound = true;
                        }
                    }
                    else
                    {
                        if (IteratePointer->Text[i] != ' ' && IteratePointer->Text[i] != '\"' && IteratePointer->Text[i] != '\'')
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
                        if (IteratePointer->Text[i] == ' ' || IteratePointer->Text[i] == '\'' || IteratePointer->Text[i] == '\"' || IteratePointer->Text[i] == '>')
                        {
                            EndLocation = i;
                            break;
                        }
                    }
                }
            }
            IteratePointer->Text = strdup(TurnToFullRelativePath(getSubstring(IteratePointer->Text, StartLocation, EndLocation - 1), GetBasePath(filename)));
            printf("JS: %s\n", IteratePointer->Text);
            IteratePointer++;
        }
    }
    printf("Code doesn't reach here\n");
    CombineRegexMatchArrays(&CSSDependencies, &JSDependencies);
    CombineRegexMatchArrays(&CSSDependencies, &HTMLIncludeMatches);

    IteratePointer = &CSSDependencies[0];
    while (IteratePointer->IsArrayEnd != true)
    {
        printf("Final Iterate pointer: %s\n", IteratePointer->Text);
        IteratePointer++;
    }

    printf("finished printing\n");
    return CSSDependencies;
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindCSSDependencies(char *filename)
{
    struct RegexMatch *Dependencies = BasicRegexDependencies(filename, "@import [^;]*", 0, 0);
    struct RegexMatch *IteratePointer = &Dependencies[0];
    while (IteratePointer->IsArrayEnd != true)
    {
        int StartLocation = -1;
        int EndLocation = -1;
        for (int i = 0; i < strlen(IteratePointer->Text); i++)
        {
            if (IteratePointer->Text[i] == 'u' && IteratePointer->Text[i + 1] == 'r' && IteratePointer->Text[i + 2] == 'l' && IteratePointer->Text[i + 3] == '(')
            {
                StartLocation = i + 5;
                for (int v = i + 5; v < strlen(IteratePointer->Text); v++)
                {
                    if (IteratePointer->Text[v] == ')')
                    {
                        EndLocation = v - 2;
                    }
                }
            }
        }

        strcpy(IteratePointer->Text, getSubstring(IteratePointer->Text, StartLocation, EndLocation));
        printf("CSS URL: %s", IteratePointer->Text);
        IteratePointer++;
    }
    MakeMatchesFullPath(Dependencies, GetBasePath(filename));
    return Dependencies;
}