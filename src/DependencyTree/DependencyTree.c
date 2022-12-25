#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include "../C/TextColors.h"
#include "DependencyTree.h"
#include "../C/FileHandler.h"
#include <emscripten.h>
#include "../C/cJSON/cJSON.h" // https://github.com/DaveGamble/cJSON
#include "../Regex/RegexFunctions.h"
#include "../C/BundleFiles.h"
#include "../C/StringRelatedFunctions.h"
#include "./FindDependencies.h"
#include "../SettingsSingleton/settingsSingleton.h"

// Function to order the nodes of a graph into a flat list for the module bundler
struct Node **get_vertex_order(Graph *graph)
{
    // Create an array to store the nodes in order
    struct Node **vertex_order = malloc(sizeof(struct Node *) * graph->VerticesNum);
    printf("vertexnum: %i\n", graph->VerticesNum);
    // Initialize the array with NULL values
    for (int i = 0; i < graph->VerticesNum; i++)
    {
        vertex_order[i] = NULL;
    }

    // Set up a queue to store the nodes that still need to be processed
    struct Node **queue = malloc(sizeof(struct Node *) * graph->VerticesNum);
    int queue_size = 0;

    // Add all nodes with no dependents to the queue
    for (int i = 0; i < graph->VerticesNum; i++)
    {
        printf("Test: %s\n", graph->Vertexes[i]->path);
        struct Node *node = graph->Vertexes[i];
        if (node->HiddenEdge == NULL)
        {
            printf("%s has no dependecies\n", graph->Vertexes[i]->path);
            queue[queue_size++] = node;
        }
    }
    // Process the nodes in the queue
    int pos = graph->VerticesNum;
    while (queue_size > 0)
    {
        printf("queue_size:%i\n", queue_size);
        // Get the next node from the queue
        struct Node *node = queue[--queue_size];
        printf("Processing node: %s\n", node->path);
        // Set the pos field of the node

        printf("Adding node %s at pos: %i\n", node->path, pos);
        // Add the node to the order array
        vertex_order[pos--] = node;
        node->VertexPos = pos;
        // Add all of the node's neighbors to the queue
        struct Edge *edge = node->edge;
        while (edge != NULL)
        {
            struct Node *neighbor = edge->vertex;
            printf("Processing neighbor: %s\n", neighbor->path);
            // Check if the neighbor has any remaining dependencies
            bool has_dependencies = false;
            struct Edge *e = neighbor->edge;
            while (e != NULL)
            {
                struct Node *n = e->vertex;
                if (vertex_order[n->VertexPos] == NULL) // Checks if dependency is not already in array
                {
                    has_dependencies = true;
                    break;
                }
                e = e->next;
            }

            // If the neighbor has no remaining dependencies, add it to the queue
            if (!has_dependencies)
            {
                queue[queue_size++] = neighbor;
            }

            edge = edge->next;
        }
    }

    // Free the queue
    free(queue);

    return vertex_order;
}

char *entryPath; // global variable for entry (base) path

/*if (pathLen < charLen) {return false;}
unsigned short int o = 0;
for (unsigned int i = charLen; i >= 0; i--) {
    if (path[*pathLen-i] !=  extension[o]) {return false;}
    o++;
}
return true;*/

struct FileRule GetFileRuleFromPath(const char *path, struct FileRule *fileRules)
{

    unsigned int pathLen = strlen(path);

    int lastFullStop = 0;
    for (unsigned int i = 0; i < pathLen; i++)
    {
        if (path[i] == '.')
        {
            lastFullStop = i;
        }
    }
    if (lastFullStop == 0)
    {
        printf("Error: could not file character \".\" in path %s", path);
        exit(1);
    }

    lastFullStop++;
    const int length = pathLen - lastFullStop; // gets length of file ext
    char extension[length];
    for (int i = 0; i < length; i++)
    {
        extension[i] = path[lastFullStop + i]; // string includes fullstop
    }

    for (int i = 0; i < sizeof(*fileRules) / sizeof(struct FileRule); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%s\n", fileRules[i].FileExtensions[j]);
            if (fileRules[i].FileExtensions[j] == NULL || *fileRules[i].FileExtensions[j] == '\0')
            {
                break;
            }
            if (strcasecmp(fileRules[i].FileExtensions[j], extension) == 0)
            {
                return fileRules[i];
            }
        }
    }
    printf("Could not find rule for processing file %s\n", path);
    exit(1);
}

RegexMatch EMSCRIPTEN_KEEPALIVE *GetDependencies(char *Path, int FileTypeID)
{
    char *FileExtension = GetFileExtension(Path);
    // Very unfortunate that C doesn't support switch statements for strings
    switch (FileTypeID)
    {
    case HTMLFILETYPE_ID:
        printf("Returning HTML Dependencies\n");
        return FindHTMLDependencies(Path);
        break;
    case CSSFILETYPE_ID:
        printf("CSS not implemented yet");
        break;
    case JSFILETYPE_ID:
        printf("JS not implemented yet");
        break;
    case SCSSFILETYPE_ID:
        printf("SCSS not implemented yet");
        break;
    default:
        break;
    }

    struct RegexMatch *temp = malloc(sizeof(RegexMatch)); // just here temporarily so compiler doesnt throw an error
    temp->IsArrayEnd = true;
    return temp;
}

