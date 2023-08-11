/* This file handles almost all of the bundling logic */
#include "BundleFiles.h"

/* Struct to hold details about an imported ESM module*/
struct ImportedESM
{
    char *name;
    char *alias;
    bool IsDefault;
    bool IsArrayEnd;
} ImportedESM;

/* Used for checking if a character is a valid character in a JS variable/function name */
int IsEndOfJSName(const char character)
{
    const char disallowed_chars[] = {' ', '!', '@', '#', '%', '^', '&', '*', '(', ')', '-', '+', '=', '{', '}', '[', ']', ':', ';', '"', '\'', '<', '>', ',', '.', '?', '/', '+', '-', '*', '/', '%', '+', '-', '\n'};
    const int array_size = sizeof(disallowed_chars) / sizeof(disallowed_chars[0]);
    for (int i = 0; i < array_size; i++)
    {
        if (disallowed_chars[i] == character)
        {
            return true;
        }
    }
    return false;
}

// Initialize variables
char *FileContents = NULL;
struct Edge *CurrentEdge = NULL;
struct ShiftLocation *ShiftLocations = NULL;
char *InsertText = NULL;
int ShiftLocationsLength = 0;
char *DependencyExitPath = NULL;

void BundleHTMLinHTML()
{
    FileContents = ReplaceSectionOfString(
        FileContents,
        GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
        GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations), InsertText); // Replace the <include> tag with the contents of the dependency

    AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - ((CurrentEdge->EndRefPos + 1) - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength); // Add the shift amount and location}
}

void BundleJSinHTML()
{
    char *ReferencedString = getSubstring(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations));
    if (!StringContainsSubstring(ReferencedString, " defer") && !StringContainsSubstring(ReferencedString, "async")) // Don't bundle asynchronously fetched scripts
    {

        int startlocation = -1;
        int endlocation = -1;
        int TempShiftedAmount = GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations);

        // Find the part of the <script> tag where the src attribute is
        for (int v = GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations); v < strlen(FileContents); v++)
        {
            if (strncasecmp(FileContents + v, "src", 3) == 0) // Find "src" attribute
            {
                startlocation = v;
                break;
            }
        }
        if (startlocation != -1) // Check "src" attribute has been found
        {

            endlocation = startlocation;
            bool pastEquals = false;
            bool pastText = false;
            for (int v = startlocation; v < strlen(FileContents); v++)
            {
                if (!pastEquals) // Search for '=' in src attribute
                {
                    if (FileContents[v] == '=') // Equals sign is found
                    {
                        pastEquals = true;
                    }
                }
                else if (!pastText) // Search for end of the script location
                {
                    if (FileContents[v] != '\'' && FileContents[v] != '\"' && FileContents[v] != ' ')
                    {
                        pastText = true;
                    }
                }
                else
                {
                    if (FileContents[v] == '\'' || FileContents[v] == '\"' || FileContents[v] == ' ') // Search for ending quote or space
                    {
                        endlocation = v + 1;
                        break;
                    }
                    else if (FileContents[v] == '>' || FileContents[v] == '\0') // Check for end of the element incase of no quotes being present
                    {
                        endlocation = v - 1;
                        break;
                    }
                }
            }
            RemoveSectionOfString(FileContents, startlocation, endlocation);                                                   // Remove the "src" attribute
            AddShiftNum(CurrentEdge->StartRefPos, ((endlocation)-startlocation) * -1, &ShiftLocations, &ShiftLocationsLength); // Add shift location from removing the src attribute

            // Remove the "export" attributes from the dependency
            InsertText = RemoveSubstring(InsertText, "export default ");
            InsertText = RemoveSubstring(InsertText, "export ");

            // Insert the dependency content into the HTML file in the script tag
            FileContents = InsertStringAtPosition(FileContents, InsertText, GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations));
            AddShiftNum(CurrentEdge->EndRefPos + 1, strlen(InsertText), &ShiftLocations, &ShiftLocationsLength);
        }
    }
}

