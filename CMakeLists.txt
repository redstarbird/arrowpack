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
    "--no-entry" # The C source does not contain a "main" entry point
    "--js-library" "${CMAKE_CURRENT_SOURCE_DIR}/src/js/JSCLibrary.js"
    "-sALLOW_TABLE_GROWTH=1"
    "-sENVIRONMENT=node"
    "-sEXPORT_ES6=0"
    "-sMODULARIZE=1"
    "-sUSE_ES6_IMPORT_META=0"
    "-sEXPORTED_FUNCTIONS=_malloc"
    "-sEXPORTED_RUNTIME_METHODS=['ccall','addFunction','lengthBytesUTF8','stringToUTF8','UTF8ToString','stringToNewUTF8']"
    "-sNODERAWFS=1"
    "-sBINARYEN=1"
    "-sEXIT_RUNTIME=1"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sEXPORT_ALL=1"
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
        -fsanitize=undefined
        -fsanitize=address
        -g3
    )

    # Debugging linker flags
    target_link_options(CFunctions PRIVATE
        "--profiling"
        "-sRUNTIME_DEBUG=1"
        "-fsanitize=undefined"
        "-fsanitize=address"
        "-sLLD_REPORT_UNDEFINED=1"
        "-g3"
        "-sSTACK_OVERFLOW_CHECK=2"
        "-sASSERTIONS=2"
    )

else() # Handle release mode (max optimisations)
    message(STATUS "Release mode enabled")

    target_compile_options(CFunctions PRIVATE -O3)
    target_link_options(CFunctions PRIVATE
        "-O3"
        "-sELIMINATE_DUPLICATE_FUNCTIONS=1"
        "-sMINIFY_HTML=1"
        )

endif()
