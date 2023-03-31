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
    bool FileFound = false;
    for (int i = 0; i < DependencyGraph->VerticesNum; i++) // Finds the vertices for each file that has been modified
    {
        for (int j = 0; j < FilesNum; j++)
        {
            if (strcasecmp(DependencyGraph->Vertexes[i]->path, Files[j]) == 0 || strcasecmp(EntryToPreprocessPath(Files[j]), DependencyGraph->Vertexes[i]->path) == 0)
            {
                FileFound = true;
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
                if (StringStartsWith(DependencyGraph->Vertexes[i]->path, PREPROCESS_DIR))
                {
                    DependencyGraph->Vertexes[i]->path = ReplaceSectionOfString(DependencyGraph->Vertexes[i]->path, 0, 30, GetSetting("entry")->valuestring);
                }
                CreateDependencyEdges(DependencyGraph->Vertexes[i], &DependencyGraph);
                struct Edge *NewTempEdge = DependencyGraph->Vertexes[i]->edge;
                while (NewTempEdge != NULL)
                {
                    if (StringStartsWith(NewTempEdge->vertex->path, PREPROCESS_DIR))
                    {
                        CreateDependencyEdges(NewTempEdge->vertex, &DependencyGraph);
                        BundleFile(NewTempEdge->vertex);
                    }
                    NewTempEdge = NewTempEdge->next;
                }
                // topological_sort(TempGraph);
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
            struct Edge *tempEdge = NewVertex->edge;
            while (tempEdge != NULL)
            {
                if (StringStartsWith(tempEdge->vertex->path, PREPROCESS_DIR))
                {
                    add_vertex(TempGraph, tempEdge->vertex);
                }
                tempEdge = tempEdge->next;
            }

            BundleFiles(TempGraph);
            add_vertex(DependencyGraph, NewVertex);
            topological_sort(DependencyGraph);
            FinalChangedFiles = Files[i];
        }
    }

    free(TempGraph);
    return FinalChangedFiles;
}

void EMSCRIPTEN_KEEPALIVE CheckWasm()
{
    printf("Wasm started Successfully...\n");
}