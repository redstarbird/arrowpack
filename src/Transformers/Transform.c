#include "Transform.h"
// extern void JSTransform(char *FileName);
bool EMSCRIPTEN_KEEPALIVE TransformFiles(struct Graph *DependencyGraph, char *(*functionPTR)(char *, char *, char *))
{
    cJSON *CJSONElement = NULL;
    cJSON *CJSONElement2 = NULL;
    printf("CWD: %s\n", GetSetting("INTERNAL_CWD")->valuestring);
    char *TempTransformerPath = malloc(strlen(GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring) + 1000);
    strcpy(TempTransformerPath, GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring);
    strcat(TempTransformerPath, "/");
    strcat(TempTransformerPath, "example/extensions/transformer.js");
    for (int i = 0; i < DependencyGraph->VerticesNum; i++) // Loop through each file in the graph
    {
        CJSONElement = GetSetting("transformers")->child;
        while (CJSONElement) // Loop through each transformer plugin
        {

            if (MatchGlob(DependencyGraph->Vertexes[i]->path, CJSONElement->string)) // Check if the file matches the glob pattern for the transformer
            {
                CJSONElement2 = CJSONElement->child;
                char *NewFileContents = NULL;
                while (CJSONElement2) // Loop through each plugin for the glob pattern
                {
                    // Gets the path to the plugin file
                    char *ExtensionPath = malloc(strlen(CJSONElement2->valuestring) + strlen(GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring) + 2);
                    strcpy(ExtensionPath, GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring);
                    strcat(ExtensionPath, CJSONElement2->valuestring);

                    char *TempBuffer = (char *)(*functionPTR)(ReadDataFromFile(DependencyGraph->Vertexes[i]->path), ExtensionPath, DependencyGraph->Vertexes[i]->path); // Call the transformer
                    if (TempBuffer != NULL)                                                                                                                             // Check that the file content has actually changed
                    {
                        if (NewFileContents != NULL)
                        {
                            free(NewFileContents);
                        }
                        NewFileContents = TempBuffer;
                    }
                    CJSONElement2 = CJSONElement2->next;
                }
                if (NewFileContents != NULL) // Save the transformed file contents
                {
                    char *NewPreprocessPath = EntryToPreprocessPath(DependencyGraph->Vertexes[i]->path);
                    DependencyGraph->Vertexes[i]->edge = NULL;

                    CreateFileWrite(NewPreprocessPath, NewFileContents);
                    DependencyGraph->Vertexes[i]->path = NewPreprocessPath;
                    CreateDependencyEdges(DependencyGraph->Vertexes[i], &DependencyGraph);
                }
            }
            CJSONElement = CJSONElement->next;
        }
    }

    return true;
}