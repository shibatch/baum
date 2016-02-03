// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This file is part of BAUM software library. BAUM library is in
// public domain. You can use and modify this code for any purpose
// without any obligation.

#if defined(__cplusplus)
extern "C" {
#endif
  typedef struct baum_t baum_t;

  baum_t *baum_init(int did, int iw, int ih);
  baum_t *baum_init2(struct _cl_device_id *device0, struct _cl_context *context0, int iw, int ih);
  void baum_dispose(baum_t *thiz);
  int baum_enqueueTask(baum_t *thiz, void *resultPtr, size_t bufSize, const void *imgPtr, const int iw, const int ih, const int ws);
  int baum_poll(baum_t *thiz, int waitFlag);
  int baum_queueLen(baum_t *thiz);
  void baum_createPlan(baum_t *thiz, const char *path, void *resultPtr, size_t bufSize, const void *imgPtr, const int iw, const int ih, const int ws);
  int baum_loadPlan(baum_t *thiz, const char *path);

  char *baum_getDeviceName(baum_t *thiz);
  void *baum_malloc(baum_t *thiz, size_t z);
  void baum_free(baum_t *thiz, void *ptr);

  void baum_fprintMarkerSVG(FILE *fp, int data, double radius, double posx, double posy);
#if defined(__cplusplus)
}
#endif
