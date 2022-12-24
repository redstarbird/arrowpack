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

typedef struct Edge // Wraps a regular Node struct and includes the start and end positions of where the node is referenced so it doesn't need to be worked out again
{
    unsigned int StartRefPos, EndRefPos;
} Edge;

typedef struct Node Node;

struct Node
{ // structure for individual nodes (Modules/Files) in the tree
    char *path;
    bool visited;
    unsigned int DependenciesInTree, DependentsInTree;
    struct Node *next;
    int FileType; // File type ID for file (File type IDs are define in FileTypesHandler.h)
    struct Edge *edge;
};

typedef struct Graph
{
    int VerticesNum;    // Number of vertices in the graph
    Node **Adjacencies; // Array of pointers to the head of the linked lists for each vertex
} Graph;
struct Node *CreateTree(char *Wrapped_paths, int ArrayLength); // Creates dependency tree/graph/array

char **FindDependencies(char *Path); // function that returns an array of strings representing dependencies for the given file

#endif