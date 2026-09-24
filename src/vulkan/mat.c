#include <string.h>
#include "mat.h"
#include "math.h"

void mat4Identity(float *mat){

    for (int i = 0; i < 16; i++) mat[i] = 0.0f;

    mat[0] = 1.0f;  // 1 0 0 0
    mat[5] = 1.0f;  // 0 1 0 0
    mat[10] = 1.0f; // 0 0 1 0
    mat[15] = 1.0f; // 0 0 0 1

}

void mat4Translate(float *mat, float x, float y){

    mat4Identity(mat);

    mat[3] = x; // 1 0 0 x
    mat[7] = y; // 0 1 0 y

}

void mat4Scale(float *mat, float x, float y){

    mat4Identity(mat);

    mat[0] = x; // x 0 0 0
    mat[5] = y; // 0 y 0 0

}

void mat4Rotate(float *mat, float angle){

    angle = angle * M_PI/180;

    float c = cosf(angle);
    float s = sinf(angle);

    mat4Identity(mat);

    mat[0] = c; // c -s  0  0
    mat[1] = -s;// s  c  0  0
    mat[4] = s; // 0  0  1  0
    mat[5] = c; // 0  0  0  1

}

void mat4Multiply(float *R, float *a, float *b){

    float result[16];

    for (int i = 0; i < 16; i++){
        for (int j = 0; j < 16; j++){
            result[i + (4*j)] = a[i]*b[4*j] + 
                                a[i + 4]*b[1 + 4*j] + 
                                a[i + 8]*b[2 + 4*j] + 
                                a[i + 12]*b[3 + 4*j];
        }
    }

    memcpy(R, result, sizeof(result));

}