#ifndef ASSET_H
#define ASSET_H
#include <cstdint>
#include <stdio.h>

typedef enum
{
    ASSET_HTML,
    ASSET_CSS,
    ASSET_JS,
    ASSET_TS,
    ASSET_PNG,
    ASSET_JPEG,
    ASSET_WEBP,
    ASSET_SVG,
    // Undefined image asset
    ASSET_IMAGE,
    // Any undefined asset
    ASSET_RAW_BLOB
} AssetType;

struct Asset
{
    AssetType type;
    const char *filepath;
    const char *absPath;

    // Raw file data
    uint8_t *buffer;
    // Size of the file in bytes
    size_t size;

    // Array of pointers to dependencies
    Asset **dependencies;
    size_t dependenciesCount;
    size_t dependenciesCapacity;
}

#endif // !ASSET_H
