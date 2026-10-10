/**
 * @file NapiBridge.c
 * @brief The bridge/interface between the Node.js N-api and the native C code.
 * Any functions/values that need to be accessible on the JS side are given and Napi wrapper function
 * and defined as part of a JS Napi module
 */

#ifndef __EMSCRIPTEN__

#    include <node_api.h>
#    include <stdlib.h>

#    include "../config.h"
#    include "../graph/DependencyGraph.h"
#    include "BundleFiles.h"

// Macro for declaring a napi method in a `napi_property_descriptor` struct
#    define DECLARE_NAPI_METHOD(name, function) {name, NULL, function, NULL, NULL, NULL, napi_default, NULL}


// Declarations from Main.c
extern char *RebuildFiles(struct Graph *DependencyGraph, char *EncodedFiles, int FilesNum);
extern bool InitSettings(char *JSON);


static napi_value Napi_InitSettings(napi_env env, napi_callback_info info)
{
    // 1 arg (stringified json)
    size_t argc = 1;

    napi_value args[1];

    napi_status status;

    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    if (status != napi_ok) {
        // Todo
    }

    // Get size of JSON string
    size_t strSize;
    status = napi_get_value_string_utf8(env, args[0], NULL, 0, &strSize);
    if (status != napi_ok) {
        // Todo
    }

    // Convert JS JSON string to C string
    char *json = malloc(strSize + sizeof(char));  // Add null terminator
    status = napi_get_value_string_utf8(env, args[0], json, 0, &strSize);
    if (status != napi_ok) {
        // Todo
    }

    bool success = InitSettings(json);
    free(json);

    // Create JS boolean to return
    napi_value ret;
    status = napi_get_boolean(env, success, &ret);
    if (status != napi_ok) {
        // Todo
    }

    return ret;
}

static napi_value Napi_InitFileTypes(napi_env env, napi_callback_info info)
{
    InitFileTypes();
    return NULL;
}

static napi_value Napi_CreateGraph(napi_env env, napi_callback_info info)
{
    struct Graph *graph = CreateGraph();

    // napi_external is used to store the raw pointer to the dependency graph
    napi_value result;
    napi_create_external(env, (void *)&graph, NULL, NULL, &result);
    return result;
}

static napi_value Napi_topological_sort(napi_env env, napi_callback_info info)
{
    // 1 arg (dependency graph pointer)
    size_t argc = 1;

    napi_value args[1];

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct Graph *graph;
    napi_get_value_external(env, args[0], (void **)&graph);

    topological_sort(graph);

    return NULL;
}

static napi_value Napi_BundleFiles(napi_env env, napi_callback_info info)
{
    // 1 arg (dependency graph pointer)
    size_t argc = 1;
    napi_value args[1];

    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    struct Graph *graph;
    napi_get_value_external(env, args[0], (void **)&graph);

    bool success = BundleFiles(graph);

    // Return success as a JS boolean
    napi_value ret;
    napi_get_boolean(env, success, &ret);

    return ret;
}

static napi_value Napi_RebuildFiles(napi_env env, napi_callback_info info)
{
    // 3 args (dependency graph pointer, serialised file path string, number of file paths)
    size_t argc = 3;
    napi_value args[3];

    // Get C value from JS napi_values
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);


    struct Graph *graph;
    napi_get_value_external(env, args[0], (void **)&graph);

    char *serialisedPaths;
    size_t serialisedPathsLength;
    // Get size of serialised path string
    napi_get_value_string_utf8(env, args[1], NULL, 0, &serialisedPathsLength);
    // Alloc with null terminator added (not included from napi_get_value_string_utf8)
    serialisedPaths = (char *)malloc(serialisedPathsLength + sizeof(char));
    // Copy string in serialisedPaths buffer
    napi_get_value_string_utf8(env, args[1], serialisedPaths, 0, &serialisedPathsLength);

    int pathsNum;
    napi_get_value_int32(env, args[2], &pathsNum);

    char *retPaths = RebuildFiles(graph, serialisedPaths, pathsNum);

    // Return success as JS boolean
    napi_value ret;
    napi_create_string_utf8(env, retPaths, strlen(retPaths), &ret);
    return ret;
}


// Initialises Napi module and registers functions to be callable by JS
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("InitSettings", Napi_InitSettings),
        DECLARE_NAPI_METHOD("InitFileTypes", Napi_InitFileTypes),
        DECLARE_NAPI_METHOD("CreateGraph", Napi_CreateGraph),
        DECLARE_NAPI_METHOD("TopologicalSort", Napi_topological_sort),
        DECLARE_NAPI_METHOD("BundleFiles", Napi_BundleFiles),
        DECLARE_NAPI_METHOD("RebuildFiles", Napi_RebuildFiles),
    };

    size_t propertiesCount = sizeof(properties) / sizeof(properties[0]);

    napi_status status = napi_define_properties(env, exports, propertiesCount, &properties);

    if (status != napi_ok) {
        // Todo
    }

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);

#endif