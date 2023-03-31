#include "Plugins.h"
// extern void JSTransform(char *FileName);
bool EMSCRIPTEN_KEEPALIVE ExecutePlugin(struct Graph *DependencyGraph, char *(*functionPTR)(char *, char *, char *), int pluginIndex)
{
    // int test = EM_ASM_INT({ return Module.exports.JSTransform($0); }, "hello world!");
    //  JSTransform("hello world!");
    // printf("Received %i!\n", test);
    cJSON *CJSONElement = NULL;
    cJSON *CJSONElement2 = NULL;
    for (int i = 0; i < DependencyGraph->VerticesNum; i++)
    {
        switch (pluginIndex)
        {
        case 1:
            CJSONElement = GetSetting("resolvers")->child;
            break;
        case 2:
            CJSONElement = GetSetting("validators")->child;
            break;
        case 3:
            CJSONElement = GetSetting("postProcessors")->child;
            break;
        default:
            ThrowFatalError("Internal error, pluginIndex is not valid...");
            break;
        }

        while (CJSONElement)
        {

            if (MatchGlob(DependencyGraph->Vertexes[i]->path, CJSONElement->string))
            {
                CJSONElement2 = CJSONElement->child;
                char *NewFileContents = NULL;
                while (CJSONElement2)
                {
                    char *Result;
                    char *ExtensionPath = malloc(strlen(CJSONElement2->valuestring) + strlen(GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring) + 2);
                    strcpy(ExtensionPath, GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring);
                    strcat(ExtensionPath, CJSONElement2->valuestring + 2 * (CJSONElement2->valuestring[0] == '.' && CJSONElement2->valuestring[1] == '/'));

                    char *FilePath;

                    if (pluginIndex == 3)
                    {
                        FilePath = EntryToExitPath(DependencyGraph->Vertexes[i]->path);
                    }
                    else
                    {
                        FilePath = DependencyGraph->Vertexes[i]->path;
                    }

                    if (pluginIndex == 3)
                    {
                        Result = (char *)(*functionPTR)(ReadDataFromFile(DependencyGraph->Vertexes[i]->path), ExtensionPath, DependencyGraph->Vertexes[i]->path);
                        if (Result != NULL)
                        {
                            if (NewFileContents != NULL)
                            {
                                free(NewFileContents);
                            }
                            NewFileContents = Result;
                        }
                    }
                    else if (pluginIndex == 2)
                    {
                        printf("running validators\n");
                        if ((*functionPTR)(ReadDataFromFile(DependencyGraph->Vertexes[i]->path), ExtensionPath, DependencyGraph->Vertexes[i]->path) != NULL)
                        {
                            ThrowFatalError("Internal error occured while runing validators\n");
                        }
                    }
                    else
                    {
                        CreateWarning("Resolvers not implemented yet\n");
                    }
                    CJSONElement2 = CJSONElement2->next;
                }
                if (pluginIndex == 3)
                {
                    if (NewFileContents != NULL)
                    {

                        DependencyGraph->Vertexes[i]->edge = NULL;

                        // char *FileExtension = GetFileExtension(DependencyGraph->SortedArray[i]->path);
                        // NewPreprocessPath = realloc(NewPreprocessPath, strlen(NewPreprocessPath) + strlen(FileExtension) + 2);
                        // strcat(NewPreprocessPath, ".");
                        // strcat(NewPreprocessPath, FileExtension);
                        CreateFileWrite(EntryToExitPath(DependencyGraph->Vertexes[i]->path), NewFileContents);
                        CreateDependencyEdges(DependencyGraph->Vertexes[i], &DependencyGraph);
                    }
                }
            }
            CJSONElement = CJSONElement->next;
        }
        /*
                cJSON_ObjectForEach(CJSONElement, )
                {
                    printf("CJSON KEY: %s\n", CJSONElement->valuestring);
                    char *ReturnString = (char *)(*functionPTR)("hamburger743.543", CJSONElement->valuestring);
                    printf("Received %s from JS\n", ReturnString);
                };*/
    }
    return true;
}