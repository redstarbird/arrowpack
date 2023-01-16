#include "FindDependencies.h"

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

void ShiftRegexMatches(struct RegexMatch **matches, int Location, int Amount)
{
    struct RegexMatch *IteratePointer = &(*matches)[0];
    while (!(IteratePointer++)->IsArrayEnd)
    {
        IteratePointer->EndIndex += Amount * IteratePointer->EndIndex >= Location;
        IteratePointer->StartIndex += Amount * IteratePointer->StartIndex >= Location;
    }
}

void RemoveRegexMatch(struct RegexMatch *match)
{
    struct RegexMatch *LastMatch = match;
    while (1)
    {
        match++;
        if (match->IsArrayEnd == true)
        {
            LastMatch->IsArrayEnd = true;
            break;
        }
        else
        {
            LastMatch->Text = match->Text;
            LastMatch->EndIndex = match->EndIndex;
            LastMatch->StartIndex = match->StartIndex;
            LastMatch++;
        }
    }
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *BasicRegexDependencies(char *filename, const char *pattern, unsigned int Startpos, unsigned int Endpos, struct RegexMatch *CommentLocations)
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
    struct RegexMatch *IteratePointer = &RegexMatches[0];
    while (IteratePointer->IsArrayEnd != true)
    {
        printf("\n\nWow: %s\n\n\n", IteratePointer->Text);
        if (StringContainsSubstring(IteratePointer->Text, "https://") || StringContainsSubstring(IteratePointer->Text, "http://"))
        {
            printf("Found URL: %s\n", IteratePointer->Text);
            RemoveRegexMatch(IteratePointer);
        }

        if (CommentLocations != NULL)
        {
            if (CommentLocations[0].IsArrayEnd != true)
            {
                struct RegexMatch *CurrentComment = &CommentLocations[0];
                while (IteratePointer->StartIndex < CurrentComment->EndIndex)
                {
                    if (IteratePointer->StartIndex > CurrentComment->StartIndex && IteratePointer->StartIndex < CurrentComment->EndIndex)
                    {
                        RemoveRegexMatch(IteratePointer);
                    }
                }
            }
        }
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

struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindHTMLDependencies(struct Node *vertex, struct Graph **DependencyGraph)
{

    char *filename = vertex->path;
    /*
        ╭╮ ╭╮╭━━━━╮╭━╮╭━╮╭╮
        ┃┃ ┃┃┃╭╮╭╮┃┃┃╰╯┃┃┃┃
        ┃╰━╯┃╰╯┃┃╰╯┃╭╮╭╮┃┃┃
        ┃╭━╮┃  ┃┃  ┃┃┃┃┃┃┃┃ ╭╮
        ┃┃ ┃┃  ┃┃  ┃┃┃┃┃┃┃╰━╯┃
        ╰╯ ╰╯  ╰╯  ╰╯╰╯╰╯╰━━━╯*/
    struct RegexMatch *HTMLIncludeMatches = BasicRegexDependencies(filename, "<include src=\"[^>]*\"", 14, 2, NULL);

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
    struct RegexMatch *CSSDependencies = BasicRegexDependencies(filename, "<link[^>$]*stylesheet[^>$]*", 0, 0, NULL);
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

    struct RegexMatch *JSDependencies = BasicRegexDependencies(filename, "<script[^>$]*", 7, 0, NULL);
    if (JSDependencies != NULL)
    {
        IteratePointer = &JSDependencies[0];
        while (IteratePointer->IsArrayEnd != true)
        {
            int srcLocation = -1;

            int TextLength = strlen(IteratePointer->Text);
            for (int i = 0; i < TextLength - 2; i++)
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
                    printf("start location: %i, end location: %i, substring %s\n", StartLocation, EndLocation, getSubstring(IteratePointer->Text, StartLocation, EndLocation - 1));
                    IteratePointer->Text = strdup(TurnToFullRelativePath(getSubstring(IteratePointer->Text, StartLocation, EndLocation - 1), GetBasePath(filename)));
                }
            }
            else
            {
                int StartLocation = -1;
                int EndLocation = -1;
                char *TempFileContents = ReadDataFromFile(filename);
                int textLength = strlen(TempFileContents);
                for (int i = IteratePointer->StartIndex; i < textLength; i++)
                {
                    if (TempFileContents[i] == '>')
                    {
                        StartLocation = i;
                        break;
                    }
                }
                if (StartLocation != -1)
                {

                    for (int i = StartLocation; i < strlen(TempFileContents); i++)
                    {
                        if (TempFileContents[i] == '<')
                        {
                            EndLocation = i;
                            break;
                        }
                    }
                    if (EndLocation != -1)
                    {
                        printf("Success\n");
                        char *NewJSContents = getSubstring(TempFileContents, StartLocation + 1, EndLocation - 2);
                        printf("New JS Contents:%s\n", NewJSContents);

                        char *NewJSName = TurnToFullRelativePath(CreateUnusedName(), GetBasePath(filename));
                        NewJSName = strdup(EntryToPreprocessPath(NewJSName));
                        NewJSName = realloc(NewJSName, (strlen(NewJSName) + 5) * sizeof(char));
                        strcat(NewJSName, ".js");

                        CreateFileWrite(NewJSName, NewJSContents);

                        RemoveSectionOfString(TempFileContents, StartLocation + 1, EndLocation - 1);
                        char *NewSRCAttr = malloc(9 + strlen(NewJSName));
                        strcpy(NewSRCAttr, " src=\"");
                        strcat(NewSRCAttr, NewJSName);
                        strcat(NewSRCAttr, "\" ");
                        printf("New src %s\n", NewSRCAttr);

                        TempFileContents = InsertStringAtPosition(TempFileContents, NewSRCAttr, StartLocation);
                        printf("\n\n%s\n", TempFileContents);

                        filename = EntryToPreprocessPath(filename);
                        vertex->path = filename;
                        printf("vertex %s\n", vertex->path);

                        CreateFileWrite(filename, TempFileContents);

                        ShiftRegexMatches(&JSDependencies, StartLocation - 1, strlen(NewSRCAttr));
                        ShiftRegexMatches(&HTMLIncludeMatches, StartLocation - 1, strlen(NewSRCAttr));
                        ShiftRegexMatches(&CSSDependencies, StartLocation - 1, strlen(NewSRCAttr));

                        ShiftRegexMatches(&JSDependencies, StartLocation + 1, -1 * (EndLocation - StartLocation));
                        ShiftRegexMatches(&CSSDependencies, StartLocation + 1, -1 * (EndLocation - StartLocation));
                        ShiftRegexMatches(&HTMLIncludeMatches, StartLocation + 1, -1 * (EndLocation - StartLocation));

                        add_vertex(*DependencyGraph, create_vertex(NewJSName, JSFILETYPE_ID, NULL));
                        // CreateDependencyEdges((*DependencyGraph)->Vertexes[(*DependencyGraph)->VerticesNum - 1], DependencyGraph);
                        IteratePointer->Text = NewJSName;
                        IteratePointer->StartIndex = StartLocation - 1;
                        IteratePointer->EndIndex = StartLocation + 8 + strlen(NewJSName);
                    }
                }
            }
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
    char *FileContents = ReadDataFromFile(filename);
    struct RegexMatch *CommentLocations = GetAllRegexMatches(FileContents, "/\\*.*\\*/", 0, 0);
    struct RegexMatch *Dependencies = BasicRegexDependencies(filename, "@import [^;]*", 0, 0, CommentLocations);
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
    free(CommentLocations);
    return Dependencies;
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindJSDependencies(char *filename)
{
    char *FileContents = ReadDataFromFile(filename);
    struct RegexMatch *CommentLocations = GetAllRegexMatches(FileContents, "/\\*.*\\*/", 0, 0);
    struct RegexMatch *CommentLocations2 = GetAllRegexMatches(FileContents, "//.*\n", 0, 1);
    CombineRegexMatchArrays(&CommentLocations, &CommentLocations2);
    free(CommentLocations2);
    CommentLocations2 = NULL;
    struct RegexMatch *CJSDependencies = BasicRegexDependencies(filename, "require[^)]*", 0, 1, CommentLocations);
    if (CJSDependencies == NULL)
    {
        return NULL;
    }
    struct RegexMatch *IteratePointer = &CJSDependencies[0];

    while (IteratePointer->IsArrayEnd != true)
    {
        int startLocation = -1;
        int endLocation = -1;
        int stringlength = strlen(IteratePointer->Text);
        bool bracketFound = false;
        bool QuotationMarkFound = false;
        for (int i = 0; i < stringlength; i++)
        {
            if (!bracketFound)
            {
                if (IteratePointer->Text[i] == '(')
                {
                    bracketFound = true;
                }
            }
            else if (!QuotationMarkFound)
            {
                if (IteratePointer->Text[i] == '\"' || IteratePointer->Text[i] == '\'')
                {
                    QuotationMarkFound = true;
                }
            }
            else
            {
                if (startLocation == -1)
                {
                    if (IteratePointer->Text[i] != ' ' && IteratePointer->Text[i] != '\'' && IteratePointer->Text[i] != '\"')
                    {
                        startLocation = i;
                    }
                }
                else
                {
                    endLocation = i;
                    if (IteratePointer->Text[i] == ' ' || IteratePointer->Text[i] == '\'' || IteratePointer->Text[i] == '\"')
                    {
                        break;
                    }
                }
            }
        }
        if (startLocation != -1 && endLocation != -1)
        {
            strcpy(IteratePointer->Text, getSubstring(IteratePointer->Text, startLocation, endLocation - 1));
            printf("Yay: %s\n", IteratePointer->Text);
            bool LocalFileFound = false;

            if (!StringEndsWith(IteratePointer->Text, ".cjs") && !StringEndsWith(IteratePointer->Text, ".js"))
            {

                char *TempCheckPath = TurnToFullRelativePath(IteratePointer->Text, GetBasePath(IteratePointer->Text));
                int TempLength = strlen(TempCheckPath);
                TempCheckPath = realloc(TempCheckPath, TempLength + 5);
                strcat(TempCheckPath, ".js");
                if (FileExists(TempCheckPath))
                {
                    LocalFileFound = true;
                    free(IteratePointer->Text);
                    IteratePointer->Text = TurnToFullRelativePath(TempCheckPath, GetBasePath(filename));
                }
                else
                {
                    TempCheckPath = ReplaceSectionOfString(TempCheckPath, TempLength, TempLength + 3, ".cjs\n");
                    TempCheckPath[TempLength + 4] = '\0';
                    printf("CJS path: %s\n", TurnToFullRelativePath(TempCheckPath, GetBasePath(filename)));
                    if (FileExists(TurnToFullRelativePath(TempCheckPath, GetBasePath(filename))))
                    {
                        LocalFileFound = true;
                        free(IteratePointer->Text);
                        IteratePointer->Text = TurnToFullRelativePath(TempCheckPath, GetBasePath(filename));
                        printf("Relative CJS path: %s\n", IteratePointer->Text);
                    }
                }
            }

            else if (FileExists(TurnToFullRelativePath(IteratePointer->Text, GetBasePath(filename))))
            {

                LocalFileFound = true;

                IteratePointer->Text = TurnToFullRelativePath(IteratePointer->Text, GetBasePath(filename));
                printf("Exiting %s\n", IteratePointer->Text);
            }
            if (!LocalFileFound)
            {
                if (DirectoryExists("node_modules"))
                {
                    printf("Node modules exists\n");
                    char *NodeModulePath = malloc(14 + strlen(IteratePointer->Text));
                    strcpy(NodeModulePath, "node_modules/");
                    strcat(NodeModulePath, IteratePointer->Text);
                    printf("Node modules path: %s\n", NodeModulePath);
                    if (DirectoryExists(NodeModulePath))
                    {
                        char *PackageJSONPath = malloc(strlen(NodeModulePath) + 15);
                        strcpy(PackageJSONPath, NodeModulePath);
                        strcat(PackageJSONPath, "/package.json");
                        if (FileExists(PackageJSONPath))
                        {
                            char *PackageJSONContent = ReadDataFromFile(PackageJSONPath);
                            cJSON *PackageJSON = cJSON_Parse(PackageJSONContent);
                            if (PackageJSON == NULL)
                            {
                                ThrowFatalError("Error parsing package.json at %s\n", PackageJSONPath);
                            }
                            cJSON *main_field = cJSON_GetObjectItemCaseSensitive(PackageJSON, "main");
                            NodeModulePath = realloc(NodeModulePath, strlen(NodeModulePath) + strlen(main_field->valuestring) + 1);
                            strcat(NodeModulePath, "/");
                            strcat(NodeModulePath, main_field->valuestring);
                            free(PackageJSONContent);
                            free(PackageJSONPath);
                            free(IteratePointer->Text);
                            IteratePointer->Text = NodeModulePath;
                            LocalFileFound = true;
                            cJSON_Delete(PackageJSON);
                            printf("wow %s\n", IteratePointer->Text);
                        }
                    }
                }
                if (!LocalFileFound)
                {
                    ThrowFatalError("Could not find module: %s\n", IteratePointer->Text);
                }
            }
        }
        IteratePointer++;
    }
    struct RegexMatch *ESDependencies = BasicRegexDependencies(filename, "import [^;]*from[^;]*", 8, 0, CommentLocations);
    while (IteratePointer->IsArrayEnd != true)
    {
        int StartLocation = -1;
        int EndLocation = -1;
        for (int i = 0; i < strlen(IteratePointer->Text); i++)
        {
            if (strncasecmp(IteratePointer->Text + i, " from ", 6) == 0)
            {
                StartLocation = i + 5; // Doesn't break so that function won't be messed up if function name includes " from "
            }
        }
        if (StartLocation == -1)
        {
            printf("Could not find JS dependency in import\n");
        }
        else
        {
            for (int i = StartLocation; i < strlen(IteratePointer->Text); i++)
            {
                if (IteratePointer->Text[i] != ' ' && IteratePointer->Text[i] != '\'' && IteratePointer->Text[i] != '\"')
                {
                    StartLocation = i;
                    break;
                }
            }
        }
    }

    return CJSDependencies;
}