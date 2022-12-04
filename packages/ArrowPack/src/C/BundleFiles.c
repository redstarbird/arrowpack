#include "BundleFiles.h"

void BundleHTMLFile(struct Node *TreeNode)
{

    char *FileContents = ReadDataFromFile(TreeNode->path);
    // printf("File contents: %s, DependenctNum: %i\n", FileContents, TreeNode->DependenciesInTree);
    for (int i = 0; i < TreeNode->DependenciesInTree; i++)
    {
        printf("Settings exit: %s\n", Settings.exit);
        ReplaceSectionOfString(TreeNode->Dependencies[i].DependencyPath, 0, strlen(Settings.entry), Settings.exit);
        char *InsertText = ReadDataFromFile(TreeNode->Dependencies[i].DependencyPath);
        printf("InsertText = %s\n", InsertText);
        int InsertEnd = TreeNode->Dependencies[i].EndRefPos;
        printf("Strlen(FileContents): %i, InsertEnd: %i\n", (int)strlen(FileContents), InsertEnd);
        /*char *TempPointer = &FileContents[InsertEnd + 1];
        printf("TempPointer = %s\n", TempPointer);
         if (StringStartsWith(TempPointer, "</include>"))
         {
             InsertEnd += 10;
             printf("String \"%s\" starts with \"%s\"\n", TempPointer, "</include>");
         }*/
        printf("String before replace: %s\n", FileContents);
        ReplaceSectionOfString(FileContents, TreeNode->Dependencies[i].StartRefPos, InsertEnd + 1, InsertText);
        printf("String after replace: %s\n", FileContents);
    }
    char *BundledFile;
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