void BundleCSSinHTML(struct Node *GraphNode)
{
    if (GetSetting("bundleCSSInHTML")->valueint == true) // CSS will only be bundled into HTML if the setting is active
    {
        char *InsertString;
        struct RegexMatch *StyleResults = GetAllRegexMatches(FileContents, "<style[^>]*>", 0, 0); // Check if a style tag already exists
        if (StyleResults[0].IsArrayEnd)                                                           // Style tag doesn't already exist
        {

            // Create new style tag
            InsertString = malloc(strlen(InsertText) + 18); // allocates space for start and end of <style> tag
            strcpy(InsertString, "<style>");
            strcat(InsertString, InsertText);
            strcat(InsertString, "</style>");

            struct RegexMatch *HeadTagResults = GetAllRegexMatches(FileContents, "< ?head[^>]*>", 0, 0); // Find head tag for where to place style tag
            if (HeadTagResults[0].IsArrayEnd == false)                                                   // If <head> tag is found
            {
                RemoveSectionOfString(
                    FileContents,
                    GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                    GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations));                                                                             // Remove original <link> tag that references the stylesheet file
                AddShiftNum(CurrentEdge->StartRefPos, ((CurrentEdge->EndRefPos + 1) - CurrentEdge->StartRefPos) * -1, &ShiftLocations, &ShiftLocationsLength); // Add shift location for link tag removal

                // Add style tag
                FileContents = InsertStringAtPosition(FileContents, InsertString, HeadTagResults[0].EndIndex);
                AddShiftNum(GetInverseShiftedAmount(HeadTagResults[0].EndIndex, ShiftLocations), strlen(InsertString), &ShiftLocations, &ShiftLocationsLength); // Add shift location for style tag
            }
            else
            {
                ColorYellow();
                printf("No <head> tag found for file: %s, unable to bundle CSS into file, file will still work\n", GraphNode->path);
                ColorReset();
            }
        }
        else
        {
            FileContents = InsertStringAtPosition(FileContents, InsertText, StyleResults[0].EndIndex); // Insert file dependency contents after style tag
        }
    }
}

void BundleCSSinCSS()
{
    int InsertEnd2 = CurrentEdge->EndRefPos + 1;
    FileContents = ReplaceSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                                          GetShiftedAmount(InsertEnd2, ShiftLocations), InsertText);                                             // Replace CSS import statement with dependency file content
    AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - (InsertEnd2 - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength); // Add the shift amount
}

