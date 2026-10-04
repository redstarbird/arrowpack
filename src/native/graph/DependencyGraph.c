#include "DependencyGraph.h"

#include "../codegen/BundleFiles.h"
#include "../util/Stack.h"
#include "./FindDependencies.h"

/**
 * @brief Recursively finds all dependents or dependencies of a given vertex including nested.
 *
 * @param Vertex The vertex to recursively search
 * @param stack The current stack of dependents/dependencies
 * @param FindDependencies Whether to find dependents or dependencies
 */
void VertexRecursiveSearch(struct Node *Vertex, struct Stack *stack, bool FindDependencies)
{
    // Mark current vertex as found
    Vertex->RebuildChecked = true;

    // For finding dependencies
    if (FindDependencies) {
        printf("Finding dependencies, not dependents\n");
        struct Edge *currentEdge = Vertex->edge;
        while (currentEdge != NULL)  // Loop through all edges
        {
            if (!Vertex->RebuildChecked)  // Check that the current vertex hasn't already been found
            {
                Stackpush(stack, currentEdge->vertex);  // Push the vertex to the stack
                VertexRecursiveSearch(currentEdge->vertex, stack,
                                      true);  // Recursively search dependencies of the new vertex

                Vertex->RebuildChecked = true;  // Mark vertex as found
            }
            currentEdge = currentEdge->next;  // Go to the next edge
        }
    }
    else  // For finding dependents
    {
        struct HiddenEdge *currentEdge = Vertex->HiddenEdge;
        while (currentEdge != NULL)  // Loop through all hidden edges
        {
            if (!currentEdge->ConnectedNode->RebuildChecked)  // Check the vertex hasn't already been found
            {
                VertexRecursiveSearch(currentEdge->ConnectedNode, stack,
                                      false);   // Recursively search dependents of the vertex
                Vertex->RebuildChecked = true;  // Mark vertex as found
            }
            currentEdge = currentEdge->next;  // Go to next edge
        }
        Stackpush(stack, Vertex);  // Push the vertex to the stack
    }
}

// Returns all dependents of a given vertex including nested dependents
struct Node **FindAllDependentsOfVertex(struct Node *Vertex, const size_t MaxStackSize, int *Number)
{
    struct Stack *stack = CreateStack(MaxStackSize, STACK_VERTEX, true);  // Create a new stack
    VertexRecursiveSearch(Vertex, stack,
                          false);  // Recursively add all dependents of the vertex to the stack in order
    struct Node **Dependents = malloc(sizeof(struct Node *) * (stack->top + 1));
    int DependentCount = 0;
    while (!StackIsEmpty(stack))  // Add vertices from the stack to the array
    {
        Dependents[DependentCount] = Stackpop(stack);
        Dependents[DependentCount]->RebuildChecked = false;
        DependentCount++;
    }
    *Number = DependentCount;  // Set the number of dependents
    return Dependents;
}

// Returns all dependencies of a vertex
struct Node **FindAllDependenciesOfVertex(struct Node *Vertex, size_t MaxStackSize, int *Number)
{
    struct Stack *stack = CreateStack(MaxStackSize, STACK_VERTEX, true);  // Create a new stack
    VertexRecursiveSearch(Vertex, stack,
                          true);  // Recursively add all dependencies of the vertex to the stack in ordere
    struct Node **Dependencies = malloc(sizeof(struct Node *) * (stack->top + 1));
    int DependencyCount = 0;
    while (!StackIsEmpty(stack))  // Add all vertices from the stack to the array
    {
        Dependencies[DependencyCount++] = Stackpop(stack);
        Dependencies[DependencyCount - 1]->RebuildChecked = false;
    }
    *Number = DependencyCount;  // Set the number of dependencies
    return Dependencies;
}

// Recursive function for recursively traversing and topologically sorting dependencies
// Pushes each node to the provided stack in the correct order to be bundled - dependencies before dependents
void topological_sort_dfs(struct Node *node, struct Stack *stack)
{
    // Mark the current node as visited
    node->visited = true;

    // Loop through all edges (dependencies) of node
    struct Edge *edge = node->edge;
    while (edge != NULL) {
        struct Node *Vertex = edge->vertex;
        // Check the current vertex hasn't already been visited
        if (!Vertex->visited) {
            // Recursively sort the current vertex
            topological_sort_dfs(Vertex, stack);
        }

        edge = edge->next;
    }

    // Push the current node to the stack
    Stackpush(stack, node);
}

