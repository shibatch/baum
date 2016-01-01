// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This software is in public domain. You can use and modify this code
// for any purpose without any obligation.


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct Poly12 Poly12;

Poly12 *initPoly12(int data);
void Poly12_dispose(Poly12 *thiz);
int Poly12_size(Poly12 *thiz);
void Poly12_add(Poly12 *thiz, float x, float y);
void Poly12_center(float *v, Poly12 *thiz);
void Poly12_drawGL(Poly12 *thiz, int mode, float mag, float px, float py);

#if defined(__cplusplus)
}
#endif
