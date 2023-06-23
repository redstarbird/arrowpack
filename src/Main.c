// This file is used to compile all the C WASM functions into one file to minimize file size and WASM performance.

#include <stdio.h>
#include "./C/FileHandler.h"
#include "./DependencyGraph/DependencyGraph.h"
#include "./C/BundleFiles.h"
#include "./SettingsSingleton/settingsSingleton.h"
#include <emscripten.h>

char *EMSCRIPTEN_KEEPALIVE RebuildFiles(struct Graph *DependencyGraph, char *EncodedFiles, int FilesNum) // Currently only used for arrowpack dev server to rebuild files that are changed
{
    printf("Rebuilding...\n");

    char **Files = ArrowDeserialize(EncodedFiles, &FilesNum); // Deserialises the array of file paths that have changed
    char **ChangedFiles;
    if (DependencyGraph == NULL)
    {
        // Need to implement
    }

    struct Graph *TempGraph = malloc(sizeof(struct Graph)); // Allocates a new temporary dependency graph
    TempGraph->VerticesNum = 0;
    TempGraph->Vertexes = malloc(sizeof(struct Node *) * DependencyGraph->VerticesNum);

    bool FileFound = false;
    for (int i = 0; i < DependencyGraph->VerticesNum; i++) // Finds the vertices for each file that has been modified
    {
        for (int j = 0; j < FilesNum; j++) // Loops through each file that has changed to find it in the graph
        {
            if (strcasecmp(DependencyGraph->Vertexes[i]->path, Files[j]) == 0 ||
                strcasecmp(EntryToPreprocessPath(Files[j]), DependencyGraph->Vertexes[i]->path) == 0) // Checks if the search vertex is the file that has changed
            {
                FileFound = true;

                int TempVertexNum = 0;
                struct Node **TempVertexArray = FindAllDependentsOfVertex(DependencyGraph->Vertexes[i], DependencyGraph->VerticesNum, &TempVertexNum); // Gets all of the dependents of the changed file
                TempGraph->SortedArray = TempVertexArray;
                TempGraph->VerticesNum = TempVertexNum;

                // Create array of paths of changed files
                ChangedFiles = malloc(TempVertexNum * sizeof(char *));
                for (int k = 0; k < TempVertexNum; k++)
                {
                    ChangedFiles[k] = TempVertexArray[k]->path;
                }

                // Delete old edges from graph
                struct Edge *OldTempEdge = DependencyGraph->Vertexes[i]->edge;
                struct Edge *TempEdge = DependencyGraph->Vertexes[i]->edge;
                while (TempEdge != NULL)
                {
                    TempEdge = TempEdge->next;
                    free(OldTempEdge);
                    OldTempEdge = TempEdge;
                }
                DependencyGraph->Vertexes[i]->edge = NULL;

                // Make sure the original file is being looked at
                if (StringStartsWith(DependencyGraph->Vertexes[i]->path, PREPROCESS_DIR))
                {
                    DependencyGraph->Vertexes[i]->path = ReplaceSectionOfString(DependencyGraph->Vertexes[i]->path, 0, 30, GetSetting("entry")->valuestring);
                }

                CreateDependencyEdges(DependencyGraph->Vertexes[i], &DependencyGraph); // Research dependencies for the changed file

                struct Edge *NewTempEdge = DependencyGraph->Vertexes[i]->edge;
                while (NewTempEdge != NULL) // Loop through dependencies of vertex
                {
                    if (StringStartsWith(NewTempEdge->vertex->path, PREPROCESS_DIR)) // Rebuild dependencies in the preprocess directory
                    {
                        CreateDependencyEdges(NewTempEdge->vertex, &DependencyGraph);
                        BundleFile(NewTempEdge->vertex);
                    }
                    NewTempEdge = NewTempEdge->next;
                }

                BundleFiles(TempGraph); // Rebundle all changed files and dependents
            }
        }
    }

    char *FinalChangedFiles;
    if (FileFound)
    {
        FinalChangedFiles = ArrowSerialize((const char **)ChangedFiles, TempGraph->VerticesNum); // Serialise array of all changed files into a string to send to JS
    }
    else // File is a new created file
    {
        for (int i = 0; i < FilesNum; i++) //
        {
            struct Node *NewVertex = create_vertex(Files[i], GetFileTypeID(Files[i]), NULL); // Create vertex for the new file

            add_vertex(TempGraph, NewVertex); // Add vertex to the temporary dependency graph

            CreateDependencyEdges(NewVertex, &DependencyGraph); // Get dependencies for vertex

            topological_sort(TempGraph); // Resort the dependency graph of changed files

            struct Edge *tempEdge = NewVertex->edge;
            while (tempEdge != NULL)
            {
                if (StringStartsWith(tempEdge->vertex->path, PREPROCESS_DIR))
                {
                    add_vertex(TempGraph, tempEdge->vertex);
                }
                tempEdge = tempEdge->next;
            }

            BundleFiles(TempGraph); // Bundle the changed files
            add_vertex(DependencyGraph, NewVertex); // Add the vertex to the dependency graph (doesn't need to be resorted because the vertex is at the end of the graph and has no dependents yet)
            FinalChangedFiles = Files[i]; // The only file changed is the new file
        }
    }

    free(TempGraph); // Free the temporary graph
    return FinalChangedFiles;
}

void EMSCRIPTEN_KEEPALIVE CheckWasm()
{
    printf("Wasm started Successfully...\n");
}