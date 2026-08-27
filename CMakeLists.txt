cmake_minimum_required(VERSION 3.10)
project(arrowpack VERSION 0.2.4 LANGUAGES C)

# Make sure that the project is being built via the Emscripten toolchain
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    message(FATAL_ERROR "This project must be compiled with Emscripten! Please check the project build instructions.")
endif()

# All C source files
set(SOURCES
    src/C/cJSON/cJSON.c
    src/DependencyGraph/DependencyGraph.c
    src/C/StringRelatedFunctions.c
    src/Regex/RegexFunctions.c
    src/DependencyGraph/FindDependencies.c
    src/SettingsSingleton/settingsSingleton.c
    src/C/ProblemHandler.c
    src/C/TextColors.c
    src/C/FileHandler.c
    src/C/IntFunctions.c
    src/Minifiers/HTMLMinifier.c
    src/C/FileTypesHandler.c
    src/C/Stack.c
    src/C/BundleFiles.c
    src/C/ProgressBar.c
    src/C/StringShiftHandler.c
    src/Minifiers/JSMinifier.c
    src/Transformers/Transform.c
    src/Plugins/Plugins.c
    src/C/JSImportedFunctions.c
)

add_executable(CFunctions src/Main.c ${SOURCES})

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
    "--js-library" "${CMAKE_CURRENT_SOURCE_DIR}/src/js/JSCLibrary.js"
    # Allow for addFunction to be used at runtime to dynamically create C function pointers
    "-sALLOW_TABLE_GROWTH=1"
    # Optimise built JS for node.js (remove browser-specific code)
    "-sENVIRONMENT=node"
    # Export commonJS instead of ES6
    "-sEXPORT_ES6=0"
    "-sUSE_ES6_IMPORT_META=0"
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

# Default build type to Release
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

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
