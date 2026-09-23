#ifndef VERTICES_H
#define VERTICES_H

#include "vertex.h"
#include <stdint.h>

typedef struct {
    float projection[16];
    float transform[16];
} uniformBufferObj;

extern uniformBufferObj ubo;
extern const Vertex vertices[];
extern Vertex frameVertices[];
extern const uint32_t indices[];
extern const uint32_t verticesCount;
extern const uint32_t indicesCount;

#endif
