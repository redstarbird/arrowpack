#include "BundleFiles.h"

struct ImportedESM
{
    char *name;
    char *alias;
    bool IsDefault;
    bool IsArrayEnd;
} ImportedESM;

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

void BundleFile(struct Node *GraphNode)
{
    int ShiftLocationsLength = 1; // Includes end element to signal the end of the array
    struct ShiftLocation *ShiftLocations = malloc(sizeof(ShiftLocation));
    ShiftLocations[0].location = -1; // Indicates end of array although probably not needed because the length of the array is being stored

    int FileTypeID = GetFileTypeID(GraphNode->path);

    char *FileContents = ReadDataFromFile(GraphNode->path);

    if (FileContents == NULL)
    {
        return;
    }
    struct Edge *CurrentEdge = GraphNode->edge;
    while (CurrentEdge != NULL)
    {
        struct Node *CurrentDependency = CurrentEdge->vertex;
        ColorGreen();
        printf("Building file: %s\n", CurrentDependency->path);
        ColorNormal();
        char *DependencyExitPath = EntryToExitPath(CurrentDependency->path);
        char *InsertText = ReadDataFromFile(DependencyExitPath);
        int DependencyFileType = GetFileTypeID(DependencyExitPath);

        if (FileTypeID == HTMLFILETYPE_ID)
        {
            if (DependencyFileType == HTMLFILETYPE_ID)
            {

                FileContents = ReplaceSectionOfString(
                    FileContents,
                    GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                    GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations), InsertText);
                // totalAmountShifted += strlen(InsertText) - (InsertEnd - GraphNode->Dependencies[i].StartRefPos);

                AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - ((CurrentEdge->EndRefPos + 1) - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength);
            }
            else if (DependencyFileType == CSSFILETYPE_ID) // Bundle CSS into HTML file
            {
                if (Settings.bundleCSSInHTML == true)
                {
                    char *InsertString;
                    struct RegexMatch *StyleResults = GetAllRegexMatches(FileContents, "<style[^>]*>", 0, 0);
                    if (StyleResults[0].IsArrayEnd)
                    {
                        // Style tag doesn't already exist
                        InsertString = malloc(strlen(InsertText) + 18); // allocates space for start and end of <style> tag
                        strcpy(InsertString, "<style>");
                        strcat(InsertString, InsertText);
                        strcat(InsertString, "</style>");
                        struct RegexMatch *HeadTagResults = GetAllRegexMatches(FileContents, "< ?head[^>]*>", 0, 0);
                        if (HeadTagResults[0].IsArrayEnd == false) // If <head> tag is found
                        {

                            RemoveSectionOfString(
                                FileContents,
                                GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                                GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations));
                            // totalAmountShifted -= (GraphNode->Dependencies[i].EndRefPos - GraphNode->Dependencies[i].StartRefPos + 1);
                            AddShiftNum(CurrentEdge->StartRefPos, ((CurrentEdge->EndRefPos + 1) - CurrentEdge->StartRefPos) * -1, &ShiftLocations, &ShiftLocationsLength);
                            FileContents = InsertStringAtPosition(FileContents, InsertString, HeadTagResults[0].EndIndex);
                            AddShiftNum(GetInverseShiftedAmount(HeadTagResults[0].EndIndex, ShiftLocations), strlen(InsertString), &ShiftLocations, &ShiftLocationsLength);
                            // totalAmountShifted += strlen(InsertString);
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
                        FileContents = InsertStringAtPosition(FileContents, InsertText, StyleResults[0].EndIndex);
                    }
                }
            }
            else if (DependencyFileType == JSFILETYPE_ID)
            {

                char *ReferencedString = getSubstring(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations));
                if (!StringContainsSubstring(ReferencedString, " defer") && !StringContainsSubstring(ReferencedString, "async"))
                {

                    int startlocation = -1;
                    int endlocation = -1;
                    int TempShiftedAmount = GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations);
                    for (int v = GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations); v < strlen(FileContents); v++)
                    {

                        if (strncasecmp(FileContents + v, "src", 3) == 0)
                        {
                            startlocation = v;
                            break;
                        }
                    }
                    if (startlocation != -1)
                    {

                        endlocation = startlocation;
                        bool pastEquals = false;
                        bool pastText = false;
                        for (int v = startlocation; v < strlen(FileContents); v++)
                        {
                            if (!pastEquals)
                            {
                                if (FileContents[v] == '=')
                                {
                                    pastEquals = true;
                                }
                            }
                            else if (!pastText)
                            {
                                if (FileContents[v] != '\'' && FileContents[v] != '\"' && FileContents[v] != ' ')
                                {
                                    pastText = true;
                                }
                            }
                            else
                            {
                                if (FileContents[v] == '\'' || FileContents[v] == '\"' || FileContents[v] == ' ')
                                {
                                    endlocation = v + 1;
                                    break;
                                }
                                else if (FileContents[v] == '>' || FileContents[v] == '\0')
                                {
                                    endlocation = v - 1;
                                    break;
                                }
                            }
                        }
                        RemoveSectionOfString(FileContents, startlocation, endlocation);
                        AddShiftNum(CurrentEdge->StartRefPos, ((endlocation)-startlocation) * -1, &ShiftLocations, &ShiftLocationsLength);
                        InsertText = RemoveSubstring(InsertText, "export default ");
                        InsertText = RemoveSubstring(InsertText, "export ");
                        FileContents = InsertStringAtPosition(FileContents, InsertText, GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations));
                        AddShiftNum(CurrentEdge->EndRefPos + 1, strlen(InsertText), &ShiftLocations, &ShiftLocationsLength);
                    }
                }
            }
            else // Will hopefully work for most custom dependencies
            {
                char *InsertText = ReadDataFromFile(DependencyExitPath);
                int InsertEnd2 = CurrentEdge->EndRefPos + 1;
                FileContents = ReplaceSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(InsertEnd2, ShiftLocations), InsertText);
                AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - (InsertEnd2 - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength);
            }
        }
        else if (FileTypeID == CSSFILETYPE_ID)
        {
            if (DependencyFileType == CSSFILETYPE_ID)
            {
                char *InsertText = ReadDataFromFile(DependencyExitPath);
                int InsertEnd2 = CurrentEdge->EndRefPos + 1;
                FileContents = ReplaceSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(InsertEnd2, ShiftLocations), InsertText);

                AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - (InsertEnd2 - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength);
            }
        }
        else if (FileTypeID == JSFILETYPE_ID)
        {
            if (DependencyFileType == JSFILETYPE_ID)
            {
                char *ReferenceText = getSubstring(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations));
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
                bool ImportDefault = false;
                bool ImportNamed = false;
                bool ImportAll = false;
                bool ImportAllAlias = false; // Tracks how/what needs to be imported for ES modules
                struct RegexMatch *FunctionNames = GetAllRegexMatches(InsertText, "function [^(]*", 9, 1);
                struct RegexMatch *VariableName = GetAllRegexMatches(InsertText, "const [^;]*", 6, 1);
                IteratePointer = &FunctionNames[0];

                for (int i = 0; i < strlen(IteratePointer->Text); i++)
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
                            bool InVariable = false;
                            bool AfterAs = false;
                            bool InAlias = false; // Initialises variables

                            int CurrentVariableStart = 0;

                            for (int i = LastOccurenceOfChar(ReferenceText, '{'); i <= EndLocation; i++) // Loops through named imports to find the names and their aliases
                            {
                                if (AfterAs) // Checks for the start of the alias if "as" has been found before
                                {
                                    if (!IsEndOfJSName(ReferenceText[i]))
                                    {
                                        AfterAs = false;
                                        InAlias = true;
                                        CurrentVariableStart = i;
                                    }
                                }
                                else if (InAlias) // Checks for end of alias if currently searching for end
                                {
                                    if (IsEndOfJSName(ReferenceText[i]))
                                    {
                                        ImportedFunctionNames[ImportedFunctionNameLength - 1].alias = getSubstring(ReferenceText, CurrentVariableStart, i - 1);
                                        InAlias = false;
                                    }
                                }
                                else if (!InVariable) // Checks if it has reached to start of a variable
                                {
                                    if (!IsEndOfJSName(ReferenceText[i]))
                                    {
                                        InVariable = true;
                                        CurrentVariableStart = i;
                                        ImportedFunctionNameLength++;
                                        ImportedFunctionNames = realloc(ImportedFunctionNames, sizeof(struct ImportedESM) * (ImportedFunctionNameLength + 1)); // Adds extra space for end struct
                                        ImportedFunctionNames[ImportedFunctionNameLength].IsArrayEnd = true;                                                   // Used for end of array when iterating
                                    }
                                }
                                else
                                {
                                    if (IsEndOfJSName(ReferenceText[i]))
                                    {
                                        InVariable = false;
                                        // ImportedFunctionNames[ImportedFunctionNameLength] = malloc(sizeof(char) * (2 + i - CurrentVariableStart));
                                        if (i - CurrentVariableStart == 2) // Checks for "as" to indicate an alias
                                        {
                                            if (ReferenceText[CurrentVariableStart] == 'a' && ReferenceText[CurrentVariableStart + 1] == 's') // detects if alias is used for previous variable
                                            {
                                                AfterAs = true;
                                                InVariable = false;
                                                ImportedFunctionNameLength--;
                                            }
                                        }
                                        if (!AfterAs)
                                        {
                                            ImportedFunctionNames[ImportedFunctionNameLength - 1].name = getSubstring(ReferenceText, CurrentVariableStart, i - 1);
                                            ImportedFunctionNames[ImportedFunctionNameLength - 1].alias = NULL;
                                            ImportedFunctionNames[ImportedFunctionNameLength - 1].IsDefault = false;
                                            ImportedFunctionNames[ImportedFunctionNameLength - 1].IsArrayEnd = false;
                                        }
                                    }
                                }
                            }
                            ImportNamed = true;

                            ImportDefault = HasRegexMatch(ReferenceText, "import[^,;]*,[^;]*{[^;]*}[^;]*;");
                        }
                        else
                        {
                            ImportDefault = true;
                        }

                        if (ImportDefault) // If a default import is present then find it's name and alias
                        {
                            if (ImportDefault || ImportAll)
                            {
                                DefaultExport = GetRegexMatch(InsertText, "export\\s+default");
                                IteratePointer = &DefaultExport[0];
                            }
                            int StartLocation = -1;
                            bool NameFound = false;
                            bool AfterAs = false;
                            int CheckEnd = LastOccurenceOfChar(ReferenceText, '{');
                            if (CheckEnd == -1)
                            {
                                CheckEnd = strlen(ReferenceText);
                            }

                            for (int i = 7; i < CheckEnd; i++)
                            {
                                if (NameFound)
                                {
                                    if (AfterAs)
                                    {
                                        if (StartLocation == -1)
                                        {
                                            if (!IsEndOfJSName(ReferenceText[i]))
                                            {
                                                StartLocation = i;
                                            }
                                        }
                                        else
                                        {
                                            if (IsEndOfJSName(ReferenceText[i]))
                                            {
                                                ImportedFunctionNames[ImportedFunctionNameLength - 1].alias = getSubstring(ReferenceText, StartLocation, i - 3);
                                            }
                                        }
                                    }
                                }
                                else if (!IsEndOfJSName(ReferenceText[i]) && NameFound)
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
                                else if (StartLocation != -1)
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

                                        ImportedFunctionNames[ImportedFunctionNameLength - 1].name = getSubstring(InsertText, DefaultExport->EndIndex + 10, DefaultExportNameEnd);
                                        ImportedFunctionNames[ImportedFunctionNameLength - 1].IsArrayEnd = false;
                                        ImportedFunctionNames[ImportedFunctionNameLength - 1].IsDefault = true;
                                        ImportedFunctionNames[ImportedFunctionNameLength - 1].alias = getSubstring(ReferenceText, StartLocation, i - 1);
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
                    IteratePointer = &FunctionNames[0];
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
                        for (int i = 0; i < strlen(FileContents) - strlen(IteratePointer->Text); i++)
                        {
                            if (FileContents[i] == '\'' || FileContents[i] == '\"')
                            {
                                if (InString)
                                {
                                    if (StringStartDoubleQuotes)
                                    {
                                        if (FileContents[i] == '\"')
                                        {
                                            InString = false;
                                        }
                                    }
                                    else
                                    {
                                        if (FileContents[i] == '\'')
                                        {
                                            InString = false;
                                        }
                                    }
                                }
                                else
                                {
                                    InString = true;
                                    StringStartDoubleQuotes = FileContents[i] == '\"';
                                }
                            }
                            else if (strncmp(FileContents + i, IteratePointer->Text, strlen(IteratePointer->Text)) == 0 && !InString)
                            {
                                ColorGreen();
                                printf("Resolving name collision: %s\n", FileContents + i);
                                ColorNormal();
                                FunctionDuplicateFound = true;
                                break;
                            }
                        }
                        if (FunctionDuplicateFound)
                        {
                            char *NewUnusedName = CreateUnusedName();
                            InString = false;
                            StringStartDoubleQuotes = false;
                            FunctionDuplicateFound = false;
                            int LoopLength = strlen(InsertText) - strlen(IteratePointer->Text) + 2;
                            for (int i = 0; i < LoopLength; i++)
                            {
                                if (InsertText[i] == '\'' || InsertText[i] == '\"')
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
                                else if (strncmp(InsertText + i, IteratePointer->Text, strlen(IteratePointer->Text)) == 0 && !InString)
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
                FileContents = RemoveSubstring(FileContents, "export default ");
                FileContents = RemoveSubstring(FileContents, "export ");
                if (ISESModule) // Bundles the files together if they are ES modules
                {

                    struct ImportedESM *ESMIteratePointer = &ImportedFunctionNames[0];
                    while (ESMIteratePointer->IsArrayEnd == false)
                    {
                        char *NewDefinition;
                        if (ESMIteratePointer->alias != NULL || ESMIteratePointer->IsDefault)
                        {
                            if (ESMIteratePointer->IsDefault)
                            {
                                NewDefinition = malloc(sizeof(char) * strlen(ESMIteratePointer->alias) + strlen(ESMIteratePointer->name) + 7);
                            }
                            else
                            {
                                NewDefinition = malloc(sizeof(char) * (10 + strlen(ESMIteratePointer->alias) + strlen(ESMIteratePointer->name)));
                            }

                            strcpy(NewDefinition, "let ");
                            NewDefinition = strcat(NewDefinition, ESMIteratePointer->alias);
                            NewDefinition = strcat(NewDefinition, "=");
                            NewDefinition = strcat(NewDefinition, ESMIteratePointer->name);
                            NewDefinition = strcat(NewDefinition, ";");
                            InsertText = realloc(InsertText, sizeof(char) * strlen(InsertText) + strlen(NewDefinition) + 1);
                            InsertText = strcat(InsertText, NewDefinition);
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

                    char *ModuleObjectDefinition = malloc(strlen(NewModuleExportsName) + 13);
                    strcpy(ModuleObjectDefinition, "let ");
                    strcat(ModuleObjectDefinition, NewModuleExportsName);
                    strcat(ModuleObjectDefinition, " = {};");
                    if (FullExportsArrayLength > 0)
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
                if (Settings.productionMode == false) // Keeps line numbers the same by turning new import into one line
                {
                    RemoveSingleLineComments(InsertText);
                    RemoveCharFromString(InsertText, '\n');
                }
                if (ISESModule)
                {
                    RemoveSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations));
                    AddShiftNum(CurrentEdge->StartRefPos, strlen(ReferenceText), &ShiftLocations, &ShiftLocationsLength);
                }
                else
                {
                    FileContents = ReplaceSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations) + 1, NewModuleExportsName);
                    AddShiftNum(CurrentEdge->StartRefPos, strlen(NewModuleExportsName) - ((CurrentEdge->EndRefPos + 1) - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength);
                }
                FileContents = InsertStringAtPosition(FileContents, InsertText, 0);
                AddShiftNum(0, strlen(InsertText), &ShiftLocations, &ShiftLocationsLength);
                free(JSFileShiftLocations);
                JSFileShiftLocations = NULL;
            }
        }
        CurrentEdge = CurrentEdge->next;
    }

    if (GraphNode->FileType == HTMLFILETYPE_ID)
    {
        FileContents = RemoveSubstring(FileContents, "</include>");
    }
    CreateFileWrite(EntryToExitPath(GraphNode->path), FileContents); // Saves final file contents
    free(FileContents);
    ColorGreen();
    printf("Finished bundling file:%s\n", GraphNode->path);
    ColorNormal();
    free(ShiftLocations);
    // ShiftLocations = NULL;
}

