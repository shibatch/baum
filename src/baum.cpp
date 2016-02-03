// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This file is part of BAUM software library. BAUM library is in
// public domain. You can use and modify this code for any purpose
// without any obligation.


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <malloc.h>

#include <assert.h>

#include <CL/cl.h>
#include <cv.h>
#include <highgui.h>

#include "helper.h"
#include "oclhelper.h"
#include "baum_cl.h"

#include "baum.h"

#define DETECTSIZE 4096
#define NANGLE 64
#define NSEGMENT 512
#define SEGLEN 2
#define OS 2
#define SIZE_OF_DETECT_T (16*4)

#define QLENMAX 8

//

#define MAGIC 0xae6b1c8d47acf7f3ULL

typedef struct baum_t {
  uint64_t magic;

  int iwmax, ihmax;
  size_t bufSizeMax;

  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_program program;

  cl_kernel kernel_convert_lumaf_bgr;
  cl_kernel kernel_edge_f_f;
  cl_kernel kernel_edgeang_f_f;
  cl_kernel kernel_labelxPreprocess_int_int;
  cl_kernel kernel_label8xMain_int_int;

  cl_kernel kernel_clear;
  cl_kernel kernel_cast_i_f;
  cl_kernel kernel_threshold_i_i;

  cl_kernel kernel_aclassify;
  cl_kernel kernel_regionSize;
  cl_kernel kernel_threRegion2;
  cl_kernel kernel_maxRegion;
  cl_kernel kernel_spread;
  cl_kernel kernel_threshold2;
  cl_kernel kernel_detect;
  cl_kernel kernel_stage1;
  cl_kernel kernel_stage2;
  cl_kernel kernel_stage3even, kernel_stage3odd;
  cl_kernel kernel_maskluma2;
  cl_kernel kernel_oversample2;
  cl_kernel kernel_decode0;
  cl_kernel kernel_decode1;

  cl_mem imgbuf, detected, decoded;
  cl_mem luma, eangle, mask, hough, edge;
  cl_mem tmp0, tmp1, tmp2, tmp3, bigtmp0, bigtmp1;

  cl_event queuedEvent[QLENMAX];
  int queueLen;
} baum_t;

//

static size_t roundup(size_t size) { return ((size + 31) & ~31); }

