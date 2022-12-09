#ifndef DEPENDENCYTREE_H
#define DEPENDENCYTREE_H

#define PATH_SEPARATOR '/'

#include <stdbool.h>
#include "../C/FileTypesHandler.h"

typedef struct FileRule
{
    char FileExtensions[4][32];
    char regexPattern[64];
    unsigned short int StartPos, EndPos; // endpos is pos from end of string
} FileRule;

typedef struct Node Node;

typedef struct Dependency // Wraps a regular Node struct and includes the start and end positions of where the node is referenced so it doesn't need to be worked out again
{
    unsigned int StartRefPos, EndRefPos;
    char *DependencyPath;
} Dependency;

struct Node
{ // structure for individual nodes (Modules/Files) in the tree
    bool IsArrayEnd;
    char *path;
    struct Node *Dependents;
    struct Dependency *Dependencies;
    unsigned int DependenciesInTree, DependentsInTree;
    unsigned int FileType; // File type ID for file (File type IDs are define in FileTypesHandler.h)
};

struct Node *CreateTree(char *Wrapped_paths, int ArrayLength); // Creates dependency tree/graph/array

char **FindDependencies(char *Path); // function that returns an array of strings representing dependencies for the given file

#endif