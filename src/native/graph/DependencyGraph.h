#ifndef DEPENDENCYGRAPH_H
#define DEPENDENCYGRAPH_H

#define PATH_SEPARATOR '/'

#include <emscripten.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "../external/cJSON/cJSON.h"  // https://github.com/DaveGamble/cJSON
#include "../util/FileUtil.h"
#include "../util/RegexUtil.h"
#include "../util/StringUtil.h"
#include "../util/TextColors.h"
#include "./FileTypesHandler.h"


/**
 * @brief Stores how file type(s) should be processed to find dependencies
 *
 */
typedef struct FileRule {
    // File extensions registered for this FileRule
    char FileExtensions[4][32];

    // Regex pattern for this FileRule
    char regexPattern[64];

    // Position from end of string
    unsigned short int StartPos;

    // Position from end of string
    unsigned short int EndPos;
} FileRule;

typedef struct Node Node;

typedef struct HTMLCustomAttribrutes {
    char **AttributeContents;
    char **AttributeNames;
    int length;
} HTMLCustomAttribrutes;

union extraData {
    struct HTMLCustomAttribrutes *HTMLCustomAttributes;
};

// Wraps a regular Node struct and includes the start and end positions of
// where the node is referenced so it doesn't need to be worked out again
typedef struct Edge {
    struct Node *vertex;  // Pointer to the vertex at the end of the edge
    struct Edge *next;    // Pointer to the next edge in the list
    unsigned int StartRefPos, EndRefPos;
    union extraData extraData;
} Edge;

// Stores a reverse connection between modules (from dependency to dependent)
typedef struct HiddenEdge {
    struct Edge *edge;
    struct Node *ConnectedNode;
    struct HiddenEdge *next;
} HiddenEdge;

// Structure for individual nodes (Modules/Files) in the tree
struct Node {
    char *path;                     // Contains path to the file
    int FileType;                   // File type ID for file (File type IDs are define in FileTypesHandler.h)
    struct Edge *edge;              // Pointer to the first edge in the list of edges connected to the vertex
    int VertexPos;                  // Position of the vertex when it is ordered
    struct HiddenEdge *HiddenEdge;  // Pointer to the first dependent of this module, stored as a linked list
    bool Bundled, visited, RebuildChecked;
};
int count_edges(struct Node *vertex);

typedef struct Graph {
    int VerticesNum;            // Number of vertices in the graph
    Node **Vertexes;            // Array of pointers to the head of the linked lists for each vertex
    struct Node **SortedArray;  // Array that contains all of the vertex in the order that they need
                                // to be bundled
} Graph;

/**
 * @brief Creates the dependency graph.
 *
 * The `entry` config setting is used to crawl through and discover files.
 * Each file is added as a vertex/module in the graph.
 * Each module file is opened and parsed to find dependencies between modules.
 *
 * @return pointer to the dependency graph.
 */
struct Graph *CreateGraph();

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

#endif  // !DEPENDENCYGRAPH_H