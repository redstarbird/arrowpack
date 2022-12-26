#include "BundleFiles.h"

typedef struct ShiftLocation
{
    int location, ShiftNum;
} ShiftLocation;
static int ShiftLocationLength;

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

static void AddShiftNum(int Location, int ShiftNum, struct ShiftLocation **ShiftLocations, int *needtoremovethis)
{
    ShiftLocationLength++;
    struct ShiftLocation *NewShiftLocations = malloc(ShiftLocationLength * sizeof(struct ShiftLocation));
    memcpy(NewShiftLocations, *ShiftLocations, ShiftLocationLength * sizeof(struct ShiftLocation));
    free(*ShiftLocations);
    *ShiftLocations = NewShiftLocations;
    unsigned int i = 0;
    while (1)
    {
        if ((*ShiftLocations)[i].location >= Location)
        {
            for (int v = ShiftLocationLength; v > i; v--) // Find where to place element so that list is ordered (should probably change to binary search)
            {
                (*ShiftLocations)[v] = (*ShiftLocations)[v - 1];
            }
            (*ShiftLocations)[i].location = Location;
            (*ShiftLocations)[i].ShiftNum = ShiftNum;
            (*ShiftLocations)[ShiftLocationLength].location = -1;
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
}

void BundleHTMLFile(struct Node *GraphNode)
{
    ShiftLocationLength = 1; // Includes end element to signal the end of the array
    struct ShiftLocation *ShiftLocations = malloc(sizeof(ShiftLocation));
    ShiftLocations[0].location = -1; // Indicates end of array although probably not needed because the length of the array is being stored

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
        if (DependencyFileType == HTMLFILETYPE_ID)
        {
            int InsertEnd = CurrentEdge->EndRefPos + 1;
            FileContents = ReplaceSectionOfString(
                FileContents,
                GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations),
                GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations), InsertText);
            // totalAmountShifted += strlen(InsertText) - (InsertEnd - GraphNode->Dependencies[i].StartRefPos);
            AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - (InsertEnd - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationLength);
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
                        AddShiftNum(CurrentEdge->StartRefPos, (CurrentEdge->EndRefPos - CurrentEdge->StartRefPos + 1) * -1, &ShiftLocations, &ShiftLocationLength);
                        FileContents = InsertStringAtPosition(FileContents, InsertString, HeadTagResults[0].EndIndex);
                        AddShiftNum(GetInverseShiftedAmount(HeadTagResults[0].EndIndex, ShiftLocations), strlen(InsertString), &ShiftLocations, &ShiftLocationLength);
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
                    FileContents = InsertStringAtPosition(FileContents, InsertString, StyleResults[0].EndIndex);
                }
            }
        }
        else if (DependencyFileType == JSFILETYPE_ID)
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
                AddShiftNum(CurrentEdge->StartRefPos, (endlocation - startlocation) * -1, &ShiftLocations, &ShiftLocationLength);
                FileContents = InsertStringAtPosition(FileContents, InsertText, GetShiftedAmount(CurrentEdge->EndRefPos + 1, ShiftLocations));
                AddShiftNum(CurrentEdge->EndRefPos + 1, strlen(InsertText), &ShiftLocations, &ShiftLocationLength);
            }
        }
        else // Will hopefully work for most custom dependencies
        {
            char *InsertText = ReadDataFromFile(DependencyExitPath);
            int InsertEnd2 = CurrentEdge->EndRefPos + 1;
            FileContents = ReplaceSectionOfString(FileContents, GetShiftedAmount(CurrentEdge->StartRefPos, ShiftLocations), GetShiftedAmount(InsertEnd2, ShiftLocations), InsertText);

            AddShiftNum(CurrentEdge->StartRefPos, strlen(InsertText) - (InsertEnd2 - CurrentEdge->StartRefPos), &ShiftLocations, &ShiftLocationLength);
        }
        CurrentEdge = CurrentEdge->next;
    }
    free(ShiftLocations);
    ShiftLocations = NULL;
    RemoveSubstring(FileContents, "</include>");
    CreateFileWrite(EntryToExitPath(GraphNode->path), FileContents);
    printf("\n\n\n\n");
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
            ExitPath = ReplaceSectionOfString(ExitPath, 0, strlen(Settings.entry), Settings.exit);
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
        char *fileType = GetFileExtension(FileNode->path); // Get file type of the Node

        if (strcmp(fileType, "html") == 0) // C doesn't support switch statements for strings
        {

            BundleHTMLFile(FileNode);
        }
        else
        {
            CreateWarning("File is not a supported file type");
        }
    }

    return Success; // This is always true currently
}