//
// Created by eliasw on 05.05.26.
//

#ifndef BACHELORPROJECT_TINYOBJLOADERIMPL_H
#define BACHELORPROJECT_TINYOBJLOADERIMPL_H

// Increase tinyobj loader stream limit to 1 GiB
#ifndef TINYOBJLOADER_STREAM_READER_MAX_BYTES
#define TINYOBJLOADER_STREAM_READER_MAX_BYTES (size_t(1024) * size_t(1024) * size_t(1024))
#endif

#define TINYOBJLOADER_IMPLEMENTATION
#include "../lib/tiny_obj_loader.h"

#endif //BACHELORPROJECT_TINYOBJLOADERIMPL_H
