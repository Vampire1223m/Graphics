#include "vertices.h"

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

const uint32_t verticesCount = sizeof(vertices);

const uint32_t indices[] = {
    0,1,2,
    0,1,3
};

const uint32_t indicesCount = sizeof(indices);
