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

struct RegexMatch EMSCRIPTEN_KEEPALIVE *BasicRegexDependencies(char *filename, const char *pattern, unsigned int Startpos, unsigned int Endpos, struct RegexMatch *CommentLocations)
{ // Allows any function that only needs basic regex to easily be run

    char *FileContents = ReadDataFromFile(filename);
    if (FileContents == NULL)
    {
        return NULL;
    }
    struct RegexMatch *RegexMatches = GetAllRegexMatches(FileContents, pattern, Startpos, Endpos);
    if (RegexMatches == NULL)
    {
        printf("No dependencies found for file: %s\n", filename);
        return NULL;
    }
    struct RegexMatch *IteratePointer = &RegexMatches[0];
    while (IteratePointer->IsArrayEnd != true)
    {
        if (StringContainsSubstring(IteratePointer->Text, "https://") || StringContainsSubstring(IteratePointer->Text, "http://"))
        {
            printf("Ignoring URL: %s\n", IteratePointer->Text);
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
                    CurrentComment++;
                }
            }
        }
        IteratePointer++;
    }

    free(FileContents);
    return RegexMatches;
}

struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindHTMLDependencies(struct Node *vertex, struct Graph **DependencyGraph)
{

    char *filename = vertex->path;
    char *FileNameBasePath = GetBasePath(filename);
    /*
        ╭╮ ╭╮╭━━━━╮╭━╮╭━╮╭╮
        ┃┃ ┃┃┃╭╮╭╮┃┃┃╰╯┃┃┃┃
        ┃╰━╯┃╰╯┃┃╰╯┃╭╮╭╮┃┃┃
        ┃╭━╮┃  ┃┃  ┃┃┃┃┃┃┃┃ ╭╮
        ┃┃ ┃┃  ┃┃  ┃┃┃┃┃┃┃╰━╯┃
        ╰╯ ╰╯  ╰╯  ╰╯╰╯╰╯╰━━━╯*/

    char *FileContents = ReadDataFromFile(filename);
    unsigned int CommentLocationsFound = 0;
    struct RegexMatch *CommentLocations = NULL;
    for (unsigned int i = 0; i < strlen(FileContents); i++)
    {
        int start, end = -1;
        if (FileContents[i] == '<' && FileContents[i + 1] == '!' && FileContents[i + 2] == '-' && FileContents[i + 3] == '-')
        {
            start = i;
            i = i + 3;
            while (i < strlen(FileContents) - 2)
            {
                if (FileContents[i] == '-' && FileContents[i + 1] == '-' && FileContents[i + 2] == '>')
                {
                    end = i + 2;
                    CommentLocationsFound++;
                    CommentLocations = realloc(CommentLocations, sizeof(struct RegexMatch) * (CommentLocationsFound + 1));
                    CommentLocations[CommentLocationsFound].IsArrayEnd = true;
                    CommentLocations[CommentLocationsFound - 1].StartIndex = start;
                    CommentLocations[CommentLocationsFound - 1].EndIndex = end;
                    CommentLocations[CommentLocationsFound - 1].IsArrayEnd = false;
                    break;
                }
                i++;
            }
        }
    }
    struct RegexMatch *HTMLIncludeMatches = BasicRegexDependencies(filename, "<include src=\"[^>]*\"", 14, 2, CommentLocations);
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
    struct RegexMatch *CSSDependencies = BasicRegexDependencies(filename, "<link[^>$]*stylesheet[^>$]*", 0, 0, CommentLocations);
    if (CSSDependencies != NULL)
    {
        IteratePointer = &CSSDependencies[0];
        while (IteratePointer->IsArrayEnd == false) // Gets link from CSS element
        {
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
            IteratePointer->Text = strdup(TurnToFullRelativePath(getSubstring(IteratePointer->Text, StartLocation, EndLocation - 1), GetBasePath(filename)));
            IteratePointer++;
        }
    }

    struct RegexMatch *JSDependencies = BasicRegexDependencies(filename, "<script[^>$]*", 7, 0, CommentLocations);
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
                    IteratePointer->Text = TurnToFullRelativePath(getSubstring(IteratePointer->Text, StartLocation, EndLocation - 1), FileNameBasePath);
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
                        char *NewJSContents = getSubstring(TempFileContents, StartLocation + 1, EndLocation - 2);

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

                        TempFileContents = InsertStringAtPosition(TempFileContents, NewSRCAttr, StartLocation);

                        filename = EntryToPreprocessPath(filename);
                        vertex->path = filename;

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
            IteratePointer++;
        }
    }
    CombineRegexMatchArrays(&CSSDependencies, &JSDependencies);
    CombineRegexMatchArrays(&CSSDependencies, &HTMLIncludeMatches);

    IteratePointer = &CSSDependencies[0];
    while (IteratePointer->IsArrayEnd != true)
    {
        IteratePointer++;
    }

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
                    if (FileExists(TurnToFullRelativePath(TempCheckPath, GetBasePath(filename))))
                    {
                        LocalFileFound = true;
                        free(IteratePointer->Text);
                        IteratePointer->Text = TurnToFullRelativePath(TempCheckPath, GetBasePath(filename));
                    }
                }
            }

            else if (FileExists(TurnToFullRelativePath(IteratePointer->Text, GetBasePath(filename))))
            {

                LocalFileFound = true;

                IteratePointer->Text = TurnToFullRelativePath(IteratePointer->Text, GetBasePath(filename));
            }
            if (!LocalFileFound)
            {
                if (DirectoryExists("node_modules"))
                {
                    printf("Node modules exists\n");
                    char *NodeModulePath = malloc(14 + strlen(IteratePointer->Text));
                    strcpy(NodeModulePath, "node_modules/");
                    strcat(NodeModulePath, IteratePointer->Text);
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
    struct RegexMatch *ESDependencies = BasicRegexDependencies(filename, "import [^;]*from[^;]*;", 8, 0, CommentLocations);
    IteratePointer = &ESDependencies[0];
    while (IteratePointer->IsArrayEnd != true)
    {

        int FromLocation = -1;
        int StartLocation = -1;
        int EndLocation = -1;
        bool InDestructure = false; // Makes it so that having " true " as a variable won't cause problems
        for (int i = 0; i < strlen(IteratePointer->Text) - 6; i++)
        {
            if (strncasecmp(IteratePointer->Text + i, " from ", 6) == 0)
            {
                FromLocation = i + 5; // Doesn't break so that function won't be messed up if function name includes " from "
            }
        }
        if (FromLocation == -1)
        {
            CreateWarning("Could not find JS dependency in import\n");
        }
        else
        {
            for (int i = FromLocation; i < strlen(IteratePointer->Text); i++)
            {
                if (IteratePointer->Text[i] != ' ' && IteratePointer->Text[i] != '\'' && IteratePointer->Text[i] != '\"')
                {
                    StartLocation = i;
                    break;
                }
            }
            if (StartLocation != -1)
            {
                for (int i = StartLocation; i < strlen(IteratePointer->Text); i++)
                {
                    if (IteratePointer->Text[i] == ' ' || IteratePointer->Text[i] == '\'' || IteratePointer->Text[i] == '\"')
                    {
                        EndLocation = i - 1;
                        break;
                    }
                }
                if (EndLocation != -1)
                {
                    IteratePointer->Text = TurnToFullRelativePath(getSubstring(IteratePointer->Text, StartLocation, EndLocation), GetBasePath(filename));

                    if (!StringEndsWith(IteratePointer->Text, ".js"))
                    {
                        IteratePointer->Text = realloc(IteratePointer->Text, strlen(IteratePointer->Text) + 4);
                        strcat(IteratePointer->Text, ".js");
                    }
                    printf("Found ESM path: %s\n", IteratePointer->Text);
                }
            }
        }
        IteratePointer++;
    }
    CombineRegexMatchArrays(&CJSDependencies, &ESDependencies);

    return CJSDependencies;
}