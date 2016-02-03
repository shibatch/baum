// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This software is in public domain. You can use and modify this code
// for any purpose without any obligation.


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#ifdef _MSC_VER
#include <Windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#include "helper.h"
#include "poly.h"

#define MAGIC_POLY12 0x7d5f11e6

#define N 12

typedef struct Poly12 {
  int magic;
  int data, size;
  float vertices[N][2];
  float dist[N];
} Poly12;

static const float dir[N][2] = {
  {  1.000,  0.000 },
  {  0.866,  0.500 },
  {  0.500,  0.866 },
  {  0.000,  1.000 },
  { -0.500,  0.866 },
  { -0.866,  0.500 },
  { -1.000,  0.000 },
  { -0.866, -0.500 },
  { -0.500, -0.866 },
  { -0.000, -1.000 },
  {  0.500, -0.866 },
  {  0.866, -0.500 },
};

Poly12 *initPoly12(int data) {
  Poly12 *thiz = (Poly12 *)calloc(1, sizeof(Poly12));
  thiz->magic = MAGIC_POLY12;
  thiz->data = data;

  for(int i=0;i<N;i++) {
    thiz->dist[i] = -1e+10f;
  }

  return thiz;
}

void Poly12_dispose(Poly12 *thiz) {
  assert(thiz->magic == MAGIC_POLY12);

  thiz->magic = 0;
  free(thiz);
}

int Poly12_size(Poly12 *thiz) {
  assert(thiz->magic == MAGIC_POLY12);

  return thiz->size;
}

void Poly12_add(Poly12 *thiz, float x, float y) {
  assert(thiz->magic == MAGIC_POLY12);

  thiz->size++;
  for(int i=0;i<N;i++) {
    float d = dir[i][0] * x + dir[i][1] * y;
    if (d > thiz->dist[i]) {
      thiz->dist[i] = d;
      thiz->vertices[i][0] = x;
      thiz->vertices[i][1] = y;
    }
  }
}

void Poly12_center(float *v, Poly12 *thiz) {
  assert(thiz->magic == MAGIC_POLY12);

  int c = 0;
  v[0] = 0; v[1] = 0;
  for(int i=0;i<N;i++) {
    if (thiz->dist[i] != -1e+10) {
      c++;
      v[0] += thiz->vertices[i][0];
      v[1] += thiz->vertices[i][1];
    }
  }
  v[0] *= 1.0f/c;
  v[1] *= 1.0f/c;
}

void Poly12_drawGL(Poly12 *thiz, int mode, float mag, float px, float py) {
  assert(thiz->magic == MAGIC_POLY12);

  glBegin(mode);

  for(int i=0;i<12;i++) {
    if (thiz->dist[i] == -1e+10f) continue;

    glVertex2f(thiz->vertices[i][0] * mag + px, thiz->vertices[i][1] * mag + py);
  }

  glEnd();
}
