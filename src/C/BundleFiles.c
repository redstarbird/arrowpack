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
    *ShiftLocations = realloc(*ShiftLocations, ShiftLocationLength * sizeof(struct ShiftLocation));

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

void BundleHTMLFile(struct Node *TreeNode)
{
    ShiftLocationLength = 1; // Includes end element to signal the end of the array
    struct ShiftLocation *ShiftLocations = malloc(sizeof(ShiftLocation));
    ShiftLocations[0].location = -1; // Indicates end of array although probably not needed because the length of the array is being stored

    char *FileContents = ReadDataFromFile(TreeNode->path);
    // printf("File contents: %s, DependenctNum: %i\n", FileContents, TreeNode->DependenciesInTree);

    for (int i = 0; i < TreeNode->DependenciesInTree; i++)
    {
        ColorGreen();
        printf("Building file: %s\n", TreeNode->Dependencies[i].DependencyPath);
        ColorNormal();
        TreeNode->Dependencies[i].DependencyPath = EntryToExitPath(TreeNode->Dependencies[i].DependencyPath);
        char *InsertText = ReadDataFromFile(TreeNode->Dependencies[i].DependencyPath);
        int DependencyFileType = GetFileTypeID(TreeNode->Dependencies[i].DependencyPath);
        if (DependencyFileType == HTMLFILETYPE_ID)
        {
            int InsertEnd = TreeNode->Dependencies[i].EndRefPos + 1;
            FileContents = ReplaceSectionOfString(
                FileContents,
                GetShiftedAmount(TreeNode->Dependencies[i].StartRefPos, ShiftLocations),
                GetShiftedAmount(TreeNode->Dependencies[i].EndRefPos + 1, ShiftLocations), InsertText);
            // totalAmountShifted += strlen(InsertText) - (InsertEnd - TreeNode->Dependencies[i].StartRefPos);
            AddShiftNum(TreeNode->Dependencies[i].StartRefPos, strlen(InsertText) - (InsertEnd - TreeNode->Dependencies[i].StartRefPos), &ShiftLocations, &ShiftLocationLength);
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
                            GetShiftedAmount(TreeNode->Dependencies[i].StartRefPos, ShiftLocations),
                            GetShiftedAmount(TreeNode->Dependencies[i].EndRefPos, ShiftLocations) + 1);
                        // totalAmountShifted -= (TreeNode->Dependencies[i].EndRefPos - TreeNode->Dependencies[i].StartRefPos + 1);
                        AddShiftNum(TreeNode->Dependencies[i].StartRefPos, (TreeNode->Dependencies[i].EndRefPos - TreeNode->Dependencies[i].StartRefPos + 1) * -1, &ShiftLocations, &ShiftLocationLength);
                        FileContents = InsertStringAtPosition(FileContents, InsertString, HeadTagResults[0].EndIndex);
                        AddShiftNum(GetInverseShiftedAmount(HeadTagResults[0].EndIndex, ShiftLocations), strlen(InsertString), &ShiftLocations, &ShiftLocationLength);
                        // totalAmountShifted += strlen(InsertString);
                    }
                    else
                    {
                        ColorYellow();
                        printf("No <head> tag found for file: %s, unable to bundle CSS into file, file will still work\n", TreeNode->path);
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
            int TempShiftedAmount = GetShiftedAmount(TreeNode->Dependencies[i].EndRefPos, ShiftLocations);
            for (int v = GetShiftedAmount(TreeNode->Dependencies[i].StartRefPos, ShiftLocations); v < strlen(FileContents); v++)
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
                AddShiftNum(TreeNode->Dependencies[i].StartRefPos, (endlocation - startlocation) * -1, &ShiftLocations, &ShiftLocationLength);
                FileContents = InsertStringAtPosition(FileContents, InsertText, GetShiftedAmount(TreeNode->Dependencies[i].EndRefPos + 1, ShiftLocations));
                AddShiftNum(TreeNode->Dependencies[i].EndRefPos + 1, strlen(InsertText), &ShiftLocations, &ShiftLocationLength);
            }
        }
        else // Will hopefully work for most custom dependencies
        {
            char *InsertText = ReadDataFromFile(TreeNode->Dependencies[i].DependencyPath);
            int InsertEnd2 = TreeNode->Dependencies[i].EndRefPos + 1;
            FileContents = ReplaceSectionOfString(FileContents, GetShiftedAmount(TreeNode->Dependencies[i].StartRefPos, ShiftLocations), GetShiftedAmount(InsertEnd2, ShiftLocations), InsertText);

            AddShiftNum(TreeNode->Dependencies[i].StartRefPos, strlen(InsertText) - (InsertEnd2 - TreeNode->Dependencies[i].StartRefPos), &ShiftLocations, &ShiftLocationLength);
        }
    }
    RemoveSubstring(FileContents, "</include>");
    CreateFileWrite(EntryToExitPath(TreeNode->path), FileContents);
    printf("\n\n\n\n");
}

bool EMSCRIPTEN_KEEPALIVE BundleFiles(Node *DependencyTree)
{
    bool Success = true;
    struct Node *IteratePointer = &DependencyTree[0]; // Pointer used to iterate through dependency tree

    while (1)
    {
        if (IteratePointer->DependenciesInTree == 0 && IteratePointer->IsArrayEnd != true)
        {
            printf("Duplicating file: %s to exit path\n", IteratePointer->path);
            char *ExitPath = strdup(IteratePointer->path);
            ExitPath = ReplaceSectionOfString(ExitPath, 0, strlen(Settings.entry), Settings.exit);
            CopyFile(IteratePointer->path, ExitPath);
            IteratePointer++;
            free(ExitPath);
        }
        else
        {
            break;
        }
    }

    while (IteratePointer->IsArrayEnd != true) // Checks if it has reached the end of the array by checking if path is NULL
    {

        ColorMagenta();
        printf("\nBundling file: %s\n", IteratePointer->path);
        ColorReset();
        char *fileType = GetFileExtension(IteratePointer->path); // Get file type of the Node

        if (strcmp(fileType, "html") == 0) // C doesn't support switch statements for strings
        {

            BundleHTMLFile(IteratePointer);
        }
        else
        {
            CreateWarning("File is not a supported file type");
        }
        IteratePointer++;
    }

    return Success; // This is always true currently
}