baum_t *baum_init2(cl_device_id device0, cl_context context0, int iw, int ih) {
  baum_t *thiz = (baum_t *)calloc(1, sizeof(baum_t));
  thiz->magic = MAGIC;

  cl_int err;

  thiz->iwmax = roundup(iw);
  thiz->ihmax = roundup(ih);
  thiz->bufSizeMax = thiz->iwmax * thiz->ihmax;

  thiz->device = device0;
  thiz->context = context0;

  thiz->queue = clCreateCommandQueue(thiz->context, thiz->device, CL_QUEUE_PROFILING_ENABLE, &err); ce(err);

  thiz->program = clCreateProgramWithSource(thiz->context, 1, (const char **)&baum_cl, NULL, &err); ce(err);
  if (simpleBuildProgram(thiz->program, thiz->device, "-Werror") != 0) {
    ce(clReleaseProgram(thiz->program));
    ce(clReleaseCommandQueue(thiz->queue));
    ce(clReleaseContext(thiz->context));
    thiz->magic = 0;
    free(thiz);
    return NULL;
  }

  thiz->kernel_convert_lumaf_bgr = clCreateKernel(thiz->program, "convert_lumaf_bgr", &err); ce(err);
  thiz->kernel_edge_f_f = clCreateKernel(thiz->program, "edge_f_f", &err); ce(err);
  thiz->kernel_edgeang_f_f = clCreateKernel(thiz->program, "edgeang_f_f", &err); ce(err);
  thiz->kernel_labelxPreprocess_int_int = clCreateKernel(thiz->program, "labelxPreprocess_int_int", &err); ce(err);
  thiz->kernel_label8xMain_int_int = clCreateKernel(thiz->program, "label8xMain_int_int", &err); ce(err);
  thiz->kernel_clear = clCreateKernel(thiz->program, "clear", &err); ce(err);
  thiz->kernel_cast_i_f = clCreateKernel(thiz->program, "cast_i_f", &err); ce(err);
  thiz->kernel_threshold_i_i = clCreateKernel(thiz->program, "threshold_i_i", &err); ce(err);
  thiz->kernel_aclassify = clCreateKernel(thiz->program, "aclassify", &err); ce(err);
  thiz->kernel_regionSize = clCreateKernel(thiz->program, "regionSize", &err); ce(err);
  thiz->kernel_threRegion2 = clCreateKernel(thiz->program, "threRegion2", &err); ce(err);
  thiz->kernel_maxRegion = clCreateKernel(thiz->program, "maxRegion", &err); ce(err);
  thiz->kernel_spread = clCreateKernel(thiz->program, "spread", &err); ce(err);
  thiz->kernel_threshold2 = clCreateKernel(thiz->program, "threshold2", &err); ce(err);
  thiz->kernel_detect = clCreateKernel(thiz->program, "detect", &err); ce(err);
  thiz->kernel_stage1 = clCreateKernel(thiz->program, "stage1", &err); ce(err);
  thiz->kernel_stage2 = clCreateKernel(thiz->program, "stage2", &err); ce(err);
  thiz->kernel_stage3even = clCreateKernel(thiz->program, "stage3even", &err); ce(err);
  thiz->kernel_stage3odd = clCreateKernel(thiz->program, "stage3odd", &err); ce(err);
  thiz->kernel_maskluma2 = clCreateKernel(thiz->program, "maskluma2", &err); ce(err);
  thiz->kernel_oversample2 = clCreateKernel(thiz->program, "oversample2", &err); ce(err);
  thiz->kernel_decode0 = clCreateKernel(thiz->program, "decode0", &err); ce(err);
  thiz->kernel_decode1 = clCreateKernel(thiz->program, "decode1", &err); ce(err);

  thiz->imgbuf   = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->luma     = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->eangle   = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->mask     = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->hough    = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, NANGLE * NSEGMENT * sizeof(cl_int), NULL, &err); ce(err);
  thiz->edge     = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->detected = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, DETECTSIZE * SIZE_OF_DETECT_T, NULL, &err); ce(err);
  thiz->decoded  = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->tmp0     = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->tmp1     = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->tmp2     = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->tmp3     = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * sizeof(cl_int), NULL, &err); ce(err);
  thiz->bigtmp0  = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * 2 * sizeof(cl_int), NULL, &err); ce(err);
  thiz->bigtmp1  = clCreateBuffer(thiz->context, CL_MEM_READ_WRITE, thiz->bufSizeMax * 2 * sizeof(cl_int), NULL, &err); ce(err);

  for(int i=0;i<QLENMAX;i++) {
    thiz->queuedEvent[i] = NULL;
  }

  thiz->queueLen = 0;

  return thiz;
}

#define MAXDEVICES 10

baum_t *baum_init(int did, int iw, int ih) {
  cl_device_id devices[MAXDEVICES];
  int ndev = simpleGetDevices(devices, MAXDEVICES);

  if (did < 0) {
    printf("Available devices :\n");
    for(int i=0;i<ndev;i++) {
      char *name = getDeviceName(devices[i]);
      printf("%d : %s\n", i, name);
      free(name);
    }
    return NULL;
  }

  if (did >= ndev) return NULL;

  return baum_init2(devices[did], simpleCreateContext(devices[did]), iw, ih);
}

char *baum_getDeviceName(baum_t *thiz) { return getDeviceName(thiz->device); }