void BundleJSinJS()
{
    char *ReferenceText = getSubstring(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                                       GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations)); // Gets the import statement used to import the dependency

    bool ISESModule = StringStartsWith(ReferenceText, "import "); //  Checks if the current dependency is an ESModule
    struct RegexMatch *IteratePointer;                            // Defines Iterate Pointer
    char *InsertText = ReadDataFromFile(DependencyExitPath);      // Reads the data for the current dependency
    struct RegexMatch *ExtraExportMatches;                        // Defines Extra Export Matches for commonJS modules to use
    struct RegexMatch *UsableExtraImports;                        // Defines usable extra imports needed for commonJS modules e.g "exports.HelloWorld"
    struct RegexMatch *FinalElement;
    struct RegexMatch *DefaultExport = NULL; // Defines default export for ES modules
    struct RegexMatch *Exports = NULL;       // Defines non-default exports for ES modules

    int FullExportsArrayLength = 0;
    char *NewModuleExportsName = NULL;
    int ImportedFunctionNameLength = 0;
    struct ImportedESM *ImportedFunctionNames = malloc(sizeof(struct ImportedESM)); // Used to keep track of imported functions tht need to have a unique name
    ImportedFunctionNames[0].IsArrayEnd = true;

    bool ImportDefault = false;                                                                // Tracks whether a default value is imported
    bool ImportNamed = false;                                                                  // Tracks whether a named value is imported
    bool ImportAll = false;                                                                    // Tracks whether all values are being imported
    bool ImportAllAlias = false;                                                               // Tracks whether all values are being imported with an alias
    struct RegexMatch *FunctionNames = GetAllRegexMatches(InsertText, "function [^(]*", 9, 1); // Find all function names
    struct RegexMatch *VariableName = GetAllRegexMatches(InsertText, "const [^;]*", 6, 1);     // Find all variable names

    IteratePointer = &FunctionNames[0];
    for (int i = 0; i < strlen(IteratePointer->Text); i++) // Get all function names on their own
    {
        if (IsEndOfJSName(IteratePointer->Text[i]))
        {
            IteratePointer->Text[i] = '\0';
            break;
        }
    }

    if (ISESModule) // If the current dependency is an ESModule then this code is run to find the imports
    {

        if (HasRegexMatch(ReferenceText, "import\\s+\\*")) // Detects if the import is importing all imports
        {
            ImportAll = true;
            if (HasRegexMatch(ReferenceText, "import\\s+\\*\\s+as")) // Checks if the import is using an alias
            {
                ImportAllAlias = true;
            }
        }
        else
        {
            if (LastOccurenceOfChar(ReferenceText, '{') != -1) // Checks if the import uses named imports
            {
                int EndLocation = LastOccurenceOfChar(ReferenceText, '}'); // Gets the end location of the named exports

                // Initialises variables
                bool InVariable = false;
                bool AfterAs = false;
                bool InAlias = false;

                int CurrentVariableStart = 0;

                for (int i = LastOccurenceOfChar(ReferenceText, '{'); i <= EndLocation; i++) // Loops through named imports to find the names and their aliases
                {
                    if (AfterAs) // Checks for the start of the alias if "as" has been found before
                    {
                        if (!IsEndOfJSName(ReferenceText[i])) // Start of the alias has been found
                        {
                            AfterAs = false;
                            InAlias = true;
                            CurrentVariableStart = i;
                        }
                    }
                    else if (InAlias) // Checks for end of alias if currently searching for end
                    {
                        if (IsEndOfJSName(ReferenceText[i])) // Check if at the end of the alias
                        {
                            ImportedFunctionNames[ImportedFunctionNameLength - 1].alias = getSubstring(ReferenceText, CurrentVariableStart, i - 1); // Set the ImportedESM alias to the alias found
                            InAlias = false;                                                                                                        // No longer in an alias statement
                        }
                    }
                    else if (!InVariable) // Checks if it has reached to start of a variable
                    {
                        if (!IsEndOfJSName(ReferenceText[i])) // A new variable has been found
                        {
                            InVariable = true;
                            CurrentVariableStart = i;
                            ImportedFunctionNameLength++;
                            ImportedFunctionNames = realloc(ImportedFunctionNames, sizeof(struct ImportedESM) * (ImportedFunctionNameLength + 1)); // Resive imported function struct to have space for
                            ImportedFunctionNames[ImportedFunctionNameLength].IsArrayEnd = true;                                                   // Used for end of array when iterating
                        }
                    }
                    else
                    {
                        if (IsEndOfJSName(ReferenceText[i]))
                        {
                            InVariable = false;
                            if (i - CurrentVariableStart == 2) // Checks for "as" to indicate an alias
                            {
                                if (ReferenceText[CurrentVariableStart] == 'a' && ReferenceText[CurrentVariableStart + 1] == 's') // detects if alias is used for previous variable
                                {
                                    AfterAs = true;
                                    InVariable = false;
                                    ImportedFunctionNameLength--; // Deincrement because "as" it not an imported function/value
                                }
                            }
                            if (!AfterAs) // Previous variable is an actual variable and not the "as" keyword
                            {
                                struct ImportedESM *FunctionName = &ImportedFunctionNames[ImportedFunctionNameLength - 1];
                                FunctionName->name = getSubstring(ReferenceText, CurrentVariableStart, i - 1); // Set function name
                                FunctionName->alias = NULL;                                                    // Set alias to NULL as it hasn't been found yet
                                ImportedFunctionNames[ImportedFunctionNameLength - 1].IsDefault = false;       // Sets variable as not being a default imported function
                                ImportedFunctionNames[ImportedFunctionNameLength - 1].IsArrayEnd = false;
                            }
                        }
                    }
                }
                ImportNamed = true;

                ImportDefault = HasRegexMatch(ReferenceText, "import[^,;]*,[^;]*{[^;]*}[^;]*;"); // Uses regex to find if a default export is imported aswell as named imports
            }
            else
            {
                ImportDefault = true;
            }

            if (ImportDefault) // If a default import is present then find it's name and alias
            {
                if (ImportDefault || ImportAll) // Default is also imported if all exports are imported
                {
                    DefaultExport = GetRegexMatch(InsertText, "export\\s+default"); // Find location of the default export
                    IteratePointer = &DefaultExport[0];
                }
                int StartLocation = -1;
                bool NameFound = false;
                bool AfterAs = false;
                int CheckEnd = LastOccurenceOfChar(ReferenceText, '{'); // Finds the end of where to check the default import
                if (CheckEnd == -1)
                {
                    CheckEnd = strlen(ReferenceText);
                }

                for (int i = 7; i < CheckEnd; i++) // Get default import and find name and alias
                {
                    if (NameFound) // Check if the name is found
                    {
                        if (AfterAs)
                        {
                            if (StartLocation == -1)
                            {
                                if (!IsEndOfJSName(ReferenceText[i])) // Finds the start of the name of the default import alias
                                {
                                    StartLocation = i;
                                }
                            }
                            else
                            {
                                if (IsEndOfJSName(ReferenceText[i])) // Finds the end of the name of the default import alias
                                {
                                    ImportedFunctionNames[ImportedFunctionNameLength - 1].alias = getSubstring(ReferenceText, StartLocation, i - 3);
                                }
                            }
                        }
                    }
                    else if (!IsEndOfJSName(ReferenceText[i]) && NameFound) // Checks for alias with default import
                    {
                        if (ReferenceText[i] == 'a' && ReferenceText[i + 1] == 's')
                        {
                            i++;
                            AfterAs = true;
                        }
                        else
                        {
                            break;
                        }
                    }
                    else if (StartLocation != -1) // Start location of default name/alias has not yet been found
                    {
                        if (IsEndOfJSName(ReferenceText[i]))
                        {
                            ImportedFunctionNameLength++;
                            ImportedFunctionNames = realloc(ImportedFunctionNames, sizeof(struct ImportedESM) * (ImportedFunctionNameLength + 1));
                            ImportedFunctionNames[ImportedFunctionNameLength].IsArrayEnd = true;
                            int DefaultExportNameEnd = strlen(InsertText);
                            for (int i = DefaultExport->EndIndex + 10; i < DefaultExportNameEnd; i++)
                            {
                                if (IsEndOfJSName(InsertText[i]))
                                {
                                    DefaultExportNameEnd = i - 1;
                                    break;
                                }
                            }
                            struct ImportedESM FunctionName = ImportedFunctionNames[ImportedFunctionNameLength - 1];
                            FunctionName.name = getSubstring(InsertText, DefaultExport->EndIndex + 10, DefaultExportNameEnd);
                            FunctionName.IsArrayEnd = false;
                            FunctionName.IsDefault = true;
                            FunctionName.alias = getSubstring(ReferenceText, StartLocation, i - 1);
                            NameFound = true;
                            StartLocation = -1;
                        }
                    }
                    else
                    {
                        if (!IsEndOfJSName(ReferenceText[i]))
                        {
                            StartLocation = i;
                        }
                    }
                }
            }
        }
        if (ImportNamed || ImportAll)
        {
            Exports = GetAllRegexMatches(InsertText, "export[^;]+;", 7, 0);
            IteratePointer = &Exports[0];
        }
        if ((ImportDefault && ImportNamed) || ImportAll)
        {
            CombineRegexMatchArrays(&DefaultExport, &Exports);
            IteratePointer = &DefaultExport[0];
        }

        struct RegexMatch *IteratePointer2;
        struct ImportedESM *ESMIteratePointer;
        while (IteratePointer->IsArrayEnd == false) // Removes export if it is a default export
        {
            ESMIteratePointer = &ImportedFunctionNames[0]; // Loops through functions that are being imported to remove uneeded exported functions
            if (!ImportAll)                                // Removes all exported functions that are not being imported so they can have name collisions fixed
            {
                bool FoundExport = false; // Tracks whether the function is being imported
                while (ESMIteratePointer->IsArrayEnd != true)
                {
                    if (StringContainsSubstring(IteratePointer->Text, ESMIteratePointer->name))
                    {
                        FoundExport = true;
                        break;
                    }
                    ESMIteratePointer++;
                }
                if (!FoundExport)
                {
                    RemoveRegexMatch(IteratePointer);
                    continue;
                }
            }

            IteratePointer2 = &FunctionNames[0];

            while (IteratePointer2->IsArrayEnd != true)
            {
                if (IteratePointer2->StartIndex > IteratePointer->StartIndex && IteratePointer2->StartIndex < IteratePointer->EndIndex)
                {
                    RemoveRegexMatch(IteratePointer2);
                }
                IteratePointer2++;
            }

            if (HasRegexMatch(IteratePointer->Text, "export\\s+default"))
            {
                RemoveRegexMatch(IteratePointer); // Doesn't break in the case of the JS being invalid and having 2 default exports
            }

            IteratePointer++;
        }
        IteratePointer = &FunctionNames[0]; // Set iteration pointer to the first function name to loop over later
    }
    else // If the current dependency is a CommonJS module then this code is run to find the exports
    {
        struct RegexMatch *FullExportMatches = GetAllRegexMatches(InsertText, "module\\.exports\\s*=\\s*[^;]*", 0, 0);
        FullExportsArrayLength = RegexMatchArrayLength(FullExportMatches);

        if (FullExportsArrayLength != 0)
        {
            FinalElement = &FullExportMatches[FullExportsArrayLength - 1];
        }
        ExtraExportMatches = GetAllRegexMatches(InsertText, "[^.]exports.[^;]*", 0, 0);
        UsableExtraImports = &ExtraExportMatches[0];
        if (FullExportsArrayLength > 0)
        {
            while (UsableExtraImports->IsArrayEnd != true)
            {
                if (UsableExtraImports->StartIndex < FinalElement->EndIndex)
                {
                    UsableExtraImports++;
                }
                else
                {
                    break;
                }
            }
        }

        NewModuleExportsName = malloc(CurrentEdge->EndRefPos - CurrentEdge->StartRefPos + 11);
        strcpy(NewModuleExportsName, getSubstring(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos + 9, ShiftLocations), GetShiftedAmount(CurrentEdge->EndRefPos - 2, ShiftLocations)));
        RemoveCharFromString(NewModuleExportsName, '/');
        strcat(NewModuleExportsName, "_ARROWPACK");

        RemoveCharFromString(NewModuleExportsName, '.');
        while (StringContainsSubstring(InsertText, NewModuleExportsName) || StringContainsSubstring(FileContents, NewModuleExportsName))
        {
            char *NewUniqueName = CreateUnusedName();
            NewModuleExportsName = realloc(NewModuleExportsName, (strlen(NewUniqueName) + strlen(NewModuleExportsName) + 1) * sizeof(char));
            strcat(NewModuleExportsName, NewUniqueName);
        }
    }
    // This code is run once the exports have been found
    struct ShiftLocation *JSFileShiftLocations = malloc(sizeof(ShiftLocation));
    JSFileShiftLocations[0].location = -1;
    int JSShiftLocationsLength = 1;

    while (IteratePointer->IsArrayEnd != true) // Solves name collisions
    {
        if (strlen(IteratePointer->Text) > 1) // Ignores unamed functions
        {

            bool InString = false;
            bool StringStartDoubleQuotes = false;
            bool FunctionDuplicateFound = false;
            for (int i = 0; i < strlen(FileContents) - strlen(IteratePointer->Text); i++) // Loop through each character in the file
            {
                if (FileContents[i] == '\'' || FileContents[i] == '\"') // Check for string quotes
                {
                    if (InString) // In string and string end has been found
                    {
                        if (StringStartDoubleQuotes) // Check if the quotes are the same as the start of the string
                        {
                            if (FileContents[i] == '\"')
                            {
                                InString = false; // End of string has been found
                            }
                        }
                        else
                        {
                            if (FileContents[i] == '\'')
                            {
                                InString = false; // End of string has been found
                            }
                        }
                    }
                    else
                    {
                        InString = true;                                   // Start of string has been found
                        StringStartDoubleQuotes = FileContents[i] == '\"'; // Track the quotes used for the start of the string
                    }
                }
                else if (strncmp(FileContents + i, IteratePointer->Text, strlen(IteratePointer->Text)) == 0 && !InString) // Check for a name collision
                {
                    FunctionDuplicateFound = true; // Name collision detected
                    break;
                }
            }
            if (FunctionDuplicateFound) // File contains at least one name collision
            {
                char *NewUnusedName = CreateUnusedName(); // Create a new unique name

                // Reset variables
                InString = false;
                StringStartDoubleQuotes = false;
                FunctionDuplicateFound = false;

                int LoopLength = strlen(InsertText) - strlen(IteratePointer->Text) + 2;
                for (int i = 0; i < LoopLength; i++) // Loop through the file
                {
                    if (InsertText[i] == '\'' || InsertText[i] == '\"') // Check not in a string
                    {
                        if (InString)
                        {
                            if (StringStartDoubleQuotes)
                            {
                                if (InsertText[i] == '\"')
                                {
                                    InString = false;
                                }
                            }
                            else
                            {
                                if (InsertText[i] == '\'')
                                {
                                    InString = false;
                                }
                            }
                        }
                        else
                        {
                            InString = true;
                            StringStartDoubleQuotes = InsertText[i] == '\"';
                        }
                    }
                    else if (strncmp(InsertText + i, IteratePointer->Text, strlen(IteratePointer->Text)) == 0 && !InString) // Replace name collision with replacement name
                    {
                        InsertText = InsertStringAtPosition(InsertText, NewUnusedName, i + strlen(IteratePointer->Text));
                        int InverseShiftAmount = GetInverseShiftedAmount(i + strlen(IteratePointer->Text), JSFileShiftLocations);
                        AddShiftNum(InverseShiftAmount, strlen(NewUnusedName), &JSFileShiftLocations, &JSShiftLocationsLength);
                    }
                }
            }
        }
        IteratePointer++;
    }
    // Remove export statements from dependency
    FileContents = RemoveSubstring(FileContents, "export default ");
    FileContents = RemoveSubstring(FileContents, "export ");

    if (ISESModule) // Bundles the files together if they are ES modules
    {

        struct ImportedESM *ESMIteratePointer = &ImportedFunctionNames[0];
        while (ESMIteratePointer->IsArrayEnd == false) // Loop through imported functions/variables
        {
            char *NewDefinition;
            if (ESMIteratePointer->alias != NULL || ESMIteratePointer->IsDefault) // Check if the function/variable has an alias
            {
                if (ESMIteratePointer->IsDefault)
                {
                    NewDefinition = malloc(sizeof(char) * strlen(ESMIteratePointer->alias) + strlen(ESMIteratePointer->name) + 7);
                }
                else
                {
                    NewDefinition = malloc(sizeof(char) * (10 + strlen(ESMIteratePointer->alias) + strlen(ESMIteratePointer->name)));
                }

                // Create alias for each function/variable
                strcpy(NewDefinition, "let ");
                NewDefinition = strcat(NewDefinition, ESMIteratePointer->alias);
                NewDefinition = strcat(NewDefinition, "=");
                NewDefinition = strcat(NewDefinition, ESMIteratePointer->name);
                NewDefinition = strcat(NewDefinition, ";");

                InsertText = realloc(InsertText, sizeof(char) * strlen(InsertText) + strlen(NewDefinition) + 1); // Adds space for new alias definition
                InsertText = strcat(InsertText, NewDefinition);                                                  // Adds alias definition to the end of the the dependency file content
            }

            ESMIteratePointer++;
        }
    }
    else // Bundles the files together if they are CommonJS modules
    {

        IteratePointer = &ExtraExportMatches[0];
        while (IteratePointer != UsableExtraImports && IteratePointer->IsArrayEnd == false)
        {
            RemoveSectionOfString(InsertText, IteratePointer->StartIndex, IteratePointer->EndIndex);
            AddShiftNum(IteratePointer->StartIndex, (IteratePointer->EndIndex - IteratePointer->StartIndex) * -1, &JSFileShiftLocations, &JSShiftLocationsLength);
            IteratePointer++;
        }

        // Create object definition for imported functions/variables for the imported module
        char *ModuleObjectDefinition = malloc(strlen(NewModuleExportsName) + 13);
        strcpy(ModuleObjectDefinition, "let ");
        strcat(ModuleObjectDefinition, NewModuleExportsName);
        strcat(ModuleObjectDefinition, " = {};");

        if (FullExportsArrayLength > 0) // Add the new module object definition
        {
            InsertText = InsertStringAtPosition(InsertText, ModuleObjectDefinition, GetShiftedAmount(FinalElement->StartIndex, JSFileShiftLocations));
            AddShiftNum(FinalElement->StartIndex, strlen(ModuleObjectDefinition), &JSFileShiftLocations, &JSShiftLocationsLength);
            InsertText = ReplaceSectionOfString(InsertText, GetShiftedAmount(FinalElement->StartIndex, JSFileShiftLocations), GetShiftedAmount(FinalElement->StartIndex, JSFileShiftLocations) + 14, NewModuleExportsName);
            AddShiftNum(FinalElement->StartIndex, strlen(NewModuleExportsName) - 14, &JSFileShiftLocations, &JSShiftLocationsLength);
        }
        else
        {
            InsertText = InsertStringAtPosition(InsertText, ModuleObjectDefinition, UsableExtraImports->StartIndex);
            AddShiftNum(UsableExtraImports->StartIndex, strlen(ModuleObjectDefinition), &JSFileShiftLocations, &JSShiftLocationsLength);
        }

        while (!UsableExtraImports->IsArrayEnd)
        {
            InsertText = ReplaceSectionOfString(InsertText, GetShiftedAmount(UsableExtraImports->StartIndex, JSFileShiftLocations), GetShiftedAmount(UsableExtraImports->StartIndex + 8, JSFileShiftLocations), NewModuleExportsName);
            AddShiftNum(UsableExtraImports->StartIndex, strlen(NewModuleExportsName) - 8, &JSFileShiftLocations, &JSShiftLocationsLength);
            UsableExtraImports++;
        }
    }

    if (GetSetting("productionMode")->valueint == false) // Keeps line numbers the same by turning new import into one line
    {
        RemoveSingleLineComments(InsertText);   // Remove single line comments
        RemoveCharFromString(InsertText, '\n'); // Makes everything one line
    }
    if (ISESModule)
    {
        RemoveSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                              GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations));                      // Remove import because exports are defined in dependency string
        AddShiftNum(CurrentEdge->StartRefPos, strlen(ReferenceText), &ShiftLocations, &ShiftLocationsLength); // Add shift location
    }
    else
    {
        FileContents = ReplaceSectionOfString(FileContents,
                                              GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                                              GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations) + 1, NewModuleExportsName); // Add object definition for imported functions/variables

        AddShiftNum(CurrentEdge->StartRefPos, strlen(NewModuleExportsName) - ((CurrentEdge->EndRefPos + 1) - CurrentEdge->StartRefPos),
                    &ShiftLocations, &ShiftLocationsLength); // Add shift location
    }
    FileContents = InsertStringAtPosition(FileContents, InsertText, 0);
    AddShiftNum(0, strlen(InsertText), &ShiftLocations, &ShiftLocationsLength);
    free(JSFileShiftLocations);
    JSFileShiftLocations = NULL;
}

