#include "BundleFiles.h"

void BundleHTMLFile(struct Node *TreeNode)
{
    printf("TreeNode: %p\n", TreeNode);

    char *FileContents = ReadDataFromFile(TreeNode->path);
    printf("File contents: %s, DependenctNum: %i\n", FileContents, TreeNode->DependenciesInTree);
    for (int i = 0; i < TreeNode->DependenciesInTree; i++)
    {
        printf("i: %i, %s\n", i, TreeNode->Dependencies[i].DependencyPath);
        char *InsertText = ReadDataFromFile(TreeNode->Dependencies[i].DependencyPath);
        printf("InsertText = %s\n", InsertText);
    }
    char *BundledFile;
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
            CreateFileWrite(EntryToExitPath(IteratePointer->path), ReadDataFromFile(IteratePointer->path));
            IteratePointer++;
        }
        else
        {
            break;
        }
    }

    while (IteratePointer->IsArrayEnd != true && IteratePointer->path && strlen(IteratePointer->path)) // Checks if it has reached the end of the array by checking if path is NULL
    {
        printf("confused now: %i\n", IteratePointer->IsArrayEnd);
        printf("Bundling file: %s, strlen(path) = %i\n", IteratePointer->path, strlen(IteratePointer->path));
        char *fileType = GetFileExtension(IteratePointer->path); // Get file type of the Node

        if (strcmp(fileType, "html") == 0) // C doesn't support switch statements for strings
        {

            printf("File is HTML\n");
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