cmake_minimum_required(VERSION 3.10)
project(arrowpack VERSION 0.2.4 LANGUAGES C)

# All C source files
set(SOURCES
    src/native/external/cJSON/cJSON.c
    src/native/graph/DependencyGraph.c
    src/native/util/StringUtil.c
    src/native/util/RegexUtil.c
    src/native/graph/FindDependencies.c
    src/native/config.c
    src/native/util/ProblemHandler.c
    src/native/util/TextColors.c
    src/native/util/FileUtil.c
    src/native/util/IntUtil.c
    src/native/minify/HTMLMinifier.c
    src/native/graph/FileTypesHandler.c
    src/native/util/Stack.c
    src/native/codegen/BundleFiles.c
    src/native/util/ProgressBar.c
    src/native/codegen/StringShiftHandler.c
    src/native/minify/JSMinifier.c
    src/native/bridge/plugins/Transform.c
    src/native/bridge/plugins/Plugins.c
    src/native/bridge/JSImportedFunctions.c
    src/native/bridge/Main.c
)


# Default build type to Release
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# ----------------------------------
# Fallback target: Emscripten (WASM)
# ----------------------------------

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    message(STATUS "Building WebAssembly targen with Emscripten")

    add_executable(CFunctions ${SOURCES})

    # Force output to have .js extension so Emscripten generates WebAssembly + JS wrapper
    set_target_properties(CFunctions PROPERTIES
        OUTPUT_NAME "CFunctions"
        SUFFIX ".js"
    )

    # Base Emscripten compiler and linker flags
    target_link_options(CFunctions PRIVATE
        # The C source does not contain a "main" entry point (build as a library)
        "--no-entry"
        # Plugin JS functions that can be called from C
        "--js-library" "${CMAKE_CURRENT_SOURCE_DIR}/src/node/misc/JSCLibrary.js"
        # Allow for addFunction to be used at runtime to dynamically create C function pointers
        "-sALLOW_TABLE_GROWTH=1"
        # Optimise built JS for node.js (remove browser-specific code)
        "-sENVIRONMENT=node"
        # Export commonJS instead of ES6
        "-sEXPORT_ES6=0"
        # Wraps generated JS into a module
        "-sMODULARIZE=1"
        # Exposes malloc to JS
        "-sEXPORTED_FUNCTIONS=_malloc"
        # Expose built-in Emscripten functions to JS
        "-sEXPORTED_RUNTIME_METHODS=['ccall','addFunction','lengthBytesUTF8','stringToUTF8','UTF8ToString','stringToNewUTF8']"
        # Allow C to read directly from OS by replacing C IO functions with Node.js fs calls
        "-sNODERAWFS=1"
        # Optimise compiled Wasm
        "-sBINARYEN=1"
        # Run standard library cleanups
        "-sEXIT_RUNTIME=1"
        # Allows Wasm memory to grow dynamically
        "-sALLOW_MEMORY_GROWTH=1"
    )

    # Handle Debug (dev) mode

    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        message(STATUS "Dev mode enabled (Debug build)")

        # Debugging compiler flags
        target_compile_options(CFunctions PRIVATE
            -O0 # Ensure no optimisations happen
            -fsanitize=undefined
            -fsanitize=address
            -g3
        )

        # Debugging linker flags
        target_link_options(CFunctions PRIVATE
            -O0
            "--profiling"
            "-sRUNTIME_DEBUG=1"
            "-fsanitize=undefined"
            "-fsanitize=address"
            "-sLLD_REPORT_UNDEFINED=1"
            "-g3"
            "-gsource-map"
            "-sSTACK_OVERFLOW_CHECK=2"
            "-sASSERTIONS=2"
        )

    else() # Handle release mode (max optimisations)
        message(STATUS "Release mode enabled")

        target_compile_options(CFunctions PRIVATE -O3)
        target_link_options(CFunctions PRIVATE
            "-O3"
            "-sELIMINATE_DUPLICATE_FUNCTIONS=1"
            )

    endif()

# ---------------------------------------
# Default target: Native Node-API (N-API)
# ---------------------------------------
else()
    message(STATUS "Building Native Node-API target")

    add_library(arrowpack_native SHARED ${SOURCES} ${CMAKE_JS_SRC})

    set_target_properties(
        arrowpack_native PROPERTIES
        PREFIX ""
        OUTPUT_NAME "arrowpack"
        SUFFIX ".node"

        # Ensure arrowpack.node is directly in the build/native directory
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"

        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
    )

    target_link_libraries(arrowpack_native ${CMAKE_JS_LIB})

    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(arrowpack_native PRIVATE -O0 -g3)
    else()
        target_compile_options(arrowpack_native PRIVATE -O3)
    endif()
endif()