void EMSCRIPTEN_KEEPALIVE FatalInvalidFile(const char *filename)
{ // Throws a fatal error with the given filename
    printf("Fatal error: %s is invalid\n", filename);
    exit(1);
}

struct FileRule *InitFileRules() // Gets file rules from FileTypes.json file
{

    char *rawJSON = ReadDataFromFile("src/FileTypes.json"); // string containing raw JSON from FileTypes.json file

    struct FileRule *fileRules = (struct FileRule *)malloc(sizeof(struct FileRule) * 2);

    cJSON *json = cJSON_Parse(rawJSON);
    cJSON *currentArray;
    cJSON *currentElement;
    cJSON *fileTypesArray;

    unsigned short int currentTimesIterated = 0; // resets for every file rule
    unsigned int arrayElementCount = 0;
    unsigned short int currentFileTypesArrayIndex = 0;

    // char tempFileTypesBuffer[4][32] = {"\0", "\0", "\0", "\0"};

    if (cJSON_IsInvalid(json))
    {
        FatalInvalidFile("FileTypes.json");
    }

    cJSON_ArrayForEach(currentArray, json)
    {
        arrayElementCount++;
        if (cJSON_IsInvalid(currentArray))
        {
            FatalInvalidFile("FileTypes.json");
        }
        if (cJSON_IsArray(currentArray))
        {
            currentTimesIterated = 0;
            currentFileTypesArrayIndex = 0; // reset variables to zero

            fileRules = (struct FileRule *)realloc(fileRules, arrayElementCount * sizeof(struct FileRule));

            cJSON_ArrayForEach(currentElement, currentArray)
            {
                if (cJSON_IsInvalid(currentArray))
                {
                    FatalInvalidFile("FileTypes.json");
                }
                switch (currentTimesIterated)
                {

                case 0:
                    if (cJSON_IsArray(currentElement))
                    {
                        cJSON_ArrayForEach(fileTypesArray, currentElement)
                        {
                            strncpy(fileRules[arrayElementCount].FileExtensions[currentFileTypesArrayIndex], fileTypesArray->valuestring, 32);
                            currentFileTypesArrayIndex++;
                        }
                    }
                    else if (cJSON_IsString(currentElement))
                    {
                        strncpy(fileRules[arrayElementCount].FileExtensions[0], currentElement->valuestring, 32);
                    }
                    break;

                case 1:
                    if (!cJSON_IsString(currentElement))
                    {
                        FatalInvalidFile("FileTypes.json");
                    }
                    else
                    {
                        strncpy(fileRules[arrayElementCount].regexPattern, currentElement->valuestring, 64);
                    }
                    break;

                case 2:
                    if (!cJSON_IsNumber(currentElement))
                    {
                        FatalInvalidFile("FileTypes.json");
                    }
                    else
                    {
                        fileRules[arrayElementCount].StartPos = currentElement->valueint;
                    }
                    break;

                case 3:
                    if (!cJSON_IsNumber(currentElement))
                    {
                        FatalInvalidFile("File");
                    }
                    else
                    {
                        fileRules[arrayElementCount].EndPos = currentElement->valueint;
                    }
                    break;

                default:
                    break;
                }
                currentTimesIterated++;
            }
        }
        else
        {
            FatalInvalidFile("FileTypes.json");
        }
    }
    cJSON_Delete(json); // free memory allocated by cJSON
    return fileRules;
}

// Function to create a new edge
Edge *create_edge(struct Node *vertex, int StartRefPos, int EndRefPos)
{
    // Allocate memory for the edge
    Edge *edge = malloc(sizeof(Edge));

    // Initialize the fields of the edge
    edge->vertex = vertex;
    edge->next = NULL;
    edge->StartRefPos = StartRefPos;
    edge->EndRefPos = EndRefPos;

    return edge;
}

struct HiddenEdge *CreateHiddenEdge(struct Edge *edge, struct Node *HiddenNode)
{
    struct HiddenEdge *hidden_edge = malloc(sizeof(struct HiddenEdge));
    hidden_edge->edge = edge;
    hidden_edge->next = NULL;
    hidden_edge->ConnectedNode = HiddenNode;
    return hidden_edge;
}

// Function to create a new vertex
struct Node *create_vertex(char *path, int filetype, Edge *edge)
{
    // Allocate memory for the vertex
    struct Node *vertex = malloc(sizeof(struct Node));

    // Initialize the fields of the vertex
    vertex->path = path;
    vertex->FileType = filetype;
    vertex->edge = edge;
    vertex->HiddenEdge = NULL;
    vertex->VertexPos = -1;

    return vertex;
}

