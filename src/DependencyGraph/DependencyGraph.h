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

typedef struct Edge // Wraps a regular Node struct and includes the start and end positions of where the node is referenced so it doesn't need to be worked out again
{
    struct Node *vertex; // Pointer to the vertex at the end of the edge
    struct Edge *next;   // Pointer to the next edge in the list
    unsigned int StartRefPos, EndRefPos;
} Edge;

typedef struct HiddenEdge
{
    struct Edge *edge;
    struct Node *ConnectedNode;
    struct HiddenEdge *next;
} HiddenEdge;

struct Node // structure for individual nodes (Modules/Files) in the tree
{
    char *path;        // Contains path to the file
    int FileType;      // File type ID for file (File type IDs are define in FileTypesHandler.h)
    struct Edge *edge; // Pointer to the first edge in the list of edges connected to the vertex
    int VertexPos;     // Position of the vertex when it is ordered
    struct HiddenEdge *HiddenEdge;
    bool Bundled, visited;
};
int count_edges(struct Node *vertex);

typedef struct Graph
{
    int VerticesNum; // Number of vertices in the graph
    Node **Vertexes; // Array of pointers to the head of the linked lists for each vertex
    struct Node **SortedArray;
} Graph;
struct Graph *CreateTree(char *Wrapped_paths, int ArrayLength); // Creates dependency tree/graph/array

char **FindDependencies(char *Path); // function that returns an array of strings representing dependencies for the given file

#endif