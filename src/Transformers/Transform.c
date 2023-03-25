#include "Transform.h"
// extern void JSTransform(char *FileName);
bool EMSCRIPTEN_KEEPALIVE TransformFiles(struct Graph *DependencyGraph, char *(*functionPTR)(char *, char *))
{
    // int test = EM_ASM_INT({ return Module.exports.JSTransform($0); }, "hello world!");
    //  JSTransform("hello world!");
    // printf("Received %i!\n", test);
    cJSON *CJSONElement = NULL;
    cJSON *CJSONElement2 = NULL;
    printf("CWD: %s\n", GetSetting("INTERNAL_CWD")->valuestring);
    char *TempTransformerPath = malloc(strlen(GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring) + 1000);
    strcpy(TempTransformerPath, GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring);
    strcat(TempTransformerPath, "/");
    strcat(TempTransformerPath, "example/extensions/transformer.js");
    for (int i = 0; i < DependencyGraph->VerticesNum; i++)
    {
        CJSONElement = GetSetting("transformers")->child;
        while (CJSONElement)
        {
            printf("The key is %s\n", CJSONElement->string);

            if (MatchGlob(DependencyGraph->Vertexes[i]->path, CJSONElement->string))
            {
                printf("File path %s matches %s\n", DependencyGraph->Vertexes[i]->path, CJSONElement->string);
                CJSONElement2 = CJSONElement->child;
                char *NewFileContents = NULL;
                while (CJSONElement2)
                {
                    char *ExtensionPath = malloc(strlen(CJSONElement2->valuestring) + strlen(GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring) + 2);
                    strcpy(ExtensionPath, GetSetting("INTERNAL_FULL_CONFIG_PATH")->valuestring);
                    strcat(ExtensionPath, CJSONElement2->valuestring);
                    char *TempBuffer = (char *)(*functionPTR)(ReadDataFromFile(DependencyGraph->Vertexes[i]->path), ExtensionPath);
                    if (TempBuffer != NULL)
                    {
                        if (NewFileContents != NULL)
                        {
                            free(NewFileContents);
                        }
                        NewFileContents = TempBuffer;
                    }
                    CJSONElement2 = CJSONElement2->next;
                }
                if (NewFileContents != NULL)
                {
                    char *NewPreprocessPath = EntryToPreprocessPath(DependencyGraph->Vertexes[i]->path);
                    DependencyGraph->Vertexes[i]->edge = NULL;

                    // char *FileExtension = GetFileExtension(DependencyGraph->SortedArray[i]->path);
                    // NewPreprocessPath = realloc(NewPreprocessPath, strlen(NewPreprocessPath) + strlen(FileExtension) + 2);
                    // strcat(NewPreprocessPath, ".");
                    // strcat(NewPreprocessPath, FileExtension);
                    CreateFileWrite(NewPreprocessPath, NewFileContents);
                    DependencyGraph->Vertexes[i]->path = NewPreprocessPath;
                    CreateDependencyEdges(DependencyGraph->Vertexes[i], &DependencyGraph);
                }
            }
            else
            {
                printf("File path %s DOESNT MATCH!!!! %s\n", DependencyGraph->Vertexes[i]->path, CJSONElement->string);
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

    /*char *ReturnString = (char *)EM_ASM_PTR({
        Module.exports.CJSfunction();
        var TempStr = import($0)($1);
        console.log(TempStr);
        var str = "Hello World!\\0";
        var lengthBytes = lengthBytesUTF8(str) + 1;
        var stringOnWasmHeap = Module._malloc(lengthBytes);
        stringToUTF8(str, stringOnWasmHeap, lengthBytes);
        return stringOnWasmHeap;
    },
                                            TempTransformerPath, "Hamburger epic!");
    printf("Returned string: %s\n", ReturnString);*/
    return true;
}