// Function to add an edge to a vertex
void add_edge(struct Node *vertex, struct Node *neighbor, int StartRefPos, int EndRefPos)
{
    // Create a new edge
    Edge *edge = create_edge(neighbor, StartRefPos, EndRefPos);
    struct HiddenEdge *hidden_edge = CreateHiddenEdge(edge, vertex);

    // Add the edge to the front of the list of edges
    edge->next = vertex->edge;
    vertex->edge = edge;
    hidden_edge->next = neighbor->HiddenEdge;
    neighbor->HiddenEdge = hidden_edge;
}

// Function to add a vertex to a graph
void add_vertex(Graph *graph, struct Node *vertex)
{
    // Increase the size of the vertexes array
    graph->VerticesNum++;
    graph->Vertexes = realloc(graph->Vertexes, sizeof(struct Node *) * graph->VerticesNum);

    // Add the vertex to the end of the array
    graph->Vertexes[graph->VerticesNum - 1] = vertex;
}

int count_edges(struct Node *vertex)
{
    int count = 0;
    struct Edge *edge = vertex->edge;
    // Iterate through the edges connected to the vertex
    while (edge != NULL)
    {
        count++;
        edge = edge->next;
    }

    return count;
}

struct Node EMSCRIPTEN_KEEPALIVE **CreateTree(char *Wrapped_paths, int ArrayLength) // Main function for creating dependency tree
{

    printf("Started creating dependency tree\n");
    int CurrentWrappedArrayIndex = 0;
    int lastNewElement = 0;
    int ElementsUnwrapped = 0;

    char **paths = malloc((ArrayLength - 1) * (sizeof(char *) + 1)); // Allocates memory for array of strings

    while (ElementsUnwrapped < ArrayLength && CurrentWrappedArrayIndex < strlen(Wrapped_paths)) // Paths are wrapped into one string because passing array of strings from JS to C is complicated
    {
        if (Wrapped_paths[CurrentWrappedArrayIndex] == ':') // Checks for paths divider character thing
        {
            paths[ElementsUnwrapped] = (char *)malloc(CurrentWrappedArrayIndex - lastNewElement + 1);

            for (int i = 0; i < CurrentWrappedArrayIndex - lastNewElement; i++)
            {
                paths[ElementsUnwrapped][i] = Wrapped_paths[lastNewElement + i];
            }
            paths[ElementsUnwrapped][CurrentWrappedArrayIndex - lastNewElement] = '\0';
            lastNewElement = CurrentWrappedArrayIndex + 2;
            ElementsUnwrapped++;
            CurrentWrappedArrayIndex++;
        }

        CurrentWrappedArrayIndex++;
    }
    size_t TreeSize = (int)sizeof(struct Node) * ArrayLength;

    struct Graph *DependencyGraph = malloc(sizeof(struct Graph)); // Allocates memory for graph
    DependencyGraph->VerticesNum = 0;                             // Sets number of vertices in graph

    DependencyGraph->Vertexes = malloc(sizeof(struct Node));
    for (unsigned int i = 0; i < ArrayLength; i++) // Sets up values for each element in the tree
    {

        add_vertex(DependencyGraph, create_vertex(TurnToFullRelativePath(paths[i], ""), GetFileTypeID(paths[i]), NULL));
    }
    ColorGreen();
    printf("Finding dependencies...\n\n\n");
    ColorReset();

    bool DependencyFound = false;

    for (int i = 0; i < ArrayLength; i++) // Loops through each node and finds dependencies
    {
        printf("Graph processing: %s\n", DependencyGraph->Vertexes[i]->path);
        printf("Finding dependencies for file: %s\n", paths[i]);
        struct RegexMatch *Dependencies = GetDependencies(paths[i], DependencyGraph->Vertexes[i]->FileType); // Gets dependencies as strings
        if (Dependencies != NULL && Dependencies[0].IsArrayEnd == false)                                     // Checks if dependencies have been found
        {
            struct RegexMatch *IteratePointer = &Dependencies[0];
            while (IteratePointer->IsArrayEnd != true) // Loops through each dependency
            {

                for (int j = 0; j < DependencyGraph->VerticesNum; j++) // Compares dependencies found to dependencies in tree
                {
                    if (strcasecmp(DependencyGraph->Vertexes[j]->path, IteratePointer->Text) == 0)
                    {
                        add_edge(DependencyGraph->Vertexes[i], DependencyGraph->Vertexes[j], IteratePointer->StartIndex, IteratePointer->EndIndex);
                        DependencyFound = true;
                        break;
                    }
                }
                if (!DependencyFound)
                {
                    ColorYellow();
                    printf("Couldn't find dependency %s in tree. External dependancies are not yet supported :(\n", IteratePointer->Text);
                    ColorNormal();
                }
                DependencyFound = false;
                IteratePointer++;
            }

            // free(Dependencies); this code was causing errors for some reason need to fix because it is probably causing memory leaks
        }
        printf("\n\nFile has %i dependencies!\n\n", count_edges(DependencyGraph->Vertexes[i]));
    }
    printf("\n\n");

    struct Node **StructArray = get_vertex_order(DependencyGraph);

    for (int i = 0; i < DependencyGraph->VerticesNum; i++)
    {
        struct Node *node = StructArray[i];
        printf("Ordered: %s\n", node->path);
    }

    printf("exiting\n");
    exit(0);
    return StructArray;
}