/* Used to bundle a given node */
void BundleFile(struct Node *GraphNode)
{
    ShiftLocationsLength = 1;                       // Includes end element to signal the end of the array
    ShiftLocations = malloc(sizeof(ShiftLocation)); // Allocate shift locations
    ShiftLocations[0].location = -1;                // Indicates end of array although probably not needed because the length of the array is being stored

    int FileTypeID = GetFileTypeID(GraphNode->path); // Finds the file type

    FileContents = ReadDataFromFile(GraphNode->path);

    if (FileContents == NULL)
    {
        CreateWarning("Could not read file: %s\n", GraphNode->path);
        return;
    }
    CurrentEdge = GraphNode->edge;
    while (CurrentEdge != NULL) // Loop through each dependency
    {
        struct Node *CurrentDependency = CurrentEdge->vertex;
        ColorGreen();
        printf("Building file: %s\n", CurrentDependency->path);
        ColorNormal();
        DependencyExitPath = EntryToExitPath(CurrentDependency->path);
        InsertText = ReadDataFromFile(DependencyExitPath);          // The file contents of the dependency
        int DependencyFileType = GetFileTypeID(DependencyExitPath); // The file type of the dependency

        if (FileTypeID == HTMLFILETYPE_ID) // Current node is a HTML file
        {
            if (DependencyFileType == HTMLFILETYPE_ID) // Current dependency is HTML for a HTML file
            {
                BundleHTMLinHTML();
            }
            else if (DependencyFileType == CSSFILETYPE_ID) // Bundle CSS into HTML file
            {
                BundleCSSinHTML(GraphNode);
            }
            else if (DependencyFileType == JSFILETYPE_ID) // Bundle JS into HTML file
            {
            }
            else // Will hopefully work for most custom dependencies
            {

                int InsertEnd2 = CurrentEdge->EndRefPos + 1;
                FileContents = ReplaceSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(InsertEnd2, ShiftLocations), InsertText); // Replace reference location with dependency contents
                AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - (InsertEnd2 - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength);                               // Add shift location
            }
        }
        else if (FileTypeID == CSSFILETYPE_ID) // Node is a CSS file
        {
            if (DependencyFileType == CSSFILETYPE_ID) // Bundling CSS in a CSS file
            {
                BundleCSSinCSS();
            }
        }
        else if (FileTypeID == JSFILETYPE_ID) // File is a JS file
        {
            if (DependencyFileType == JSFILETYPE_ID) // Bundle JS in a JS file
            {
                BundleJSinJS();
            }
        }
        CurrentEdge = CurrentEdge->next; // Go to next dependency in the linked list
    }

    if (GraphNode->FileType == HTMLFILETYPE_ID) // Extra bundling code for HTML after dependency bundling
    {
        FileContents = RemoveSubstring(FileContents, "</include>");                                  // Remove closing include tags if present
        struct RegexMatch *HeadTagResults = GetAllRegexMatches(FileContents, "< ?head[^>]*>", 0, 0); // Get location of head tag
        if (!HeadTagResults[0].IsArrayEnd)                                                           // Check if head tag is present
        {
            if (GetSetting("addBaseTag")->valueint) // Add HTML <base> tag if setting is enabled
            {
                // Create base tag as string
                char *BasePath = GetBasePath(GraphNode->path);
                char *BaseTag = malloc(strlen(BasePath) + 18);
                strcpy(BaseTag, "<base href=\"/");
                strcat(BaseTag, BasePath + strlen(GetSetting("entry")->valuestring));
                strcat(BaseTag, "\">");

                FileContents = InsertStringAtPosition(FileContents, BaseTag, HeadTagResults[0].EndIndex); // Insert base tag into file contents

                free(BaseTag);
                free(BasePath);
            }
            if (GetSetting("faviconPath")->valuestring[0] != '\0' && GetSetting("faviconPath")->valuestring != NULL) // Check if a favicon path is provided
            {
                struct RegexMatch *FaviconTagResults = GetRegexMatch(FileContents, "<link\\s*rel\\s*=\"?icon\"?[^>]*");
                if (FaviconTagResults == NULL) // Check there in not already a favicon
                {
                    // Create favicon tag as string
                    char *FaviconLink = malloc(100 + strlen(GetSetting("faviconPath")->valuestring));
                    strcpy(FaviconLink, "<link rel=\"icon\" type=\"image/x-icon\" href=\"/");

                    if (StringStartsWith(GetSetting("faviconPath")->valuestring, GetSetting("entry")->valuestring)) // Check if favicon path includes entry path
                    {
                        strcat(FaviconLink, GetSetting("faviconPath")->valuestring + strlen(GetSetting("entry")->valuestring)); // Add favicon path without entry path
                    }
                    else
                    {
                        strcat(FaviconLink, GetSetting("faviconPath")->valuestring); // Add the whole favicon path because it doesn't contain the entry path
                    }
                    strcat(FaviconLink, "\">");
                    FileContents = InsertStringAtPosition(FileContents, FaviconLink, HeadTagResults[0].EndIndex); // Add favicon path to string
                }
                free(FaviconTagResults);
            }
        }
    }
    CreateFileWrite(EntryToExitPath(GraphNode->path), FileContents); // Saves final file contents
    free(FileContents);
    ColorGreen();
    printf("Finished bundling file:%s\n", GraphNode->path);
    ColorNormal();
    free(ShiftLocations);
}

