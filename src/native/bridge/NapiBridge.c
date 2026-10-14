/**
 * @file NapiBridge.c
 * @brief The bridge/interface between the Node.js N-api and the native C code.
 * Any functions/values that need to be accessible on the JS side are given and Napi wrapper function
 * and defined as part of a JS Napi module.
 * @see Documentation for N-api: https://nodejs.org/api/n-api.html
 */

#ifndef __EMSCRIPTEN__

#    include <node_api.h>
#    include <stdlib.h>

#    include "../codegen/BundleFiles.h"
#    include "../config.h"
#    include "../graph/DependencyGraph.h"

// Macro for declaring a napi method in a `napi_property_descriptor` struct
#    define DECLARE_NAPI_METHOD(name, function) {name, NULL, function, NULL, NULL, NULL, napi_default, NULL}

static void ThrowNapiError(napi_env env, napi_status status)
{

    // Get error message from env or use generic fallback message instead
    const napi_extended_error_info *error_info = NULL;
    napi_get_last_error_info(env, &error_info);
    const char *msg = (error_info == NULL || error_info->error_message == NULL) ? "Error occurred during N-API call."
                                                                                : error_info->error_message;

    /*
     * Check that an exception is not already pending/in progress.
     This must be called after napi_get_last_error_info as it returns a n-api status which overwrites any errors.
     */
    bool exceptionPending = false;
    napi_is_exception_pending(env, &exceptionPending);
    if (exceptionPending) {
        return;
    }

    // Throw type error if the error is a type error otherwise throw a general error
    switch (status) {
    case napi_invalid_arg:
    case napi_object_expected:
    case napi_string_expected:
    case napi_name_expected:
    case napi_function_expected:
    case napi_number_expected:
    case napi_boolean_expected:
    case napi_array_expected:
    case napi_bigint_expected:
    case napi_date_expected:
    case napi_arraybuffer_expected:
    case napi_detachable_arraybuffer_expected:
        napi_throw_type_error(env, NULL, msg);
        break;
    default:
        napi_throw_error(env, NULL, msg);
        break;
    }
}

// Macro for N-API call that will handle errors
#    define NAPI_CALL(env, call)             \
        do {                                 \
            napi_status status = (call);     \
            if (status != napi_ok) {         \
                ThrowNapiError(env, status); \
                return NULL;                 \
            }                                \
        } while (0)

// Macro for N-API call that will handle errors and perform a cleanup operation in case of an error
#    define NAPI_CALL_CLEANUP(env, call, cleanup) \
        do {                                      \
            napi_status status = (call);          \
            if (status != napi_ok) {              \
                ThrowNapiError(env, status);      \
                (cleanup);                        \
                return NULL;                      \
            }                                     \
        } while (0)

#    define NAPI_ASSERT_ARGC(env, actual, expected)                                                     \
        do {                                                                                            \
            if ((actual) != (expected)) {                                                               \
                napi_throw_type_error((env), NULL, "Incorrect number of arguments to N-API function."); \
                return NULL;                                                                            \
            }                                                                                           \
        } while (0)


// Declarations from Main.c
extern char *RebuildFiles(struct Graph *DependencyGraph, char *EncodedFiles, int FilesNum);
extern bool InitSettings(char *JSON);
extern int CheckNative();

static napi_value Napi_CheckNative(napi_env env, napi_callback_info info)
{
    int result = CheckNative();
    napi_value ret;
    NAPI_CALL(env, napi_create_int32(env, (int32_t)result, &ret));

    return ret;
}

static napi_value Napi_InitSettings(napi_env env, napi_callback_info info)
{
    // 1 arg (stringified json)
    size_t argc = 1;

    napi_value args[1];

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
    NAPI_ASSERT_ARGC(env, argc, 1);

    // Get size of JSON string
    size_t strSize;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], NULL, 0, &strSize));


    // Convert JS JSON string to C string
    char *json = malloc(strSize + sizeof(char));  // Add null terminator
    if (json == NULL) {
        napi_throw_error(env, NULL, "Failed to allocate memory!");
        return NULL;
    }
    NAPI_CALL_CLEANUP(env, napi_get_value_string_utf8(env, args[0], json, strSize + 1, &strSize), free(json));

    bool success = InitSettings(json);
    free(json);

    // Create JS boolean to return
    napi_value ret;
    NAPI_CALL(env, napi_get_boolean(env, success, &ret));


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
    NAPI_CALL(env, napi_create_external(env, (void *)graph, NULL, NULL, &result));
    return result;
}

