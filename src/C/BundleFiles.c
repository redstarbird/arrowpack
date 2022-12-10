#include "BundleFiles.h"

void BundleHTMLFile(struct Node *TreeNode)
{

    char *FileContents = ReadDataFromFile(TreeNode->path);
    // printf("File contents: %s, DependenctNum: %i\n", FileContents, TreeNode->DependenciesInTree);
    int totalAmountShifted = 0;
    for (int i = 0; i < TreeNode->DependenciesInTree; i++)
    {
        TreeNode->Dependencies[i].DependencyPath = EntryToExitPath(TreeNode->Dependencies[i].DependencyPath);
        char *InsertText = ReadDataFromFile(TreeNode->Dependencies[i].DependencyPath);
        int InsertEnd = TreeNode->Dependencies[i].EndRefPos + 1;
        FileContents = ReplaceSectionOfString(FileContents, TreeNode->Dependencies[i].StartRefPos + totalAmountShifted, InsertEnd + totalAmountShifted, InsertText);
        totalAmountShifted += strlen(InsertText) - (InsertEnd - TreeNode->Dependencies[i].StartRefPos);
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