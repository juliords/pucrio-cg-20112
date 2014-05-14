#include "color.h"
#include "algebra.h"

void myoglInitCanvas(int w, int h);

void myoglClose(char* name);

void myoglLight(float* ambient,float* diffuse,float* specular,float* position);

void myoglPerspective(float fovy, float aspect, float n, float f);

void myoglLookAt(float ex, float ey, float ez, float cx, float cy, float cz, float upx, float upy, float upz);

void myoglMaterial( float* ambiente, float* diffuse, float* specular, float* shininess);

void myoglDrawTriang(float* normal, float* v0, float* v1, float* v2);

// testes
void myoglTest( float x, float y, float z );
