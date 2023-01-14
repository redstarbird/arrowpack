#include "BundleFiles.h"

typedef struct ShiftLocation
{
    int location, ShiftNum;
} ShiftLocation;

static int GetShiftedAmount(int Location, struct ShiftLocation *ShiftLocations)
{
    unsigned int i = 0;
    unsigned int ShiftNum = 0;
    while (ShiftLocations[i].location != -1)
    {
        if (ShiftLocations[i].location <= Location)
        {
            ShiftNum += ShiftLocations[i].ShiftNum;
            i++;
        }
        else
        {
            break;
        }
    }

    return ShiftNum + Location;
}

static int GetInverseShiftedAmount(int Location, struct ShiftLocation *ShiftLocations)
{
    unsigned int i = 0;
    unsigned int ShiftNum = 0;
    while (ShiftLocations[i].location != -1 && ShiftLocations[i].location <= Location)
    {
        if (ShiftNum + ShiftLocations[i].ShiftNum > Location)
        {
            break;
        }
        else
        {
            ShiftNum += ShiftLocations[i].ShiftNum;
        }
    }
    return Location - ShiftNum;
}

static void AddShiftNum(int Location, int ShiftNum, struct ShiftLocation **ShiftLocations, int *ShiftLocationLength)
{
    (*ShiftLocationLength)++;
    printf("shiftlocations:%i\n", *ShiftLocationLength);
    struct ShiftLocation *NewShiftLocations = malloc(((*ShiftLocationLength) + 1) * sizeof(struct ShiftLocation));
    memcpy(NewShiftLocations, *ShiftLocations, (*ShiftLocationLength) * sizeof(struct ShiftLocation));
    printf("about to free\n");
    free(*ShiftLocations);
    printf("freed\n");
    *ShiftLocations = NewShiftLocations;
    unsigned int i = 0;
    while (1)
    {
        if ((*ShiftLocations)[i].location >= Location)
        {
            for (int v = (*ShiftLocationLength); v > i; v--) // Find where to place element so that list is ordered (should probably change to binary search)
            {
                (*ShiftLocations)[v] = (*ShiftLocations)[v - 1];
            }
            (*ShiftLocations)[i].location = Location;
            (*ShiftLocations)[i].ShiftNum = ShiftNum;
            (*ShiftLocations)[(*ShiftLocationLength)].location = -1;
            break;
        }
        else if ((*ShiftLocations)[i].location == -1)
        {
            (*ShiftLocations)[i].location = Location;
            (*ShiftLocations)[i].ShiftNum = ShiftNum;
            (*ShiftLocations)[i + 1].location = -1;
            break;
        }
        i++;
    }
    printf("Reached end of function\n");
}

