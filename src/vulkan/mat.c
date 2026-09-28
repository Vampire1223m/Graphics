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

    float temp[16];
    float result[16];

    mat4Identity(temp);

    temp[12] = x; // 1 0 0 x
    temp[13] = y; // 0 1 0 y

    mat4Multiply(result, mat, temp);

    memcpy(mat, result, sizeof(result));

}

void mat4Scale(float *mat, float x, float y){

    float temp[16];
    float result[16];

    mat4Identity(temp);

    temp[0] = x; // x 0 0 0
    temp[5] = y; // 0 y 0 0

    mat4Multiply(result, mat, temp);

    memcpy(mat, result, sizeof(result));

}

void mat4Rotate(float *mat, float angle){

    angle = angle * M_PI/180;

    float c = cosf(angle);
    float s = sinf(angle);

    float temp[16];
    float result[16];

    mat4Identity(temp);

    temp[0] = c; // c -s  0  0
    temp[4] = -s;// s  c  0  0
    temp[1] = s; // 0  0  1  0
    temp[5] = c; // 0  0  0  1

    mat4Multiply(result, mat, temp);

    memcpy(mat, result, sizeof(result));

}

void mat4Multiply(float *R, float *a, float *b){

    float result[16];

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            result[i + (4*j)] = a[i]*b[4*j] + 
                                a[i + 4]*b[1 + 4*j] + 
                                a[i + 8]*b[2 + 4*j] + 
                                a[i + 12]*b[3 + 4*j];
        }
    }

    memcpy(R, result, sizeof(result));

}
