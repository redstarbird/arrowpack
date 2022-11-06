#include "BundleFiles.h"

char *BundleHTMLFile(const char *data)
{
    char *BundledFile;
    return NULL;
}

bool EMSCRIPTEN_KEEPALIVE BundleFiles(Node *DependencyTree)
{
    bool Success = false;

    for (unsigned int i = 0; i < 4; i++)
    {
        printf("Bundling: %s\n", DependencyTree[i].path);
        char *fileType = GetFileExtension(DependencyTree[i].path);
    }

    return Success;
}