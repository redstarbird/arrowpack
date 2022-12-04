#include "BundleFiles.h"

void BundleHTMLFile(struct Node *TreeNode)
{

    char *FileContents = ReadDataFromFile(TreeNode->path);
    // printf("File contents: %s, DependenctNum: %i\n", FileContents, TreeNode->DependenciesInTree);
    int totalAmountShifted = 0;
    for (int i = 0; i < TreeNode->DependenciesInTree; i++)
    {
        printf("Settings exit: %s\n", Settings.exit);
        TreeNode->Dependencies[i].DependencyPath = EntryToExitPath(TreeNode->Dependencies[i].DependencyPath);
        char *InsertText = ReadDataFromFile(TreeNode->Dependencies[i].DependencyPath);
        printf("InsertText = %s\n", InsertText);
        int InsertEnd = TreeNode->Dependencies[i].EndRefPos + 1;
        printf("Strlen(FileContents): %i, InsertEnd: %i\n", (int)strlen(FileContents), InsertEnd);
        printf("String before replace: %s\n", FileContents);
        FileContents = ReplaceSectionOfString(FileContents, TreeNode->Dependencies[i].StartRefPos + totalAmountShifted, InsertEnd + totalAmountShifted, InsertText);
        totalAmountShifted += strlen(InsertText) - (InsertEnd - TreeNode->Dependencies[i].StartRefPos);
        printf("String after replace: %s\n", FileContents);
    }
    RemoveSubstring(FileContents, "</include>");
    CreateFileWrite(EntryToExitPath(TreeNode->path), FileContents);
    printf("\n\n\n\n");
}

bool EMSCRIPTEN_KEEPALIVE BundleFiles(Node *DependencyTree)
{
    bool Success = false;
    struct Node *IteratePointer = &DependencyTree[0]; // Pointer used to iterate through dependency tree

    while (1)
    {
        if (IteratePointer->DependenciesInTree == 0 && IteratePointer->IsArrayEnd != true)
        {
            printf("creating file for %s, dependenciesnum: %i\n", IteratePointer->path, IteratePointer->DependenciesInTree);
            char *ExitPath = strdup(IteratePointer->path);
            printf("ExitPath: %s\n", ExitPath);
            ExitPath = ReplaceSectionOfString(ExitPath, 0, strlen(Settings.entry), Settings.exit);
            printf("ExitPath: %s\n", ExitPath);
            CreateFileWrite(ExitPath, ReadDataFromFile(IteratePointer->path));
            IteratePointer++;
            free(ExitPath);
        }
        else
        {
            break;
        }
    }

    while (IteratePointer->IsArrayEnd != true && IteratePointer->path && strlen(IteratePointer->path)) // Checks if it has reached the end of the array by checking if path is NULL
    {
        ColorMagenta();
        printf("\nBundling file: %s, strlen(path) = %i\n", IteratePointer->path, (int)strlen(IteratePointer->path));
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

    return Success;
}