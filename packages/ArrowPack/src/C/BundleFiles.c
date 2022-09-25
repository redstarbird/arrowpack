#include "BundleFiles.h"

char *BundleHTMLFile(const char *data)
{
    char *BundledFile;
}

bool BundleFiles(Node *DependencyTree)
{
    bool Success = false;

    for (unsigned int i = 0; i < sizeof(DependencyTree) / sizeof(Node); i++)
    {
        char *fileType = GetFileExtension(DependencyTree[i].path);
    }

    return Success;
}