// Topologically sorts a given graph so dependencies are sorted in the order they need to be
// processed
void EMSCRIPTEN_KEEPALIVE topological_sort(Graph *graph)
{
    // Initialises a stack to store the sorted nodes
    struct Stack *stack = CreateStack(graph->VerticesNum, STACK_VERTEX, false);
    graph->SortedArray = malloc(sizeof(struct Node *) * graph->VerticesNum);

    // Loop through the vertices in the graph
    for (int i = 0; i < graph->VerticesNum; i++) {
        // Sort through each vertex
        if (!graph->Vertexes[i]->visited) {
            topological_sort_dfs(graph->Vertexes[i], stack);
        }
    }

    // Go through the stack and add the nodes to the dependency graph
    int pos = graph->VerticesNum - 1;
    while (!StackIsEmpty(stack)) {
        graph->SortedArray[pos--] = Stackpop(stack);
    }
}

// Finds all dependencies of a given file and returns them as an array of RegexMatch structs
RegexMatch EMSCRIPTEN_KEEPALIVE *GetDependencies(struct Node *vertex, int FileTypeID, struct Graph **DependencyGraph)
{
    // Run the dependency discovery function for a given file type
    char *Path = vertex->path;
    char *FileExtension = GetFileExtension(Path);
    switch (FileTypeID) {
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

    // Return an empty array if dependencies can't be found
    struct RegexMatch *empty = malloc(sizeof(RegexMatch));
    empty->IsArrayEnd = true;
    return empty;
}

// Common function for invalid files
void EMSCRIPTEN_KEEPALIVE FatalInvalidFile(const char *filename)
{
    ThrowFatalError("Fatal error: %s is invalid\n", filename);
}

// Gets file rules from FileTypes.json file (no longer needed)
struct FileRule *InitFileRules()
{

    char *rawJSON = ReadDataFromFile("src/FileTypes.json");  // string containing raw JSON from FileTypes.json file

    struct FileRule *fileRules = (struct FileRule *)malloc(sizeof(struct FileRule) * 2);

    cJSON *json = cJSON_Parse(rawJSON);
    cJSON *currentArray;
    cJSON *currentElement;
    cJSON *fileTypesArray;

    unsigned short int currentTimesIterated = 0;  // resets for every file rule
    unsigned int arrayElementCount = 0;
    unsigned short int currentFileTypesArrayIndex = 0;

    // char tempFileTypesBuffer[4][32] = {"\0", "\0", "\0", "\0"};

    if (cJSON_IsInvalid(json)) {
        FatalInvalidFile("FileTypes.json");
    }

    cJSON_ArrayForEach(currentArray, json)
    {
        arrayElementCount++;
        if (cJSON_IsInvalid(currentArray)) {
            FatalInvalidFile("FileTypes.json");
        }
        if (cJSON_IsArray(currentArray)) {
            currentTimesIterated = 0;
            currentFileTypesArrayIndex = 0;  // reset variables to zero

            fileRules = (struct FileRule *)realloc(fileRules, arrayElementCount * sizeof(struct FileRule));

            cJSON_ArrayForEach(currentElement, currentArray)
            {
                if (cJSON_IsInvalid(currentArray)) {
                    FatalInvalidFile("FileTypes.json");
                }
                switch (currentTimesIterated) {

                case 0:
                    if (cJSON_IsArray(currentElement)) {
                        cJSON_ArrayForEach(fileTypesArray, currentElement)
                        {
                            strncpy(fileRules[arrayElementCount].FileExtensions[currentFileTypesArrayIndex],
                                    fileTypesArray->valuestring, 32);
                            currentFileTypesArrayIndex++;
                        }
                    }
                    else if (cJSON_IsString(currentElement)) {
                        strncpy(fileRules[arrayElementCount].FileExtensions[0], currentElement->valuestring, 32);
                    }
                    break;

                case 1:
                    if (!cJSON_IsString(currentElement)) {
                        FatalInvalidFile("FileTypes.json");
                    }
                    else {
                        strncpy(fileRules[arrayElementCount].regexPattern, currentElement->valuestring, 64);
                    }
                    break;

                case 2:
                    if (!cJSON_IsNumber(currentElement)) {
                        FatalInvalidFile("FileTypes.json");
                    }
                    else {
                        fileRules[arrayElementCount].StartPos = currentElement->valueint;
                    }
                    break;

                case 3:
                    if (!cJSON_IsNumber(currentElement)) {
                        FatalInvalidFile("File");
                    }
                    else {
                        fileRules[arrayElementCount].EndPos = currentElement->valueint;
                    }
                    break;

                default:
                    break;
                }
                currentTimesIterated++;
            }
        }
        else {
            FatalInvalidFile("FileTypes.json");
        }
    }
    cJSON_Delete(json);  // free memory allocated by cJSON
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

// Creates a new hidden (dependent) edge
struct HiddenEdge *CreateHiddenEdge(struct Edge *edge, struct Node *HiddenNode)
{
    // Allocate memory for the hidden edge
    struct HiddenEdge *hidden_edge = malloc(sizeof(struct HiddenEdge));

    // Initialize the fields of the hidden edge
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
    vertex->RebuildChecked = false;

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

    // Add reverse (hidden edge)
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

// Returns the number of edges a vertex has
int count_edges(struct Node *vertex)
{
    int count = 0;
    struct Edge *edge = vertex->edge;

    // Iterate through the edges connected to the vertex
    while (edge != NULL) {
        count++;
        edge = edge->next;
    }

    return count;
}

// Removes all edges connected to a vertex and frees them
void RemoveEdges(struct Node *vertex)
{
    struct Edge *currentEdge = vertex->edge;
    struct Edge *previousEdge;
    while (currentEdge != NULL) {
        previousEdge = currentEdge;
        currentEdge = currentEdge->next;
        free(previousEdge);
    }
}

// Finds all dependencies of the given vertex and creates edges/connections between the vertex and it's
// dependencies
void CreateDependencyEdges(struct Node *vertex, struct Graph **DependencyGraph)
{
    bool DependencyFound = false;

    ColorGreen();
    printf("Finding dependencies for file: %s\n", vertex->path);
    ColorNormal();

    // Parse the file and find its dependencies as RegexMatch structs
    struct RegexMatch *Dependencies = GetDependencies(vertex, vertex->FileType, DependencyGraph);

    // Checks if any dependencies have been found
    if (Dependencies[0].IsArrayEnd == false) {
        // No dependency resolution available for given file type
        if (Dependencies == NULL) {
            (*DependencyGraph)->VerticesNum--;
            return;
        }

        // Loop through each dependency and attempt to locate it in the graph
        struct RegexMatch *IteratePointer = &Dependencies[0];
        while (IteratePointer->IsArrayEnd != true) {

            // Compares dependencies found to dependencies in Graph
            for (int j = 0; j < (*DependencyGraph)->VerticesNum; j++) {

                // Compare module path to dependency import path
                if (strcasecmp((*DependencyGraph)->Vertexes[j]->path, IteratePointer->Text) == 0) {

                    // If the module is the imported dependency, add a edge between the dependent module and dependency
                    add_edge(vertex, (*DependencyGraph)->Vertexes[j], IteratePointer->StartIndex,
                             IteratePointer->EndIndex);

                    // Mark that the dependency was succesfully found
                    DependencyFound = true;
                    break;
                }
            }

            // Dependency found is not in Graph so needs to be added
            if (!DependencyFound) {
                if (!StringStartsWith(IteratePointer->Text, GetSetting("entry")->valuestring)) {
                    ColorMagenta();
                    printf("Creating new node: %s\n", IteratePointer->Text);
                    ColorNormal();

                    // Create vertex for the new module and add it to the graph
                    add_vertex(*DependencyGraph,
                               create_vertex(IteratePointer->Text, GetFileTypeID(IteratePointer->Text), NULL));

                    // Add new vertex as a dependency
                    add_edge(vertex, (*DependencyGraph)->Vertexes[(*DependencyGraph)->VerticesNum - 1],
                             IteratePointer->StartIndex, IteratePointer->EndIndex);


                    // Find dependencies of new dependency
                    CreateDependencyEdges((*DependencyGraph)->Vertexes[(*DependencyGraph)->VerticesNum - 1],
                                          DependencyGraph);

                    // Mark that the dependency was successfully found (added in this case)
                    DependencyFound = true;
                }
                else {
                    CreateWarning("Could not find dependency file %s\n", IteratePointer->Text);
                }
            }

            DependencyFound = false;
            IteratePointer++;
        }
    }

    printf("\n\nFile %s has %i dependencies\n", vertex->path, count_edges(vertex));
}

struct Graph EMSCRIPTEN_KEEPALIVE *CreateGraph()
{
    printf("Creating dependency Graph...\n");

    // Get all of the files from the entry directory
    int pathsNum = 0;
    char **returned_paths = GetAllFilesInDirectory(GetSetting("entry")->valuestring, true, &pathsNum);
    if (pathsNum == 0) {
        printf("No files found in source directory!\n");
    }

    // Create a new dependency graph
    struct Graph *DependencyGraph = malloc(sizeof(struct Graph));
    DependencyGraph->VerticesNum = 0;
    DependencyGraph->Vertexes = malloc(sizeof(struct Node));

    // Add all of the vertexes (modules) to the graph
    for (unsigned int i = 0; i < pathsNum; i++) {

        add_vertex(DependencyGraph, create_vertex(TurnToFullRelativePath(returned_paths[i], ""),
                                                  GetFileTypeID(returned_paths[i]), NULL));
    }

    ColorGreen();
    printf("Finding dependencies...\n\n\n");
    ColorReset();

    // Resolve dependencies and create edges between all vertices
    // Loops through each node and finds its dependencies
    for (unsigned int i = 0; i < DependencyGraph->VerticesNum; i++) {
        CreateDependencyEdges(DependencyGraph->Vertexes[i], &DependencyGraph);
    }

    return DependencyGraph;
}
