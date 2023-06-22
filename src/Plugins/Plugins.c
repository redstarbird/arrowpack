#include "Plugins.h"

// Executes a plugin
bool EMSCRIPTEN_KEEPALIVE ExecutePlugin(struct Graph *DependencyGraph, char *(*functionPTR)(char *, char *, char *), int pluginIndex)
{
    cJSON *CJSONElement = NULL;
    cJSON *CJSONElement2 = NULL;
    for (int i = 0; i < DependencyGraph->VerticesNum; i++) // Loop through each dependency in the graph
    {
        switch (pluginIndex) // Finds which plugin section to load from
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

        while (CJSONElement) // Run every plugin in array
        {

            if (MatchGlob(DependencyGraph->Vertexes[i]->path, CJSONElement->string)) // Check if the plugin glob matches the file
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

                    if (pluginIndex == 3) // File path will be the exit path for postprocessing plugins
                    {
                        FilePath = EntryToExitPath(DependencyGraph->Vertexes[i]->path);
                    }
                    else
                    {
                        FilePath = DependencyGraph->Vertexes[i]->path;
                    }

                    if (pluginIndex == 3) // Post processing plugins
                    {
                        Result = (char *)(*functionPTR)(ReadDataFromFile(DependencyGraph->Vertexes[i]->path),
                                                        ExtensionPath,
                                                        DependencyGraph->Vertexes[i]->path); // Run the javascript plugin

                        if (Result != NULL)
                        {
                            if (NewFileContents != NULL)
                            {
                                free(NewFileContents);
                            }
                            NewFileContents = Result; // Set the new file contents
                        }
                    }
                    else if (pluginIndex == 2) // Validator plugins
                    {
                        if ((*functionPTR)(ReadDataFromFile(DependencyGraph->Vertexes[i]->path), ExtensionPath, DependencyGraph->Vertexes[i]->path) != NULL)
                        {
                            ThrowFatalError("Internal error occured while runing validators\n");
                        }
                    }
                    else // Resolver plugins
                    {
                        CreateWarning("Resolvers not implemented yet\n");
                    }
                    CJSONElement2 = CJSONElement2->next;
                }
                if (pluginIndex == 3) // Save post-processed file
                {
                    if (NewFileContents != NULL)
                    {

                        DependencyGraph->Vertexes[i]->edge = NULL;

                        CreateFileWrite(EntryToExitPath(DependencyGraph->Vertexes[i]->path), NewFileContents);
                        CreateDependencyEdges(DependencyGraph->Vertexes[i], &DependencyGraph);
                    }
                }
            }
            CJSONElement = CJSONElement->next;
        }
    }
    return true;
}