void baum_dispose(baum_t *thiz) {
  assert(thiz->magic == MAGIC);

  while(thiz->queueLen != 0) {
    baum_poll(thiz, 1);
  }

  ce(clReleaseMemObject(thiz->bigtmp1));
  ce(clReleaseMemObject(thiz->bigtmp0));
  ce(clReleaseMemObject(thiz->tmp3));
  ce(clReleaseMemObject(thiz->tmp2));
  ce(clReleaseMemObject(thiz->tmp1));
  ce(clReleaseMemObject(thiz->tmp0));
  ce(clReleaseMemObject(thiz->decoded));
  ce(clReleaseMemObject(thiz->detected));
  ce(clReleaseMemObject(thiz->edge));
  ce(clReleaseMemObject(thiz->hough));
  ce(clReleaseMemObject(thiz->mask));
  ce(clReleaseMemObject(thiz->eangle));
  ce(clReleaseMemObject(thiz->luma));
  ce(clReleaseMemObject(thiz->imgbuf));

  ce(clReleaseKernel(thiz->kernel_decode1));
  ce(clReleaseKernel(thiz->kernel_decode0));
  ce(clReleaseKernel(thiz->kernel_oversample2));
  ce(clReleaseKernel(thiz->kernel_maskluma2));
  ce(clReleaseKernel(thiz->kernel_stage3odd));
  ce(clReleaseKernel(thiz->kernel_stage3even));
  ce(clReleaseKernel(thiz->kernel_stage2));
  ce(clReleaseKernel(thiz->kernel_stage1));
  ce(clReleaseKernel(thiz->kernel_detect));
  ce(clReleaseKernel(thiz->kernel_threshold2));
  ce(clReleaseKernel(thiz->kernel_spread));
  ce(clReleaseKernel(thiz->kernel_maxRegion));
  ce(clReleaseKernel(thiz->kernel_threRegion2));
  ce(clReleaseKernel(thiz->kernel_aclassify));
  ce(clReleaseKernel(thiz->kernel_threshold_i_i));
  ce(clReleaseKernel(thiz->kernel_cast_i_f));
  ce(clReleaseKernel(thiz->kernel_clear));
  ce(clReleaseKernel(thiz->kernel_label8xMain_int_int));
  ce(clReleaseKernel(thiz->kernel_labelxPreprocess_int_int));
  ce(clReleaseKernel(thiz->kernel_edgeang_f_f));
  ce(clReleaseKernel(thiz->kernel_edge_f_f));
  ce(clReleaseKernel(thiz->kernel_convert_lumaf_bgr));

  ce(clReleaseProgram(thiz->program));
  ce(clReleaseCommandQueue(thiz->queue));
  ce(clReleaseContext(thiz->context));

  thiz->magic = 0;

  free(thiz);
}

void *baum_malloc(baum_t *thiz, size_t z) {
  assert(thiz->magic == MAGIC);
  return allocatePinnedMemory(z, thiz->context, thiz->queue);
}

void baum_free(baum_t *thiz, void *ptr) {
  assert(thiz->magic == MAGIC);
  freePinnedMemory(ptr, thiz->context, thiz->queue);
}

static void clear(baum_t *thiz, cl_mem mem, int size) {
  simpleSetKernelArg(thiz->kernel_clear, "Mi", mem, size);
  runKernel1D(thiz->queue, thiz->kernel_clear, __COUNTER__, roundup(size), -1);
}

static void label8x(baum_t *thiz, cl_mem out, cl_mem in, cl_mem tmp, int bgc, int iw, int ih) {
  const size_t ww = roundup(iw), wh = roundup(ih);

#define MAXPASS 10

  simpleSetKernelArg(thiz->kernel_labelxPreprocess_int_int, "MMMiiii", out, in, tmp, MAXPASS, bgc, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_labelxPreprocess_int_int, __COUNTER__, ww, wh, -1);

  for(int i=1;i<=MAXPASS;i++) {
    simpleSetKernelArg(thiz->kernel_label8xMain_int_int, "MMMiii", out, in, tmp, i, iw, ih);
    runKernel2D(thiz->queue, thiz->kernel_label8xMain_int_int, __COUNTER__, ww, wh, -1);
  }
}