static napi_value Napi_topological_sort(napi_env env, napi_callback_info info)
{
    // 1 arg (dependency graph pointer)
    size_t argc = 1;

    napi_value args[1];

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
    NAPI_ASSERT_ARGC(env, argc, 1);

    struct Graph *graph;
    NAPI_CALL(env, napi_get_value_external(env, args[0], (void **)&graph));

    topological_sort(graph);

    return NULL;
}

static napi_value Napi_BundleFiles(napi_env env, napi_callback_info info)
{
    // 1 arg (dependency graph pointer)
    size_t argc = 1;
    napi_value args[1];

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
    NAPI_ASSERT_ARGC(env, argc, 1);

    struct Graph *graph;
    NAPI_CALL(env, napi_get_value_external(env, args[0], (void **)&graph));

    bool success = BundleFiles(graph);

    // Return success as a JS boolean
    napi_value ret;
    NAPI_CALL(env, napi_get_boolean(env, success, &ret));

    return ret;
}

static napi_value Napi_RebuildFiles(napi_env env, napi_callback_info info)
{
    // 3 args (dependency graph pointer, serialised file path string, number of file paths)
    size_t argc = 3;
    napi_value args[3];

    // Get C value from JS napi_values
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));
    NAPI_ASSERT_ARGC(env, argc, 3);

    struct Graph *graph;
    NAPI_CALL(env, napi_get_value_external(env, args[0], (void **)&graph));

    // Get pathsNum from arguments and validate
    int32_t pathsNum;
    NAPI_CALL(env, napi_get_value_int32(env, args[2], &pathsNum));
    if (pathsNum < 0) {
        napi_throw_range_error(env, NULL, "Number of file paths to rebuild cannot be negative!");
        return NULL;
    }

    /*
     * Get serialisedPaths from string napi_value
     */
    char *serialisedPaths;
    size_t serialisedPathsLength;

    // Get size of serialised path string
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[1], NULL, 0, &serialisedPathsLength));

    // Alloc with null terminator added (not included from napi_get_value_string_utf8)
    serialisedPaths = (char *)malloc(serialisedPathsLength + sizeof(char));
    if (serialisedPaths == NULL) {
        napi_throw_error(env, NULL, "Failed to allocate memory!");
        return NULL;
    }

    // Copy string in serialisedPaths buffer
    NAPI_CALL_CLEANUP(
        env,
        napi_get_value_string_utf8(env, args[1], serialisedPaths, serialisedPathsLength + 1, &serialisedPathsLength),
        free(serialisedPaths));

    char *retPaths = RebuildFiles(graph, serialisedPaths, pathsNum);
    free(serialisedPaths);

    /*
     * RebuildFiles cannot currently return NULL but it is safer to check anyway incase this is changed in future.
     */
    if (retPaths == NULL) {
        napi_throw_error(env, NULL, "RebuildFiles failed (returned NULL)!");
        return NULL;
    }

    // Return serialised array of strings representing modified files
    napi_value ret;
    NAPI_CALL_CLEANUP(env, napi_create_string_utf8(env, retPaths, NAPI_AUTO_LENGTH, &ret), free(retPaths));
    free(retPaths);
    return ret;
}


// Initialises Napi module and registers functions to be callable by JS
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("CheckNative", Napi_CheckNative),
        DECLARE_NAPI_METHOD("InitSettings", Napi_InitSettings),
        DECLARE_NAPI_METHOD("InitFileTypes", Napi_InitFileTypes),
        DECLARE_NAPI_METHOD("CreateGraph", Napi_CreateGraph),
        DECLARE_NAPI_METHOD("topological_sort", Napi_topological_sort),
        DECLARE_NAPI_METHOD("BundleFiles", Napi_BundleFiles),
        DECLARE_NAPI_METHOD("RebuildFiles", Napi_RebuildFiles),
    };

    size_t propertiesCount = sizeof(properties) / sizeof(properties[0]);

    NAPI_CALL(env, napi_define_properties(env, exports, propertiesCount, properties));

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);

#endif