void BundleFile(struct Node *GraphNode)
{
    int ShiftLocationsLength = 1; // Includes end element to signal the end of the array
    struct ShiftLocation *ShiftLocations = malloc(sizeof(ShiftLocation));
    ShiftLocations[0].location = -1; // Indicates end of array although probably not needed because the length of the array is being stored

    int FileTypeID = GetFileTypeID(GraphNode->path);

    char *FileContents = ReadDataFromFile(GraphNode->path);

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
                int InsertEnd = CurrentEdge->EndRefPos + 1;
                FileContents = ReplaceSectionOfString(
                    FileContents,
                    GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                    GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations), InsertText);
                // totalAmountShifted += strlen(InsertText) - (InsertEnd - GraphNode->Dependencies[i].StartRefPos);
                AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - (InsertEnd - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength);
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
                        InsertString = malloc(strlen(InsertText) + 16); // allocates space for start and end of <style> tag
                        strcpy(InsertString, "<style>");
                        strcpy(InsertString + 7, InsertText);
                        strcat(InsertString, "</style>");
                        struct RegexMatch *HeadTagResults = GetAllRegexMatches(FileContents, "< ?head[^>]*>", 0, 0);
                        if (HeadTagResults[0].IsArrayEnd == false) // If <head> tag is found
                        {

                            RemoveSectionOfString(
                                FileContents,
                                GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                                GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations) + 1);
                            // totalAmountShifted -= (GraphNode->Dependencies[i].EndRefPos - GraphNode->Dependencies[i].StartRefPos + 1);
                            AddShiftNum(CurrentEdge->StartRefPos, (CurrentEdge->EndRefPos - CurrentEdge->StartRefPos + 1) * -1, &ShiftLocations, &ShiftLocationsLength);
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
                        AddShiftNum(CurrentEdge->StartRefPos, (endlocation - startlocation) * -1, &ShiftLocations, &ShiftLocationsLength);
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
                char *InsertText = ReadDataFromFile(DependencyExitPath);
                struct RegexMatch *FullExportMatches = GetAllRegexMatches(InsertText, "module\\.exports\\s*=\\s*[^;]*", 0, 0);
                int FullExportsArrayLength = RegexMatchArrayLength(FullExportMatches);

                struct RegexMatch *FinalElement;

                if (FullExportsArrayLength != 0)
                {
                    FinalElement = &FullExportMatches[FullExportsArrayLength - 1];
                    printf("Final element num: %i, path: %s\n", FullExportsArrayLength - 1, FinalElement->Text);
                }
                printf("here\n");
                struct RegexMatch *ExtraExportMatches = GetAllRegexMatches(InsertText, "[^.]exports.[^;]*", 0, 0);
                struct RegexMatch *UsableExtraImports = &ExtraExportMatches[0];
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

                char *NewModuleExportsName = malloc(CurrentEdge->EndRefPos - CurrentEdge->StartRefPos + 11);
                strcpy(NewModuleExportsName, getSubstring(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos + 9, ShiftLocations), GetShiftedAmount(CurrentEdge->EndRefPos - 2, ShiftLocations)));
                RemoveCharFromString(NewModuleExportsName, '/');
                strcat(NewModuleExportsName, "_ARROWPACK");

                RemoveCharFromString(NewModuleExportsName, '.');
                struct ShiftLocation *JSFileShiftLocations = malloc(sizeof(ShiftLocation));
                JSFileShiftLocations[0].location = -1;
                int JSShiftLocationsLength = 1;
                struct RegexMatch *IteratePointer = &ExtraExportMatches[0];
                printf("Insert text:%s\n", InsertText);
                while (IteratePointer != UsableExtraImports && IteratePointer->IsArrayEnd == false)
                {
                    printf("test\n");
                    RemoveSectionOfString(InsertText, IteratePointer->StartIndex, IteratePointer->EndIndex);
                    AddShiftNum(IteratePointer->StartIndex, (IteratePointer->EndIndex - IteratePointer->StartIndex) * -1, &JSFileShiftLocations, &JSShiftLocationsLength);
                    IteratePointer++;
                }
                printf("\n\nInsert text:%s\n\n\n\n", InsertText);
                char *ModuleObjectDefinition = malloc(strlen(NewModuleExportsName) + 13);
                strcpy(ModuleObjectDefinition, "let ");
                strcat(ModuleObjectDefinition, NewModuleExportsName);
                strcat(ModuleObjectDefinition, " = {};");
                printf("Module definition %s\n", ModuleObjectDefinition);
                if (FullExportsArrayLength > 0)
                {
                    InsertText = InsertStringAtPosition(InsertText, ModuleObjectDefinition, FinalElement->StartIndex);
                    AddShiftNum(FinalElement->StartIndex, strlen(ModuleObjectDefinition), &JSFileShiftLocations, &JSShiftLocationsLength);
                    InsertText = ReplaceSectionOfString(InsertText, GetShiftedAmount(FinalElement->StartIndex, JSFileShiftLocations), GetShiftedAmount(FinalElement->StartIndex, JSFileShiftLocations) + 14, NewModuleExportsName);
                    AddShiftNum(FinalElement->StartIndex, strlen(NewModuleExportsName) - 14, &JSFileShiftLocations, &ShiftLocationsLength);
                    printf("is this coorrect int: %i, Not shifted: %i\n", GetShiftedAmount(FinalElement->StartIndex, JSFileShiftLocations), FinalElement->StartIndex);
                }
                else
                {
                    InsertText = InsertStringAtPosition(InsertText, ModuleObjectDefinition, UsableExtraImports->StartIndex);
                    AddShiftNum(UsableExtraImports->StartIndex, strlen(ModuleObjectDefinition), &JSFileShiftLocations, &JSShiftLocationsLength);
                }

                printf("\n\ntest:%s\n\n\n\n", InsertText);
                while (!UsableExtraImports->IsArrayEnd)
                {
                    InsertText = ReplaceSectionOfString(InsertText, GetShiftedAmount(UsableExtraImports->StartIndex, JSFileShiftLocations), GetShiftedAmount(UsableExtraImports->StartIndex + 8, JSFileShiftLocations), NewModuleExportsName);
                    AddShiftNum(UsableExtraImports->StartIndex, strlen(NewModuleExportsName) - 8, &JSFileShiftLocations, &JSShiftLocationsLength);
                    UsableExtraImports++;
                }
                printf("\n\nInsert text:%s\n\n\n\n", InsertText);
                FileContents = ReplaceSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(CurrentEdge->EndRefPos, ShiftLocations) + 1, NewModuleExportsName);
                AddShiftNum(CurrentEdge->StartRefPos, strlen(NewModuleExportsName) - ((CurrentEdge->EndRefPos + 1) - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationsLength);
                FileContents = InsertStringAtPosition(FileContents, InsertText, 0);
                printf("is this working: %s\n", FileContents);
                AddShiftNum(0, strlen(InsertText), &ShiftLocations, &ShiftLocationsLength);
                printf("Edited:%s\n", FileContents);
                /*free(JSFileShiftLocations);
                JSFileShiftLocations = NULL;*/
            }
        }
        CurrentEdge = CurrentEdge->next;
    }
    free(ShiftLocations);
    ShiftLocations = NULL;
    RemoveSubstring(FileContents, "</include>");
    CreateFileWrite(EntryToExitPath(GraphNode->path), FileContents);
    printf("\n\n\n\n");
    ColorGreen();
    printf("Finished bundling file:%s\n", GraphNode->path);
    ColorNormal();
}