cl_event genTask(baum_t *thiz, void *resultPtr, size_t bufSize, const void *imgPtr, const int iw, const int ih, const int ws) {
  assert(thiz->magic == MAGIC);

  if (iw > thiz->iwmax || ih > thiz->ihmax) exitf(-1, "Image size larger than initialized size\n");

  const size_t ww = roundup(iw), wh = roundup(ih), wwh = roundup(iw * ih);

  cl_event evWrite;

  //ce(clEnqueueWriteBuffer(thiz->queue, thiz->imgbuf, CL_FALSE, 0, iw * ih * 3, imgPtr, 0, NULL, &evWrite));
  {
    size_t origin[3] = {0, 0, 0}, region[3] = {(size_t)(iw * 3), (size_t)ih, 1};
    ce(clEnqueueWriteBufferRect(thiz->queue, thiz->imgbuf, CL_FALSE, origin, origin, region, iw*3, 0, ws, 0, imgPtr, 0, NULL, &evWrite));
  }
  clFlush(thiz->queue);

  simpleSetKernelArg(thiz->kernel_convert_lumaf_bgr, "MMii", thiz->luma, thiz->imgbuf, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_convert_lumaf_bgr, __COUNTER__, ww, wh, -1);

  simpleSetKernelArg(thiz->kernel_edge_f_f, "MMii", thiz->edge, thiz->luma, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_edge_f_f, __COUNTER__, ww, wh, -1);

  simpleSetKernelArg(thiz->kernel_cast_i_f, "MMfi", thiz->tmp1, thiz->edge, 1024 * 1024.0f, iw * ih);
  runKernel1D(thiz->queue, thiz->kernel_cast_i_f, __COUNTER__, wwh, -1);

  simpleSetKernelArg(thiz->kernel_edgeang_f_f, "MMii", thiz->eangle, thiz->luma, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_edgeang_f_f, __COUNTER__, ww, wh, -1);

  simpleSetKernelArg(thiz->kernel_aclassify, "MMMfii", thiz->tmp0, thiz->eangle, thiz->edge, 0.01f, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_aclassify, __COUNTER__, ww, wh, -1);

  label8x(thiz, thiz->bigtmp1, thiz->tmp0, thiz->tmp2, -1, iw, ih);

  clear(thiz, thiz->tmp3, iw*ih);
  simpleSetKernelArg(thiz->kernel_regionSize, "MMii", thiz->tmp3, thiz->bigtmp1, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_regionSize, __COUNTER__, ww/2, wh/2, -1);

  clear(thiz, thiz->bigtmp0, iw*ih);
  simpleSetKernelArg(thiz->kernel_maxRegion, "MMMii", thiz->bigtmp0, thiz->bigtmp1, thiz->tmp1, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_maxRegion, __COUNTER__, ww, wh, -1);

  simpleSetKernelArg(thiz->kernel_threRegion2, "MMMiii", thiz->bigtmp0, thiz->bigtmp1, thiz->tmp3, 32, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_threRegion2, __COUNTER__, ww, wh, -1);

  simpleSetKernelArg(thiz->kernel_threshold_i_i, "MMiiii", thiz->bigtmp0, thiz->bigtmp0, 0, (int)(1024 * 1024 * 0.08f), (int)(1024 * 1024 * 1.0f), iw*ih);
  runKernel1D(thiz->queue, thiz->kernel_threshold_i_i, __COUNTER__, wwh, -1);

  simpleSetKernelArg(thiz->kernel_spread, "MMii", thiz->bigtmp0, thiz->bigtmp1, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_spread, __COUNTER__, ww, wh, -1);

  simpleSetKernelArg(thiz->kernel_threshold2, "MMiii", thiz->mask, thiz->bigtmp0, (int)(1024 * 1024 * 0.5f), iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_threshold2, __COUNTER__, ww, wh, -1);

  clear(thiz, thiz->hough, NANGLE*NSEGMENT);
  simpleSetKernelArg(thiz->kernel_detect, "MMMMii", thiz->hough, thiz->edge, thiz->eangle, thiz->mask, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_detect, __COUNTER__, ww, wh, -1);

  clear(thiz, thiz->tmp0, thiz->bufSizeMax);
  simpleSetKernelArg(thiz->kernel_stage1, "MMi", thiz->tmp0, thiz->hough, thiz->bufSizeMax);
  runKernel2D(thiz->queue, thiz->kernel_stage1, __COUNTER__, NANGLE, NSEGMENT, -1);

  clear(thiz, thiz->detected, DETECTSIZE * SIZE_OF_DETECT_T / 4);
  simpleSetKernelArg(thiz->kernel_stage2, "MMMMMfiiii", thiz->detected, thiz->tmp0, thiz->hough, thiz->mask, thiz->eangle, hypotf(iw, ih), DETECTSIZE * SIZE_OF_DETECT_T / 4, thiz->bufSizeMax, iw, ih);
  runKernel1D(thiz->queue, thiz->kernel_stage2, __COUNTER__, DETECTSIZE, -1);

  for(int i=0;i<6;i++) {
    simpleSetKernelArg(thiz->kernel_stage3odd, "M", thiz->detected);
    runKernel1D(thiz->queue, thiz->kernel_stage3odd , __COUNTER__, DETECTSIZE, -1);

    simpleSetKernelArg(thiz->kernel_stage3even, "Mi", thiz->detected, i == 5 ? 1 : 0);
    runKernel1D(thiz->queue, thiz->kernel_stage3even, __COUNTER__, DETECTSIZE, -1);
  }

  simpleSetKernelArg(thiz->kernel_maskluma2, "MMMii", thiz->tmp0, thiz->luma, thiz->mask, iw, ih);
  runKernel2D(thiz->queue, thiz->kernel_maskluma2, __COUNTER__, ww, wh, -1);

  simpleSetKernelArg(thiz->kernel_oversample2, "MMMiii", thiz->bigtmp0, thiz->detected, thiz->tmp0, iw, ih, thiz->bufSizeMax*2);
  runKernel1D(thiz->queue, thiz->kernel_oversample2, __COUNTER__, DETECTSIZE, -1);

  static const float fc[] = { 1-0.15f, 1-0.0375f };

  for(int i=0;i<2;i++) {
    simpleSetKernelArg(thiz->kernel_decode0, "MMMMMfiii", thiz->decoded, thiz->tmp0, thiz->bigtmp1, thiz->bigtmp0, thiz->detected, fc[i], i == 0 ? 1 : 0, thiz->bufSizeMax, thiz->bufSizeMax*2);
    runKernel1D(thiz->queue, thiz->kernel_decode0, __COUNTER__, DETECTSIZE, -1);

    simpleSetKernelArg(thiz->kernel_decode1, "MMMMi", thiz->decoded, thiz->tmp0, thiz->bigtmp1, thiz->detected, thiz->bufSizeMax);
    runKernel1D(thiz->queue, thiz->kernel_decode1, __COUNTER__, DETECTSIZE, -1);
  }

  cl_event ev;

  if (bufSize > thiz->bufSizeMax*4) bufSize = thiz->bufSizeMax*4;

  ce(clEnqueueReadBuffer(thiz->queue, thiz->decoded, CL_FALSE, 0, bufSize, resultPtr, 0, NULL, &ev));
  clFlush(thiz->queue);

  waitForEvent(evWrite);
  ce(clReleaseEvent(evWrite));

  return ev;
}

