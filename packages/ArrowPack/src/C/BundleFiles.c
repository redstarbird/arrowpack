#include "BundleFiles.h"

void BundleHTMLFile(struct Node *TreeNode)
{
    printf("Bundling file!\n");
    printf("Node path: %s, DependenciesInTree: %i, DependentsInTree: %i\n", TreeNode->path, TreeNode->DependenciesInTree, TreeNode->DependentsInTree);
    for (int i = 0; i < TreeNode->DependenciesInTree; i++)
    {
        char *InsertText = ReadDataFromFile(TreeNode->Dependencies[i].DependencyPath);
        printf("InsertText = %s\n", InsertText);
    }
    char *BundledFile;
}

bool EMSCRIPTEN_KEEPALIVE BundleFiles(Node *DependencyTree)
{
    bool Success = false;
    struct Node *IteratePointer = &DependencyTree[0]; // Pointer used to iterate through dependency tree
    while (IteratePointer->path)                      // Checks if it has reached the end of the array by checking if path is NULL
    {
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