void PostProcessFile(struct Node *node, struct Graph *graph)
{
    if (node->path == NULL)
    {
        return;
    }
    struct ShiftLocation *shiftLocations = malloc(sizeof(struct ShiftLocation));
    int ShiftlocationLength = 1;
    shiftLocations[0].location = -1;
    char *ExitPath = EntryToExitPath(node->path);
    if (node->FileType == JSFILETYPE_ID)
    {
        char *FileContents = ReadDataFromFile(ExitPath);
        if (FileContents == NULL)
        {
            return;
        }
        struct RegexMatch *FullExportMatches = GetAllRegexMatches(FileContents, "module\\.exports\\s*=\\s*[^;]*", 0, 0);
        struct RegexMatch *IteratePointer = &FullExportMatches[0];
        while (IteratePointer->IsArrayEnd == false)
        {
            RemoveSectionOfString(FileContents, GetShiftedAmount(IteratePointer->StartIndex, shiftLocations), GetShiftedAmount(IteratePointer->EndIndex, shiftLocations) + 1);
            AddShiftNum(IteratePointer->StartIndex, IteratePointer->EndIndex - IteratePointer->StartIndex - 1, &shiftLocations, &ShiftlocationLength);
            IteratePointer++;
        }
        struct RegexMatch *SmallExportMatches = GetAllRegexMatches(FileContents, "[^.]exports.[^;]*", 0, 0);
        IteratePointer = &SmallExportMatches[0];
        while (IteratePointer->IsArrayEnd == false)
        {
            RemoveSectionOfString(FileContents, GetShiftedAmount(IteratePointer->StartIndex, shiftLocations), GetShiftedAmount(IteratePointer->EndIndex, shiftLocations) + 1);
            AddShiftNum(IteratePointer->StartIndex, IteratePointer->EndIndex - IteratePointer->StartIndex - 1, &shiftLocations, &ShiftlocationLength);
            IteratePointer++;
        }
        FileContents = RemoveSubstring(FileContents, "export default ");
        FileContents = RemoveSubstring(FileContents, "export ");
        ColorGreen();
        printf("Post processed: %s", FileContents);
        ColorNormal();
        CreateFileWrite(ExitPath, FileContents);
    }
    free(shiftLocations);
    shiftLocations = NULL;
}

bool EMSCRIPTEN_KEEPALIVE BundleFiles(struct Graph *graph)
{
    bool Success = true;
    int FilesBundled;

    for (FilesBundled = 0; FilesBundled < graph->VerticesNum; FilesBundled++)
    {
        struct Node *FileNode = graph->SortedArray[FilesBundled];
        if (count_edges(FileNode) == 0)
        {
            char *TempExitPath = strdup(FileNode->path);
            char *ExitPath = EntryToExitPath(TempExitPath);
            free(TempExitPath);

            CopyFile(FileNode->path, ExitPath);
            free(ExitPath);
        }
        else
        {
            struct Node *FileNode = graph->SortedArray[FilesBundled];
            ColorMagenta();
            printf("\nBundling file: %s\n", FileNode->path);
            ColorReset();
            // print_progress_bar(i, graph->VerticesNum);
            BundleFile(FileNode);
        }
        graph->SortedArray[FilesBundled]->Bundled = true;
    }

    // PostProcess functions that run after main bundling process
    for (int i = 0; i < graph->VerticesNum; i++)
    {
        PostProcessFile(graph->SortedArray[i], graph);
    }
    ColorNormal();  // Need to find out where everything is being turned green
    return Success; // This is always true currently
}