int baum_enqueueTask(baum_t *thiz, void *resultPtr, size_t bufSize, const void *imgPtr, const int iw, const int ih, const int ws) {
  assert(thiz->magic == MAGIC);

  cl_event ev = genTask(thiz, resultPtr, bufSize, imgPtr, iw, ih, ws);

  if (thiz->queueLen >= QLENMAX) exitf(-1, "Task queue overflow\n");

  ce(clRetainEvent(ev));
  thiz->queuedEvent[thiz->queueLen++] = ev;

  return 0;
}

cl_event *baum_peekEvent(baum_t *thiz) {
  assert(thiz->magic == MAGIC);

  if (thiz->queueLen == 0) return NULL;

  return &(thiz->queuedEvent[0]);
}

int baum_poll(baum_t *thiz, int waitFlag) {
  assert(thiz->magic == MAGIC);

  if (thiz->queueLen == 0) return 0;

  if (!waitFlag) {
    cl_int ret;
    ce(clGetEventInfo(thiz->queuedEvent[0], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &ret, NULL));
    if (ret != CL_COMPLETE) return 0;
  }

  waitForEvent(thiz->queuedEvent[0]);

  ce(clReleaseEvent(thiz->queuedEvent[0]));

  for(int i=1;i<thiz->queueLen;i++) {
    thiz->queuedEvent[i-1] = thiz->queuedEvent[i];
  }

  thiz->queueLen--;

  return 1;
}

int baum_queueLen(baum_t *thiz) {
  assert(thiz->magic == MAGIC);
  return thiz->queueLen;
}

void baum_createPlan(baum_t *thiz, const char *path, void *resultPtr, size_t bufSize, const void *imgPtr, const int iw, const int ih, const int ws) {
  assert(thiz->magic == MAGIC);

  for(int xs = 2;xs <= 256;xs *= 2) {
    for(int ys = 2;ys <= 64;ys *= 2) {
      //printf("%d, %d\n", xs, ys); fflush(stdout);
      
      startProfiling(xs, ys, 1);

      baum_enqueueTask(thiz, resultPtr, bufSize, imgPtr, iw, ih, ws);
      baum_poll(thiz, 1);

      finishProfiling();
    }
  }

  //showPlan();

  savePlan(path, thiz->device);
}

int baum_loadPlan(baum_t *thiz, const char *path) {
  assert(thiz->magic == MAGIC);

  return loadPlan(path, thiz->device);
}

