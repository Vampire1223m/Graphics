#include "vertices.h"
#include "params.h"

const Vertex vertices[] = {
};

const uint32_t verticesCount = sizeof(vertices);

void drawPentagon(float cx, float cy, float r, Vertex* vertices){
    float x = TARGET_DISPLAY_WIDTH / 2;
    float y = TARGET_DISPLAY_HEIGHT / 2;

    vertices[sizeof(vertices)] = {      { cx , cy + r/y },
                                        { 1.0f, 0.0f, 0.0f },
                                        { 0.0f, 0.0f, 0.0f }}
    vertices[sizeof(vertices)+1] = {    { cx + (r * 1.732)/x, cy + (r/2)/y },
                                        { 0.0f, 1.0f, 0.0f },
                                        { 0.0f, 0.0f, 0.0f }}
    vertices[sizeof(vertices)+2] = {    { cx - (r * 1.732)/x, cy + (r/2)/y },
                                        { 0.0f, 0.0f, 1.0f },
                                        { 0.0f, 0.0f, 0.0f }}
    vertices[sizeof(vertices)+3] = {    { cx + (r/2)/x, cy - (r * 1.732)/y },
                                        { 1.0f, 0.0f, 1.0f },
                                        { 0.0f, 0.0f, 0.0f }}
    vertices[sizeof(vertices)+4] = {    { cx - (r/2)/x, cy - (r * 1.732)/y },
                                        { 1.0f, 1.0f, 0.0f },
                                        { 0.0f, 0.0f, 0.0f }}

}

