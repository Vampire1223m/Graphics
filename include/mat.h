#ifndef MAT_H
#define MAT_H

void mat4Identity(float *mat);
void mat4Translate(float *mat, float x, float y);
void mat4Scale(float *mat, float x, float y);
void mat4Rotate(float *mat, float angle);
void mat4Multiply(float *mat, float *a, float *b);

#endif