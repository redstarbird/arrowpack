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

void EMSCRIPTEN_KEEPALIVE SortDependencyTree(struct Node *tree, int treeLength)
{
    ColorGreen();
    printf("Sorting dependency tree...\n");
    ColorNormal();
    bool SwitchMade = false;
    struct Node tempHolder;
    for (int x = 0; x < FILETYPESIDNUM; x++) // Might be more sorts than needed
    {
        for (int j = 0; j < treeLength - 1; j++) // This basic bubble sort is probably quite slow and inefficient and can probably be optimized a lot
        {
            SwitchMade = false;
            for (int i = 0; i < treeLength - 1; i++)
            {
                if (tree[i].FileType > tree[i + 1].FileType)
                {
                    SwitchMade = true;
                    tempHolder = tree[i]; // Basic code to swap array elements
                    tree[i] = tree[i + 1];
                    tree[i + 1] = tempHolder;
                }
                else if (tree[i].DependenciesInTree > tree[i + 1].DependenciesInTree)
                {
                    SwitchMade = true;
                    tempHolder = tree[i]; // Basic code to swap array elements
                    tree[i] = tree[i + 1];
                    tree[i + 1] = tempHolder;
                }
                else if (tree[i].DependenciesInTree == tree[i + 1].DependenciesInTree)
                {
                    if (tree[i].DependentsInTree < tree[i + 1].DependentsInTree)
                    {
                        SwitchMade = true;
                        tempHolder = tree[i];
                        tree[i] = tree[i + 1];
                        tree[i + 1] = tempHolder;
                    }
                }
            }
            if (tree[0].DependenciesInTree > 0)
            {
                printf("Error: Dependency loop found in file %s.\n", tree[0].path);
                exit(1);
            }
            if (!SwitchMade)
            {
                return;
            }
        }
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

struct Dependency EMSCRIPTEN_KEEPALIVE *DependencyFromRegexMatch(struct RegexMatch *match)
{
    struct Dependency *dependency = malloc(sizeof(struct Dependency)); // Allocates memory for Dependency
    dependency->StartRefPos = match->StartIndex;
    dependency->EndRefPos = match->EndIndex;
    return dependency;
}

struct Edge *create_edge(int startpos, int endpos)
{
    // Allocate memory for the edge structure
    Edge *edge = malloc(sizeof(Edge));

    // Set the startpos and endpos fields of the edge
    edge->StartRefPos = startpos;
    edge->EndRefPos = endpos;

    return edge;
}
struct Node *create_vertex(char *path, int filetype, struct Edge *edge)
{
    struct Node *node = malloc(sizeof(struct Node));
    node->path = path;
    node->FileType = filetype;
    node->next = NULL;
}
void add_edge(struct Graph *graph, int vertex, int neighbor, struct Edge *edge)
{
    // Create a new vertex with the given data value and edge
    struct Node *new_vertex = create_vertex(graph->Adjacencies[neighbor]->path, graph->Adjacencies[neighbor]->FileType, edge);

    // Add the vertex to the adjacency list of the given vertex
    new_vertex->next = graph->Adjacencies[vertex];
    graph->Adjacencies[vertex] = new_vertex;
}

void add_vertex(Graph *graph, char *path, int fileType, struct Edge *edge)
{
    // Create a new vertex
    struct Node *node = create_vertex(path, fileType, edge);
    // Add the vertex to the adjacency list of the graph
    int index = hash(path, graph->VerticesNum);
    node->next = graph->Adjacencies[index];
    graph->Adjacencies[index] = node;
}

int count_edges(struct Node *vertex)
{
    int count = 0;

    // Iterate through the edges connected to the vertex
    while (vertex != NULL)
    {
        count++;
        vertex = vertex->next;
    }

    return count;
}

struct Node EMSCRIPTEN_KEEPALIVE *CreateTree(char *Wrapped_paths, int ArrayLength) // Main function for creating dependency tree
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
    DependencyGraph->VerticesNum = ArrayLength;                   // Sets number of vertices in graph

    DependencyGraph->Adjacencies = malloc(ArrayLength * sizeof(struct Node));
    for (unsigned int i = 0; i < ArrayLength; i++) // Sets up values for each element in the tree
    {
        add_vertex(DependencyGraph, TurnToFullRelativePath(paths[i], ""), GetFileTypeID(paths[i]), NULL);
    }
    ColorGreen();
    printf("Finding dependencies...\n\n\n");
    ColorReset();

    bool DependencyFound = false;

    for (int i = 0; i < ArrayLength; i++) // Loops through each node and finds dependencies
    {
        printf("Finding dependencies for file: %s\n", paths[i]);
        struct RegexMatch *Dependencies = GetDependencies(paths[i], DependencyGraph->Adjacencies[i]->FileType); // Gets dependencies as strings
        if (Dependencies != NULL && Dependencies[0].IsArrayEnd == false)                                        // Checks if dependencies have been found
        {
            struct RegexMatch *IteratePointer = &Dependencies[0];
            while (IteratePointer->IsArrayEnd != true) // Loops through each dependency
            {

                for (int j = 0; j < DependencyGraph->VerticesNum; j++) // Compares dependencies found to dependencies in tree
                {
                    if (strcasecmp(DependencyGraph->Adjacencies[i]->path, IteratePointer->Text) == 0)
                    {
                        add_edge(DependencyGraph, i, j, create_edge(IteratePointer->StartIndex, IteratePointer->EndIndex));
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
        printf("\n\nFile has %i dependencies!\n\n", count_edges(DependencyGraph->Adjacencies[i]));
    }
    printf("\n\n");

    SortDependencyTree(DependencyGraph, ArrayLength);

    for (int i = 0; i < ArrayLength; i++)
    {
        printf("Tree[%i] = %s, file type id: %i\n", i, Tree[i].path, Tree[i].FileType);
    }

    printf("exiting\n");
    exit(0);
    return Tree;
}
