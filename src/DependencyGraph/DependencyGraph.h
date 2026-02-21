#ifndef DEPENDENCYTREE_H
#define DEPENDENCYTREE_H

#define PATH_SEPARATOR '/'

#include <stdbool.h>
#include "../C/FileTypesHandler.h"
#include "../Regex/RegexFunctions.h"

typedef struct FileRule
{
    char FileExtensions[4][32];
    char regexPattern[64];
    unsigned short int StartPos, EndPos; // endpos is pos from end of string
} FileRule;

typedef struct Node Node;

typedef struct HTMLCustomAttribrutes
{
    char **AttributeContents;
    char **AttributeNames;
    int length;
} HTMLCustomAttribrutes;

union extraData
{
    struct HTMLCustomAttribrutes *HTMLCustomAttributes;
};

typedef struct Edge // Wraps a regular Node struct and includes the start and end positions of where the node is referenced so it doesn't need to be worked out again
{
    struct Node *vertex; // Pointer to the vertex at the end of the edge
    struct Edge *next;   // Pointer to the next edge in the list
    unsigned int StartRefPos, EndRefPos;
    union extraData extraData;
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
    bool Bundled, visited, RebuildChecked;
};
int count_edges(struct Node *vertex);

typedef struct Graph
{
    int VerticesNum;           // Number of vertices in the graph
    Node **Vertexes;           // Array of pointers to the head of the linked lists for each vertex
    struct Node **SortedArray; // Array that contains all of the vertex in the order that they need to be bundled
} Graph;
struct Graph *CreateGraph(); // Creates dependency tree/graph/array
// Function to create a new edge
Edge *create_edge(struct Node *vertex, int StartRefPos, int EndRefPos);

// Function to add an edge to a vertex
void add_edge(struct Node *vertex, struct Node *neighbor, int StartRefPos, int EndRefPos);

void CreateDependencyEdges(struct Node *vertex, struct Graph **DependencyGraph);
struct Node *create_vertex(char *path, int filetype, Edge *edge);
void add_vertex(Graph *graph, struct Node *vertex);

struct Node **FindAllDependentsOfVertex(struct Node *Vertex, const size_t MaxStackSize, int *Number);
struct Node **FindAllDependenciesOfVertex(struct Node *Vertex, const size_t MaxStackSize, int *Number);
void EMSCRIPTEN_KEEPALIVE topological_sort(Graph *graph);
void RemoveEdges(struct Node *);
RegexMatch EMSCRIPTEN_KEEPALIVE *GetDependencies(struct Node *vertex, int FileTypeID, struct Graph **DependencyGraph);
#endif