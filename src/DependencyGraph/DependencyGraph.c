#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include "../C/TextColors.h"
#include "DependencyGraph.h"
#include "../C/FileHandler.h"
#include <emscripten.h>
#include "../C/cJSON/cJSON.h" // https://github.com/DaveGamble/cJSON
#include "../Regex/RegexFunctions.h"
#include "../C/BundleFiles.h"
#include "../C/StringRelatedFunctions.h"
#include "./FindDependencies.h"
#include "../SettingsSingleton/settingsSingleton.h"
#include "../C/Stack.h"

void topological_sort_dfs(struct Node *node, struct Stack *stack)
{
    node->visited = true;

    struct Edge *edge = node->edge;
    while (edge != NULL)
    {
        struct Node *Vertex = edge->vertex;
        if (!Vertex->visited)
        {
            topological_sort_dfs(Vertex, stack);
        }
        edge = edge->next;
    }
    Stackpush(stack, node);
}

void topological_sort(Graph *graph)
{
    struct Stack *stack = CreateStack(graph->VerticesNum, STACK_VERTEX); // Initialises a stack to store the sorted nodes
    graph->SortedArray = malloc(sizeof(struct Node *) * graph->VerticesNum);

    for (int i = 0; i < graph->VerticesNum; i++)
    {
        if (!graph->Vertexes[i]->visited)
        {
            topological_sort_dfs(graph->Vertexes[i], stack);
        }
    }
    int pos = graph->VerticesNum - 1;
    while (!StackIsEmpty(stack))
    {
        graph->SortedArray[pos--] = stack->array.VertexArray[stack->top];
        StackpopV(stack);
    }
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
        ThrowFatalError("Error: could not file character \".\" in path %s", path);
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
    ThrowFatalError("Could not find rule for processing file %s\n", path);
    struct FileRule rule;
    return rule;
}

RegexMatch EMSCRIPTEN_KEEPALIVE *GetDependencies(struct Node *vertex, int FileTypeID, struct Graph **DependencyGraph)
{
    char *Path = vertex->path;
    char *FileExtension = GetFileExtension(Path);
    switch (FileTypeID)
    {
    case HTMLFILETYPE_ID:
        return FindHTMLDependencies(vertex, DependencyGraph);
        break;
    case CSSFILETYPE_ID:
        return FindCSSDependencies(Path);
        break;
    case JSFILETYPE_ID:
        return FindJSDependencies(Path);
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
    ThrowFatalError("Fatal error: %s is invalid\n", filename);
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
    vertex->Bundled = false;
    vertex->visited = false;

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

void CreateDependencyEdges(struct Node *vertex, struct Graph **DependencyGraph)
{
    bool DependencyFound = false;
    ColorGreen();
    printf("Finding dependencies for file: %s\n", vertex->path);
    ColorNormal();
    struct RegexMatch *Dependencies = GetDependencies(vertex, vertex->FileType, DependencyGraph); // Gets dependencies as strings
    if (Dependencies[0].IsArrayEnd == false)                                                      // Checks if dependencies have been found
    {
        if (Dependencies == NULL)

        {
            (*DependencyGraph)->VerticesNum--;
            return;
        }
        struct RegexMatch *IteratePointer = &Dependencies[0];
        while (IteratePointer->IsArrayEnd != true) // Loops through each dependency
        {

            for (int j = 0; j < (*DependencyGraph)->VerticesNum; j++) // Compares dependencies found to dependencies in Graph
            {
                if (strcasecmp((*DependencyGraph)->Vertexes[j]->path, IteratePointer->Text) == 0)
                {
                    add_edge(vertex, (*DependencyGraph)->Vertexes[j], IteratePointer->StartIndex, IteratePointer->EndIndex);
                    DependencyFound = true;
                    break;
                }
            }
            if (!DependencyFound)
            {
                if (!StringStartsWith(IteratePointer->Text, Settings.entry))
                {
                    ColorMagenta();
                    printf("Creating new node: %s\n", IteratePointer->Text);
                    ColorNormal();
                    add_vertex(*DependencyGraph, create_vertex(IteratePointer->Text, GetFileTypeID(IteratePointer->Text), NULL));
                    add_edge(vertex, (*DependencyGraph)->Vertexes[(*DependencyGraph)->VerticesNum - 1], IteratePointer->StartIndex, IteratePointer->EndIndex);
                    DependencyFound = true;
                    CreateDependencyEdges((*DependencyGraph)->Vertexes[(*DependencyGraph)->VerticesNum - 1], DependencyGraph);
                }
                else
                {
                    CreateWarning("Could not find dependency file %s\n", IteratePointer->Text);
                }
            }
            DependencyFound = false;
            IteratePointer++;
        }

        // free(Dependencies); this code was causing errors for some reason need to fix because it is probably causing memory leaks
    }
    printf("\n\nFile has %s %i dependencies\n", vertex->path, count_edges(vertex));
}

struct Graph EMSCRIPTEN_KEEPALIVE *CreateGraph(char *Wrapped_paths, int ArrayLength) // Main function for creating dependency Graph
{

    printf("Creating dependency Graph!\n");
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
    size_t GraphSize = (int)sizeof(struct Node) * ArrayLength;

    struct Graph *DependencyGraph = malloc(sizeof(struct Graph)); // Allocates memory for graph
    DependencyGraph->VerticesNum = 0;                             // Sets number of vertices in graph

    DependencyGraph->Vertexes = malloc(sizeof(struct Node));
    for (unsigned int i = 0; i < ArrayLength; i++) // Sets up values for each element in the Graph
    {

        add_vertex(DependencyGraph, create_vertex(TurnToFullRelativePath(paths[i], ""), GetFileTypeID(paths[i]), NULL));
    }
    ColorGreen();
    printf("Finding dependencies...\n\n\n");
    ColorReset();
    int TempNum = 0;
    while (TempNum < DependencyGraph->VerticesNum) // Loops through each node and finds dependencies
    {
        CreateDependencyEdges(DependencyGraph->Vertexes[TempNum], &DependencyGraph);
        TempNum++;
    }

    topological_sort(DependencyGraph);

    for (int i = 0; i < DependencyGraph->VerticesNum; i++)
    {
        struct Node *node = DependencyGraph->SortedArray[i];
        printf("Ordered graph node: %s\n", node->path);
    }

    return DependencyGraph;
}