void PostProcessFile(struct Node *node, struct Graph *graph)
{
    struct ShiftLocation *shiftLocations = malloc(sizeof(struct ShiftLocation));
    int ShiftlocationLength = 0;
    char *ExitPath = EntryToExitPath(node->path);
    if (node->FileType == JSFILETYPE_ID)
    {
        char *FileContents = ReadDataFromFile(ExitPath);
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
            printf("This file uses small exports\n");
            RemoveSectionOfString(FileContents, GetShiftedAmount(IteratePointer->StartIndex, shiftLocations), GetShiftedAmount(IteratePointer->EndIndex, shiftLocations) + 1);
            AddShiftNum(IteratePointer->StartIndex, IteratePointer->EndIndex - IteratePointer->StartIndex - 1, &shiftLocations, &ShiftlocationLength);
            IteratePointer++;
        }
        ColorGreen();
        printf("Post processed: %s\n\n\n\n\n", FileContents);
        ColorReset();
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
            char *ExitPath = strdup(FileNode->path);
            if (strncasecmp(ExitPath, "node_modules/", 13) == 0)
            {
                ExitPath = ReplaceSectionOfString(ExitPath, 0, 13, Settings.exit);
                printf("node\n");
            }
            else
            {
                ExitPath = ReplaceSectionOfString(ExitPath, 0, strlen(Settings.entry), Settings.exit);
            }

            CopyFile(FileNode->path, ExitPath);
            free(ExitPath);
        }
        else
        {
            FilesBundled--;
            break;
        }
    }

    for (int i = FilesBundled; i < graph->VerticesNum; i++)
    {
        struct Node *FileNode = graph->SortedArray[i];
        ColorMagenta();
        printf("\nBundling file: %s\n", FileNode->path);
        ColorReset();
        print_progress_bar(i, graph->VerticesNum);
        BundleFile(FileNode);
    }

    // PostProcess functions that run after main bundling process
    for (int i = 0; i < graph->VerticesNum; i++)
    {
        PostProcessFile(graph->SortedArray[i], graph);
    }

    return Success; // This is always true currently
}