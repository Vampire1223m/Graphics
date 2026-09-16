#include "vertices.h"

const Vertex vertices[] = {
    {
        { 0.0f, -0.5f },
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
    }
};

const uint32_t verticescount = 3;