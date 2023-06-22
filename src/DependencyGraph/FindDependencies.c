#include "FindDependencies.h"

// Convert all file paths to full paths in matches
void MakeMatchesFullPath(struct RegexMatch *matches, char *BaseFilePath)
{
    struct RegexMatch *IteratePointer = &matches[0];
    while (IteratePointer->IsArrayEnd != true)
    {
        IteratePointer->Text = TurnToFullRelativePath(IteratePointer->Text, GetBasePath(BaseFilePath));
        IteratePointer++;
    }
}

// Shift all locations in regex matches
void ShiftRegexMatches(struct RegexMatch **matches, int Location, int Amount)
{
    struct RegexMatch *IteratePointer = &(*matches)[0];
    while (!IteratePointer->IsArrayEnd)
    {
        IteratePointer->EndIndex += Amount * (IteratePointer->EndIndex >= Location);
        IteratePointer->StartIndex += Amount * (IteratePointer->StartIndex >= Location);
        IteratePointer++;
    }
}

// Simple function using regex to find dependenciesss
struct RegexMatch EMSCRIPTEN_KEEPALIVE *BasicRegexDependencies(char *filename, const char *pattern, unsigned int Startpos, unsigned int Endpos, struct RegexMatch *CommentLocations)
{

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
    while (IteratePointer->IsArrayEnd != true) // Loop through all matches
    {
        if (StringContainsSubstring(IteratePointer->Text, "https://") || StringContainsSubstring(IteratePointer->Text, "http://")) // Remove any matches that are URLs
        {
            printf("Ignoring URL: %s\n", IteratePointer->Text);
            RemoveRegexMatch(IteratePointer);
            IteratePointer--;
        }
        else

            if (CommentLocations != NULL) // Checks if comments are present in the file
        {
            if (CommentLocations[0].IsArrayEnd != true) // Checks that comment list is not empty
            {
                struct RegexMatch *CurrentComment = &CommentLocations[0]; // Initialise variable to iterate through comment locations

                while (CurrentComment->IsArrayEnd != true) // Loops through comments
                {
                    if (IteratePointer->StartIndex > CurrentComment->StartIndex && IteratePointer->StartIndex < CurrentComment->EndIndex) // Checks if current dependency is inside of a comment
                    {

                        RemoveRegexMatch(IteratePointer); // Removes any dependencies inside of comments
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

// Finds all dependencies for a HTML file
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
    for (int i = 0; i < strlen(FileContents); i++) // Search for the locations of comments in the file
    {
        unsigned int start, end = 0;
        if (FileContents[i] == '<' && FileContents[i + 1] == '!' && FileContents[i + 2] == '-' && FileContents[i + 3] == '-') // Search for start of comment
        {
            start = i;
            i = i + 3;
            while (i < strlen(FileContents) - 2) // Search for end of comment
            {
                if (FileContents[i] == '-' && FileContents[i + 1] == '-' && FileContents[i + 2] == '>') // End of comment has been found
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
    struct RegexMatch *HTMLIncludeMatches = BasicRegexDependencies(filename, "<include src=\"[^>]*\"", 14, 2, CommentLocations); // Find all HTML include matches

    MakeMatchesFullPath(HTMLIncludeMatches, filename); // Turn all HTML dependency paths into full paths
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
    struct RegexMatch *CSSDependencies = BasicRegexDependencies(filename, "<link[^>$]*stylesheet[^>$]*", 0, 0, CommentLocations); // Find all CSS dependencies
    if (CSSDependencies != NULL)                                                                                                  // Find paths to CSS dependencies
    {
        IteratePointer = &CSSDependencies[0];
        while (IteratePointer->IsArrayEnd == false) // Gets link from CSS element
        {
            int HREFLocation = -1;
            TempStringPointer = &IteratePointer->Text[0];
            int TextLength = strlen(IteratePointer->Text);
            for (unsigned int i = 0; i < TextLength; i++) // Search for href keyword
            {
                if (strncasecmp(TempStringPointer + i, "href", 4) == 0)
                {
                    HREFLocation = i + 4;
                    break;
                }
            }
            int StartLocation = -1;
            int EndLocation = -1;
            if (HREFLocation != -1) // href location has been found
            {
                bool EqualsFound = false;
                for (unsigned int i = HREFLocation; i < TextLength; i++)
                {
                    if (!EqualsFound) // Search for equals sign
                    {
                        if (TempStringPointer[i] == '=')
                        {
                            EqualsFound = true;
                        }
                    }
                    else // Equals sign has already been found
                    {
                        if (TempStringPointer[i] != ' ' && TempStringPointer[i] != '\"' && TempStringPointer[i] != '\'') // Search start of path
                        {
                            StartLocation = i;
                            break;
                        }
                    }
                }
                EndLocation = TextLength;
                if (StartLocation != -1) // Start location has been found - search for end location
                {
                    for (unsigned int i = StartLocation; i <= TextLength; i++)
                    {
                        if (TempStringPointer[i] == ' ' || TempStringPointer[i] == '\'' || TempStringPointer[i] == '\"' || TempStringPointer[i] == '>' || TempStringPointer[i] == '\0')
                        {
                            EndLocation = i; // End location has been found
                            break;
                        }
                    }
                }
            }
            IteratePointer->Text = TurnToFullRelativePath(getSubstring(IteratePointer->Text, StartLocation, EndLocation - 1), GetBasePath(filename)); // Add path to matches array
            IteratePointer++;
        }
    }

    struct RegexMatch *JSDependencies = BasicRegexDependencies(filename, "<script[^>$]*", 7, 0, CommentLocations); // Search for Javascript dependencies

    if (JSDependencies != NULL) // Find paths of JavaScript dependencies if they are present
    {
        IteratePointer = &JSDependencies[0];
        while (IteratePointer->IsArrayEnd != true) // Loop through each dependency
        {
            int srcLocation = -1;

            int TextLength = strlen(IteratePointer->Text);
            for (int i = 0; i < TextLength - 2; i++) // Search for src attribute
            {
                if (strncasecmp(IteratePointer->Text + i, "src", 3) == 0)
                {
                    srcLocation = i + 3;
                    break;
                }
            }
            int StartLocation = -1;
            int EndLocation = -1;
            if (srcLocation != -1) // src attribute has been found
            {
                bool EqualsFound = false;
                for (unsigned int i = srcLocation; i < TextLength; i++)
                {
                    if (!EqualsFound) // Look for equals sign
                    {
                        if (IteratePointer->Text[i] == '=')
                        {
                            EqualsFound = true;
                        }
                    }
                    else // Equals sign has been found, search for start location
                    {
                        if (IteratePointer->Text[i] != ' ' && IteratePointer->Text[i] != '\"' && IteratePointer->Text[i] != '\'')
                        {
                            StartLocation = i;
                            break;
                        }
                    }
                }
                EndLocation = TextLength;
                if (StartLocation != -1) // Start location has been found, search for end location
                {
                    for (unsigned int i = StartLocation; i <= TextLength; i++)
                    {
                        if (IteratePointer->Text[i] == ' ' || IteratePointer->Text[i] == '\'' || IteratePointer->Text[i] == '\"' || IteratePointer->Text[i] == '>' || IteratePointer->Text[i] == '\0')
                        {
                            EndLocation = i; // End location found
                            break;
                        }
                    }
                    char *TempSubStr = getSubstring(IteratePointer->Text, StartLocation, EndLocation - 1); // Get file path

                    if (!StringEndsWith(TempSubStr, ".js")) // Make sure .js is at the end of the file path
                    {
                        TempSubStr = realloc(TempSubStr, (((EndLocation)-StartLocation) + 5) * sizeof(char));
                        strcat(TempSubStr, ".js");
                    }
                    IteratePointer->Text = TurnToFullRelativePath(TempSubStr, FileNameBasePath); // Apply JS path
                }
            }
            else // Script is inline JS so use a hacky way to make the inline JS it's own temporary file and link it to the HTML
            {
                int StartLocation = -1;
                int EndLocation = -1;
                char *TempFileContents = ReadDataFromFile(filename);
                int textLength = strlen(TempFileContents);
                for (int i = IteratePointer->StartIndex; i < textLength; i++) // Find the start location of the inline Javascript
                {
                    if (TempFileContents[i] == '>')
                    {
                        StartLocation = i;
                        break;
                    }
                }

                if (StartLocation != -1) // Start location has been found, search for end of inline JS
                {

                    for (int i = StartLocation; i < strlen(TempFileContents); i++) // Search for end character (could cause problems when in comments)
                    {
                        if (TempFileContents[i] == '<')
                        {
                            EndLocation = i;
                            break;
                        }
                    }

                    if (EndLocation != -1) // End location has been found
                    {
                        char *NewJSContents = getSubstring(TempFileContents, StartLocation + 1, EndLocation - 2); // Get contents of the inline JS

                        // Create a new name for the temporary JS file
                        char *NewJSName = TurnToFullRelativePath(CreateUnusedName(), GetBasePath(filename));
                        NewJSName = strdup(EntryToPreprocessPath(NewJSName));
                        NewJSName = realloc(NewJSName, (strlen(NewJSName) + 5) * sizeof(char));
                        strcat(NewJSName, ".js");

                        CreateFileWrite(NewJSName, NewJSContents); // Create the temporary file

                        // Add src link to the HTML file
                        RemoveSectionOfString(TempFileContents, StartLocation + 1, EndLocation - 1);
                        char *NewSRCAttr = malloc(9 + strlen(NewJSName));
                        strcpy(NewSRCAttr, " src=\"");
                        strcat(NewSRCAttr, NewJSName);
                        strcat(NewSRCAttr, "\" ");

                        TempFileContents = InsertStringAtPosition(TempFileContents, NewSRCAttr, StartLocation);

                        filename = EntryToPreprocessPath(filename);
                        vertex->path = filename;

                        CreateFileWrite(filename, TempFileContents); // Save new temporary HTML file

                        // Shift all dependencies used by the HTML by the amount the file has changed
                        ShiftRegexMatches(&JSDependencies, StartLocation - 1, strlen(NewSRCAttr) + 2);
                        ShiftRegexMatches(&HTMLIncludeMatches, StartLocation - 1, strlen(NewSRCAttr) + 2);
                        ShiftRegexMatches(&CSSDependencies, StartLocation - 1, strlen(NewSRCAttr) + 2);

                        ShiftRegexMatches(&JSDependencies, StartLocation + 1, -1 * (EndLocation - StartLocation));
                        ShiftRegexMatches(&CSSDependencies, StartLocation + 1, -1 * (EndLocation - StartLocation));
                        ShiftRegexMatches(&HTMLIncludeMatches, StartLocation + 1, -1 * (EndLocation - StartLocation));

                        // Add the new JS file to the dependency graph
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

    // Combine dependencies into a single array
    CombineRegexMatchArrays(&CSSDependencies, &JSDependencies);
    CombineRegexMatchArrays(&CSSDependencies, &HTMLIncludeMatches);

    return CSSDependencies;
}

// Finds all dependencies for a CSS file
struct RegexMatch EMSCRIPTEN_KEEPALIVE *FindCSSDependencies(char *filename)
{
    char *FileContents = ReadDataFromFile(filename);
    struct RegexMatch *CommentLocations = GetAllRegexMatches(FileContents, "/\\*.*\\*/", 0, 0);                  // Finds locations of comments
    struct RegexMatch *Dependencies = BasicRegexDependencies(filename, "@import [^;]*", 0, 0, CommentLocations); // Finds dependencies

    struct RegexMatch *IteratePointer = &Dependencies[0];
    while (IteratePointer->IsArrayEnd != true) // Loop through all matches
    {
        int StartLocation = -1;
        int EndLocation = -1;
        for (int i = 0; i < strlen(IteratePointer->Text); i++) // Search for url keyword
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

    // Find comment locations for both types of JS comments
    struct RegexMatch *CommentLocations = GetAllRegexMatches(FileContents, "/\\*.*\\*/", 0, 0);
    struct RegexMatch *CommentLocations2 = GetAllRegexMatches(FileContents, "//.*\n", 0, 1);
    CombineRegexMatchArrays(&CommentLocations, &CommentLocations2);
    free(CommentLocations2);
    CommentLocations2 = NULL;

    struct RegexMatch *CJSDependencies = BasicRegexDependencies(filename, "require[^)]*", 0, 1, CommentLocations); // Get commonJS dependencies

    if (CJSDependencies == NULL)
    {
        return NULL;
    }
    struct RegexMatch *IteratePointer = &CJSDependencies[0];
    while (IteratePointer->IsArrayEnd != true) // Loop through CommonJS dependencies
    {
        int startLocation = -1;
        int endLocation = -1;
        int stringlength = strlen(IteratePointer->Text);
        bool bracketFound = false;
        bool QuotationMarkFound = false;
        for (int i = 0; i < stringlength; i++)
        {
            if (!bracketFound) // Look for bracket in require statement
            {
                if (IteratePointer->Text[i] == '(')
                {
                    bracketFound = true;
                }
            }
            else if (!QuotationMarkFound) // Look for start of quotes for dependency name
            {
                if (IteratePointer->Text[i] == '\"' || IteratePointer->Text[i] == '\'')
                {
                    QuotationMarkFound = true;
                }
            }
            else
            {
                if (startLocation == -1) // Look for the start of the dependency name
                {
                    if (IteratePointer->Text[i] != ' ' && IteratePointer->Text[i] != '\'' && IteratePointer->Text[i] != '\"')
                    {
                        startLocation = i;
                    }
                }
                else // Look for the end of the dependency name
                {
                    endLocation = i;
                    if (IteratePointer->Text[i] == ' ' || IteratePointer->Text[i] == '\'' || IteratePointer->Text[i] == '\"')
                    {
                        break;
                    }
                }
            }
        }
        if (startLocation != -1 && endLocation != -1) // Dependency name has been found
        {
            strcpy(IteratePointer->Text, getSubstring(IteratePointer->Text, startLocation, endLocation - 1));
            bool LocalFileFound = false;

            if (!StringEndsWith(IteratePointer->Text, ".cjs") && !StringEndsWith(IteratePointer->Text, ".js")) // Dependency name doesn't include file extension
            {
                // Add .js file extensiom
                char *TempCheckPath = TurnToFullRelativePath(IteratePointer->Text, GetBasePath(IteratePointer->Text));
                int TempLength = strlen(TempCheckPath);
                TempCheckPath = realloc(TempCheckPath, TempLength + 5);
                strcat(TempCheckPath, ".js");

                if (FileExists(TempCheckPath)) // Check if the file exists with a .js extension
                {
                    LocalFileFound = true;
                    free(IteratePointer->Text);
                    IteratePointer->Text = TurnToFullRelativePath(TempCheckPath, GetBasePath(filename));
                }
                else // Check if the file exists with a .cjs extension
                {
                    strcpy(TempCheckPath + TempLength, ".cjs");
                    char *RelativeCheckPath = TurnToFullRelativePath(TempCheckPath, GetBasePath(filename));
                    if (FileExists(RelativeCheckPath))
                    {
                        LocalFileFound = true;
                        free(IteratePointer->Text);
                        IteratePointer->Text = RelativeCheckPath;
                    }
                }
            }

            else if (FileExists(TurnToFullRelativePath(IteratePointer->Text, GetBasePath(filename)))) // Dependency name includes file extension check if it exists in CWD
            {

                LocalFileFound = true;
                IteratePointer->Text = TurnToFullRelativePath(IteratePointer->Text, GetBasePath(filename));
            }
            if (!LocalFileFound) // Local module not found, it may be in node_modules
            {
                if (DirectoryExists("node_modules")) // Check there is a node_modules directory
                {
                    // Create node_modules path of the module
                    char *NodeModulePath = malloc(14 + strlen(IteratePointer->Text));
                    strcpy(NodeModulePath, "node_modules/");
                    strcat(NodeModulePath, IteratePointer->Text);

                    if (DirectoryExists(NodeModulePath)) // Check if the module exists in the node_modules directory
                    {
                        // Create the package.json path of the module
                        char *PackageJSONPath = malloc(strlen(NodeModulePath) + 18);
                        strcpy(PackageJSONPath, NodeModulePath);
                        strcat(PackageJSONPath, "/package.json");

                        if (FileExists(PackageJSONPath)) // Check if package.json exists
                        {
                            char *PackageJSONContent = ReadDataFromFile(PackageJSONPath); // Read package.json contents

                            cJSON *PackageJSON = cJSON_Parse(PackageJSONContent); // Parse the JSON with cJSON
                            if (PackageJSON == NULL)
                            {
                                ThrowFatalError("Error parsing package.json at %s\n", PackageJSONPath);
                            }

                            cJSON *main_field = cJSON_GetObjectItemCaseSensitive(PackageJSON, "main"); // Use cJSON to find the entry point specified in package.json

                            // Creates path to entry point of the module
                            NodeModulePath = realloc(NodeModulePath, strlen(NodeModulePath) + strlen(main_field->valuestring) + 2);
                            strcat(NodeModulePath, "/");
                            strcat(NodeModulePath, main_field->valuestring);

                            // Free allocated memory and set variables
                            free(PackageJSONContent);
                            free(PackageJSONPath);
                            free(IteratePointer->Text);
                            IteratePointer->Text = NodeModulePath;
                            LocalFileFound = true;
                            cJSON_Delete(PackageJSON);
                        }
                    }
                }
                if (!LocalFileFound) // Module cannot be found anywhere
                {
                    ThrowFatalError("Could not find module: %s\n", IteratePointer->Text);
                }
            }
        }
        IteratePointer++;
    }
    struct RegexMatch *ESDependencies = BasicRegexDependencies(filename, "import [^;]*from[^;]*;", 8, 0, CommentLocations);
    IteratePointer = &ESDependencies[0];
    while (IteratePointer->IsArrayEnd != true) // Loop over ES dependencies
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
            for (int i = FromLocation; i < strlen(IteratePointer->Text); i++) // Search for the start location
            {
                if (IteratePointer->Text[i] != ' ' && IteratePointer->Text[i] != '\'' && IteratePointer->Text[i] != '\"')
                {
                    StartLocation = i;
                    break;
                }
            }
            if (StartLocation != -1) // Check start location has been found
            {
                for (int i = StartLocation; i < strlen(IteratePointer->Text); i++) // Search for end location
                {
                    if (IteratePointer->Text[i] == ' ' || IteratePointer->Text[i] == '\'' || IteratePointer->Text[i] == '\"')
                    {
                        EndLocation = i - 1;
                        break;
                    }
                }
                if (EndLocation != -1) // Check end location has been found
                {
                    IteratePointer->Text = TurnToFullRelativePath(getSubstring(IteratePointer->Text, StartLocation, EndLocation), GetBasePath(filename));

                    if (!StringEndsWith(IteratePointer->Text, ".js")) // Make sure file name ends with .js
                    {
                        IteratePointer->Text = realloc(IteratePointer->Text, strlen(IteratePointer->Text) + 4);
                        strcat(IteratePointer->Text, ".js");
                    }
                }
            }
        }
        IteratePointer++;
    }
    CombineRegexMatchArrays(&CJSDependencies, &ESDependencies); // Combine the commonJS and ES dependency arrays together

    return CJSDependencies;
}