static int code[] = {
  0x08aba, 0x08ace, 0x08c68, 0x08db2, 0x08dcc, 0x08e44, 0x09116, 0x09164,
  0x091da, 0x09262, 0x092d6, 0x09548, 0x095aa, 0x095b4, 0x0969c, 0x096a6,
  0x098c8, 0x099b8, 0x09ab4, 0x09b26, 0x09b8a, 0x09c8e, 0x09d1a, 0x0a298,
  0x0a4ae, 0x0a528, 0x0a5d2, 0x0a624, 0x0a672, 0x0a746, 0x0a944, 0x0a9ca,
  0x0aa6a, 0x0ab2c, 0x0ab76, 0x0ac74, 0x0acc6, 0x0ad6e, 0x0ae8c, 0x0aeda,
  0x0b14e, 0x0b368, 0x0b496, 0x0b4b8, 0x0b4e2, 0x0b718, 0x0b722, 0x0b8d4,
  0x0b934, 0x0bb3a, 0x0c494, 0x0c4ba, 0x0c4ec, 0x0c556, 0x0c622, 0x0c674,
  0x0c688, 0x0c71c, 0x0c988, 0x0ca9c, 0x0cb64, 0x0cd4a, 0x0d18e, 0x0d1d4,
  0x0d25c, 0x0d2aa, 0x0d2e4, 0x0d3b6, 0x0d4c6, 0x0d562, 0x0db6a, 0x0dcb6,
  0x0dcea, 0x0ddd8, 0x0e2ac, 0x0e392, 0x0e44e, 0x0e558, 0x0e5c4, 0x0e638,
  0x0e6dc, 0x0e9a4, 0x0eac4, 0x0eb22, 0x0eb8e, 0x0ed14, 0x0edb8, 0x0ee6c,
  0x0eeaa, 0x1112c, 0x111ba, 0x112a2, 0x11494, 0x1156a, 0x11624, 0x1198e,
  0x119b4, 0x11aca, 0x11ae4, 0x11c4e, 0x11c74, 0x11ca6, 0x11dc4, 0x12232,
  0x122da, 0x122ec, 0x12476, 0x1248a, 0x12726, 0x12758, 0x128a4, 0x128ce,
  0x12956, 0x12988, 0x12ba2, 0x12c5a, 0x12d2a, 0x12d34, 0x12d92, 0x13122,
  0x133a4, 0x13564, 0x1389c, 0x138aa, 0x13b74, 0x14454, 0x14636, 0x146aa,
  0x146d8, 0x1472c, 0x14746, 0x148ae, 0x148d6, 0x14a5a, 0x14a6c, 0x14da2,
  0x152ac, 0x15472, 0x1558a, 0x15638, 0x15722, 0x15748, 0x1592a, 0x15ab6,
  0x15b4e, 0x15c92, 0x15d56, 0x15dac, 0x162c6, 0x16352, 0x163b6, 0x16562,
  0x1664c, 0x16932, 0x16994, 0x16b24, 0x16b5c, 0x16bb8, 0x16d8e, 0x16e28,
  0x16e56, 0x1714a, 0x17234, 0x172ba, 0x17446, 0x1748c, 0x17512, 0x175d8,
  0x176ca, 0x17754, 0x1888c, 0x18b92, 0x18caa, 0x18d46, 0x18dda, 0x18e52,
  0x18e94, 0x18eec, 0x19244, 0x19766, 0x1994c, 0x19a68, 0x19d14, 0x1a2a6,
  0x1a46a, 0x1a5c8, 0x1a8d8, 0x1aa8a, 0x1acb6, 0x1ad72, 0x1adac, 0x1ae22,
  0x1ae48, 0x1b154, 0x1b252, 0x1b4a4, 0x1b58e, 0x1b636, 0x1b72c, 0x1b8ec,
  0x1b96a, 0x1c564, 0x1c66e, 0x1c6c4, 0x1c92c, 0x1ca46, 0x1caea, 0x1cbb4,
  0x1cc76, 0x1cd9c, 0x1ce24, 0x1ce8e, 0x1d1c8, 0x1d2ce, 0x1d36c, 0x1d39c,
  0x1d498, 0x1d55c, 0x1d5d2, 0x1d6e8, 0x1d8dc, 0x1da74, 0x1dbae, 0x1dc44,
  0x1dd26, 0x1dd68, 0x1ddba, 0x22294, 0x2234e, 0x22522, 0x22636, 0x226ee,
  0x228a8, 0x2295a, 0x22974, 0x22b2a, 0x22b92, 0x22c9a, 0x22ccc, 0x22d2c,
  0x22d46, 0x231b8, 0x231cc, 0x232e4, 0x23456, 0x2349c, 0x23592, 0x2362c,
  0x23662, 0x238b4, 0x2391c, 0x239a2, 0x23a32, 0x23a6e, 0x23b24, 0x23b56,
  0x23b88, 0x244a8, 0x2454e, 0x2459c, 0x24672, 0x246e4, 0x24892, 0x249b8,
  0x24a36, 0x24ad8, 0x24b1c, 0x24b8a, 0x24c6c, 0x251d8, 0x252a6, 0x25444,
  0x25496, 0x25526, 0x255ba, 0x25658, 0x25756, 0x258cc, 0x25934, 0x25a52,
  0x25b22, 0x25c76, 0x25dd2, 0x2625a, 0x2645c, 0x265da, 0x2672e, 0x2689c,
  0x26916, 0x2694c, 0x26a68, 0x26bb4, 0x26db2, 0x26e66, 0x26e96, 0x2718a,
  0x27298, 0x274c8, 0x27572, 0x276b2, 0x276d4, 0x2771c, 0x27764, 0x288dc,
  0x2894e, 0x289d2, 0x28b62, 0x28bb6, 0x28d38, 0x28e34, 0x2915c, 0x29346,
  0x293a2, 0x29652, 0x29736, 0x2976c, 0x298b2, 0x2992a, 0x29994, 0x29a38,
  0x29b12, 0x29b48, 0x29c46, 0x29cda, 0x29da6, 0x2a23a, 0x2a352, 0x2a364,
  0x2a51c, 0x2a64c, 0x2aa26, 0x2adaa, 0x2ae28, 0x2b132, 0x2b1d6, 0x2b254,
  0x2b56a, 0x2b6b4, 0x2b8ea, 0x2b966, 0x2c4d2, 0x2c5ae, 0x2c62c, 0x2c6b6,
  0x2c712, 0x2c744, 0x2c88e, 0x2c8b4, 0x2c8e8, 0x2cb3a, 0x2cd9a, 0x2ce6a,
  0x2ced4, 0x2d168, 0x2d192, 0x2d2c8, 0x2d318, 0x2d32e, 0x2d4dc, 0x2d514,
  0x2d55a, 0x2d666, 0x2d6b8, 0x2d898, 0x2d8d6, 0x2d944, 0x2dae2, 0x2dce4,
  0x2dd6e, 0x2e348, 0x2e39c, 0x2e3a6, 0x2e476, 0x2e48c, 0x2e53a, 0x2e56c,
  0x2e734, 0x2e762, 0x2e8ba, 0x2e92e, 0x2ead2, 0x2eca6, 0x2ee32, 0x2eece,
  0x3123a, 0x312e8, 0x3145c, 0x31546, 0x31932, 0x31b44, 0x31b6a, 0x31c52,
  0x31cec, 0x31d9a, 0x3225c, 0x32266, 0x32496, 0x324ac, 0x32538, 0x32962,
  0x32a98, 0x32c54, 0x32c68, 0x32da6, 0x32ec6, 0x33116, 0x332d6, 0x334c4,
  0x33892, 0x338c8, 0x338e6, 0x3394e, 0x33b38, 0x33b94, 0x34516, 0x34694,
  0x3475c, 0x34888, 0x34aa4, 0x34b74, 0x34cdc, 0x34d3a, 0x34e44, 0x34ed2,
  0x34ee8, 0x3514c, 0x35254, 0x353aa, 0x3569a, 0x356cc, 0x3576e, 0x358a2,
  0x35b2c, 0x35cb8, 0x35cc6, 0x35d64, 0x35dc8, 0x36238, 0x3628c, 0x36498,
  0x36544, 0x365aa, 0x366a6, 0x36776, 0x368b6, 0x36c72, 0x36cee, 0x36d1c,
  0x36dd6, 0x37272, 0x372ee, 0x37346, 0x3745a, 0x37474, 0x37568, 0x388e2,
  0x38916, 0x38a2a, 0x38a64, 0x38ad6, 0x38b5a, 0x38d22, 0x38d8e, 0x38e4e,
  0x38ea6, 0x3916e, 0x3922c, 0x392b6, 0x392da, 0x39372, 0x39524, 0x39758,
  0x398ce, 0x39c96, 0x39d4a, 0x39d76, 0x39ddc, 0x3a234, 0x3a268, 0x3a2c4,
  0x3a32e, 0x3a388, 0x3a396, 0x3a574, 0x3a5c6, 0x3a8ae, 0x3a95c, 0x3aab2,
  0x3ab46, 0x3aba4, 0x3ac66, 0x3ac88, 0x3acd2, 0x3aeea, 0x3b148, 0x3b472,
  0x3b594, 0x3b638, 0x3b664, 0x3b68c, 0x3b692, 0x3b8b8, 0x3b952, 0x3b98a,
  0x3ba4a, 0x3bb6c,
};

