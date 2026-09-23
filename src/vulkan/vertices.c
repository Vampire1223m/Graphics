#include "vertices.h"
#include "params.h"
#include <strings.h>

const Vertex vertices[] = {
    {
        { -0.5f, -0.5f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }
    },
    {
        { 0.5f,  0.5f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }
    },
    {
        {-0.5f,  0.5f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f }
    },
    {
        {0.5f,  -0.5f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f }
    }
};

uniformBufferObj ubo = {.transform = 1,0,0,0,
                                     0,1,0,0,
                                     0,0,1,0,
                                     0,0,0,1,
                        .projection = 2/TARGET_DISPLAY_WIDTH,0,0,0,
                                      0,2/TARGET_DISPLAY_HEIGHT,0,0,
                                      0,0,1,0,
                                      0,0,0,1};

const uint32_t verticesCount = sizeof(vertices)/sizeof(Vertex);

Vertex frameVertices[4];

//frameVertices = vertices;


const uint32_t indices[] = {
    0,1,2,
    0,1,3
};

const uint32_t indicesCount = sizeof(indices)/sizeof(uint32_t);
