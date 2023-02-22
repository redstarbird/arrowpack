// This file is used to compile all the C WASM functions into one file to minimize file size and WASM performance.

#include <stdio.h>
#include "./C/FileHandler.h"
#include "./DependencyGraph/DependencyGraph.h"
#include "./C/BundleFiles.h"
#include <emscripten.h>

char *EMSCRIPTEN_KEEPALIVE RebuildFiles(struct Graph *DependencyGraph, char *EncodedFiles, int FilesNum) // Currently only used for arrowpack dev server to rebuild files that are changed
{
    printf("Rebuilding...\n");

    char **Files = ArrowDeserialize(EncodedFiles, &FilesNum);
    char **ChangedFiles;
    if (DependencyGraph == NULL)
    {
        // Need to implement
    }

    struct Graph *TempGraph = malloc(sizeof(struct Graph));
    TempGraph->VerticesNum = 0;

    TempGraph->Vertexes = malloc(sizeof(struct Node *) * DependencyGraph->VerticesNum);
    int CurrentVertexNum = 0;
    printf("This is a bit confusing...\n");
    bool FileFound = false;
    for (int i = 0; i < DependencyGraph->VerticesNum; i++) // Finds the vertices for each file that has been modified
    {
        for (int j = 0; j < FilesNum; j++)
        {
            if (strcasecmp(DependencyGraph->Vertexes[i]->path, Files[j]) == 0)
            {
                FileFound = true;
                printf("hmm\n");
                int TempVertexNum = 0;
                struct Node **TempVertexArray = FindAllDependentsOfVertex(DependencyGraph->Vertexes[i], DependencyGraph->VerticesNum, &TempVertexNum);
                TempGraph->SortedArray = TempVertexArray;
                TempGraph->VerticesNum = TempVertexNum;

                ChangedFiles = malloc(TempVertexNum * sizeof(char *));
                for (int k = 0; k < TempVertexNum; k++)
                {
                    ChangedFiles[k] = TempVertexArray[k]->path;
                }

                struct Edge *OldTempEdge = DependencyGraph->Vertexes[i]->edge;
                struct Edge *TempEdge = DependencyGraph->Vertexes[i]->edge;
                while (TempEdge != NULL)
                {
                    TempEdge = TempEdge->next;
                    free(OldTempEdge);
                    OldTempEdge = TempEdge;
                }
                DependencyGraph->Vertexes[i]->edge = NULL;

                CreateDependencyEdges(DependencyGraph->Vertexes[i], &DependencyGraph);
                BundleFiles(TempGraph);
            }
        }
    }
    char *FinalChangedFiles;
    if (FileFound)
    {
        FinalChangedFiles = ArrowSerialize((const char **)ChangedFiles, TempGraph->VerticesNum);
    }
    else
    {
        for (int i = 0; i < FilesNum; i++)
        {
            struct Node *NewVertex = create_vertex(Files[i], GetFileTypeID(Files[i]), NULL);
            add_vertex(TempGraph, NewVertex);
            topological_sort(TempGraph);
            CreateDependencyEdges(NewVertex, &DependencyGraph);
            BundleFiles(TempGraph);
            add_vertex(DependencyGraph, NewVertex);
            topological_sort(DependencyGraph);
            FinalChangedFiles = Files[i];
        }
    }

    free(TempGraph);
    printf("FinalChangedFiles: %s\n", FinalChangedFiles);
    return FinalChangedFiles;
}

void EMSCRIPTEN_KEEPALIVE CheckWasm()
{
    printf("Wasm started Successfully...\n");
}