static void drawArc(FILE *fp, double x, double y, int upper, double r0, double r1) {
  static const double C = (4.0/3.0) * (sqrt(2.0)-1.0);

  if (upper) { r0 = -r0; r1 = -r1; }

  fprintf(fp, "<path d=\"");

  fprintf(fp, "M ");
  fprintf(fp, "%.2f %.2f ", x +   r0, y       );
  fprintf(fp, "C ");
  fprintf(fp, "%.2f %.2f ", x +   r0, y + C*r0);
  fprintf(fp, "%.2f %.2f ", x + C*r0, y +   r0);
  fprintf(fp, "%.2f %.2f ", x       , y +   r0);
  fprintf(fp, "C ");
  fprintf(fp, "%.2f %.2f ", x - C*r0, y +   r0);
  fprintf(fp, "%.2f %.2f ", x -   r0, y + C*r0);
  fprintf(fp, "%.2f %.2f ", x -   r0, y       );

  fprintf(fp, "L ");
  fprintf(fp, "%.2f %.2f ", x -   r1, y       );

  fprintf(fp, "C ");
  fprintf(fp, "%.2f %.2f ", x -   r1, y + C*r1);
  fprintf(fp, "%.2f %.2f ", x - C*r1, y +   r1);
  fprintf(fp, "%.2f %.2f ", x       , y +   r1);
  fprintf(fp, "C ");
  fprintf(fp, "%.2f %.2f ", x + C*r1, y +   r1);
  fprintf(fp, "%.2f %.2f ", x +   r1, y + C*r1);
  fprintf(fp, "%.2f %.2f ", x +   r1, y       );

  fprintf(fp, "Z\" stroke=\"none\" fill=\"black\" />\n");
}

