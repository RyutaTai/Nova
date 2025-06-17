#pragma once

#include <string>
#include "../../tinygltf-release/tiny_gltf.h"

inline bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*,
    int, int, const unsigned char*, int, void*)
{
    return true;
}