/* Used to post process a given file */
void PostProcessFile(struct Node *node, struct Graph *graph)
{
    if (node->path == NULL) // Check the file path is not NULL
    {
        return;
    }
    struct ShiftLocation *shiftLocations = malloc(sizeof(struct ShiftLocation));
    int ShiftlocationLength = 1;
    shiftLocations[0].location = -1;
    char *ExitPath = EntryToExitPath(node->path);
    if (node->FileType == JSFILETYPE_ID) // Postprocessing for JS files
    {
        char *FileContents = ReadDataFromFile(ExitPath);
        if (FileContents == NULL)
        {
            return;
        }

        // Remove non-browser commonJS module.exports statements
        struct RegexMatch *FullExportMatches = GetAllRegexMatches(FileContents, "module\\.exports\\s*=\\s*[^;]*", 0, 0);
        struct RegexMatch *IteratePointer = &FullExportMatches[0];
        while (IteratePointer->IsArrayEnd == false)
        {
            RemoveSectionOfString(FileContents, GetShiftedAmount(IteratePointer->StartIndex, shiftLocations), GetShiftedAmount(IteratePointer->EndIndex, shiftLocations) + 1);
            AddShiftNum(IteratePointer->StartIndex, IteratePointer->EndIndex - IteratePointer->StartIndex - 1, &shiftLocations, &ShiftlocationLength);
            IteratePointer++;
        }

        // Remove non-browser commonJS exports. statements
        struct RegexMatch *SmallExportMatches = GetAllRegexMatches(FileContents, "[^.]exports.[^;]*", 0, 0);
        IteratePointer = &SmallExportMatches[0];
        while (IteratePointer->IsArrayEnd == false)
        {
            RemoveSectionOfString(FileContents, GetShiftedAmount(IteratePointer->StartIndex, shiftLocations), GetShiftedAmount(IteratePointer->EndIndex, shiftLocations) + 1);
            AddShiftNum(IteratePointer->StartIndex, IteratePointer->EndIndex - IteratePointer->StartIndex - 1, &shiftLocations, &ShiftlocationLength);
            IteratePointer++;
        }

        // Remove export keywords from final JS files
        FileContents = RemoveSubstring(FileContents, "export default ");
        FileContents = RemoveSubstring(FileContents, "export ");

        CreateFileWrite(ExitPath, FileContents); // Save the final file
    }
    free(shiftLocations);
    shiftLocations = NULL;
}