void baum_fprintMarkerSVG(FILE *fp, int data, double radius, double posx, double posy) {
  static const int PL = 19, HPL = PL/2 + 1;

  int cw;
  cw = data >= 0 ? code[data] : 0x2aaaaa;

  double BW = radius / sqrt(HPL+3.0);

  fprintf(fp, "<!-- BAUM marker pattern No. %d -->\n", data);
  fprintf(fp, "<!-- http://ito-lab.naist.jp/~n-sibata/ -->\n");
  fprintf(fp, "<svg version=\"1.1\" baseProfile=\"full\" xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\">\n", (int)(radius*2.1+posx), (int)(radius*2.1+posy));

  drawArc(fp, radius + posx, radius + posy, false, sqrt(HPL+0.0) * BW, sqrt(HPL+1.0) * BW);
  drawArc(fp, radius + posx, radius + posy, true , sqrt(HPL+0.0) * BW, sqrt(HPL+1.0) * BW);
  drawArc(fp, radius + posx, radius + posy, false, sqrt(HPL+2.0) * BW, sqrt(HPL+3.0) * BW);
  drawArc(fp, radius + posx, radius + posy, true , sqrt(HPL+2.0) * BW, sqrt(HPL+3.0) * BW);

  for(int i=0;i<HPL;i++) {
    if (((cw >> (PL-i-1)) & 1) != 0) {
      drawArc(fp, radius + posx, radius + posy, false, sqrt(HPL - i - 0.0) * BW, sqrt(HPL - i - 1.0) * BW);
    }
  }

  for(int i=0;i<HPL;i++) {
    if (((cw >> i) & 1) != 0) {
      drawArc(fp, radius + posx, radius + posy, true , sqrt(HPL - i - 0.0) * BW, sqrt(HPL - i - 1.0) * BW);
    }
  }

  fprintf(fp, "</svg>\n");
}