/* Main code for bundling the files in a given dependency graph */
bool EMSCRIPTEN_KEEPALIVE BundleFiles(struct Graph *graph)
{
    bool Success = true;

    for (int FilesBundled = 0; FilesBundled < graph->VerticesNum; FilesBundled++) // Loops through every node (file)
    {
        struct Node *FileNode = graph->SortedArray[FilesBundled];
        if (count_edges(FileNode) == 0) // Check if node doesn't have an dependencies
        {
            char *TempExitPath = strdup(FileNode->path);
            char *ExitPath = EntryToExitPath(TempExitPath); // Get exit path of file
            free(TempExitPath);

            CopyFile(FileNode->path, ExitPath); // Copy file with no dependencies to exit path
            free(ExitPath);
        }
        else // Node has dependencies
        {
            struct Node *FileNode = graph->SortedArray[FilesBundled];
            ColorMagenta();
            printf("\nBundling file: %s\n", FileNode->path);
            ColorReset();

            BundleFile(FileNode); // Bundle the file
        }
        graph->SortedArray[FilesBundled]->Bundled = true; // Mark the file as bundled
    }

    // PostProcess functions that run after main bundling process
    for (int i = 0; i < graph->VerticesNum; i++)
    {
        PostProcessFile(graph->SortedArray[i], graph);
    }
    ColorNormal(); // Resets colours incase of a colour not being previously reset and messing up the user's terminal
    IsNodeBuiltin(NULL);
    return Success; // This is always true currently
}