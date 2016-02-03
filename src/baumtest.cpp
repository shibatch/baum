// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This software is in public domain. You can use and modify this code
// for any purpose without any obligation.


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <cv.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#include "baum.h"

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage : %s <device number> <file name>\n", argv[0]);
    fflush(stderr);
    baum_init(-1, 0, 0);
    exit(-1);
  }

  //

  IplImage *img = 0;
  img = cvLoadImage(argv[2], CV_LOAD_IMAGE_COLOR);
  if( !img ) {
    fprintf(stderr, "Could not load %s\n", argv[2]);
    exit(-1);
  }

  if (img->nChannels != 3) {
    fprintf(stderr, "nChannels != 3\n");
    exit(-1);
  }

  int iw = img->width, ih = img->height, ws = img->widthStep;

  //

#define RESULTSIZE (1920 * 1080 * 4)

  //

  int did = 0;
  if (argc >= 3) did = atoi(argv[1]);

  baum_t *baum = baum_init(did, iw, ih);
  if (baum == NULL) {
    fprintf(stderr, "Could not build OpenCL kernels.\n");
    exit(-1);
  }

  printf("OpenCL device : %s\n", baum_getDeviceName(baum));
  
  // baum_malloc allocates pinned memory, that can be quickly accessed from the OpenCL device
  void *mpimg = baum_malloc(baum, ws * ih);
  int *result = (int *)baum_malloc(baum, RESULTSIZE);

  // By creating a plan, OpenCL kernels can run faster
  if (baum_loadPlan(baum, "baum_plan.txt") != 0) {
    printf("Could not load a plan. Please run createPlan.bat.\n");
  }

  // Copy image data to pinned memory
  memcpy(mpimg, img->imageData, ws * ih);

  // Enqueue the task to decode markers in the frame
  baum_enqueueTask(baum, result, RESULTSIZE, mpimg, iw, ih, ws);

  // You can specify non-pinned memory as arguments to baum_enqueueTask like below.
  // baum_enqueueTask(baum, result, RESULTSIZE, img->imageData, iw, ih, ws);
  // However, this takes a lot more CPU time than doing memcpy to pinned memory.

  // The enqueued task is executed asynchronously.
  // You can do other tasks here, before execution of the task finishes, like below.
  // while(baum_poll(baum, 0) == 0) doSomething();

  // Now, receive the result of the enqueued task
  baum_poll(baum, 1);
  printf("decode = %d, detect = %d\n", result[0], result[1]);

  //
  
#define SIZE_OF_DECODE_T (4 * 8)

  uint8_t *rdata = (uint8_t *)result;

  CvFont font;
  cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);

  // There are (result[0]-1) decoded markers
  for(int i=1;i<result[0];i++) {  // This starts from ONE
    float *fldata = (float *)(rdata + i * SIZE_OF_DECODE_T);
    cvLine(img, cvPoint(fldata[0], fldata[1]), cvPoint(fldata[2], fldata[3]), Scalar(255, 0, 0), 1, 8, 0);

    int *idata = (int *)(rdata + i * SIZE_OF_DECODE_T);
    int d = idata[4];

    char str[32];
    sprintf(str, "%d", d);
    cvPutText(img, str, cvPoint((fldata[0]+fldata[2])*0.5, (fldata[1]+fldata[3])*0.5), &font, Scalar(0, 255, 0));
  }

  cvSaveImage("output.png", img, NULL);

  //

  baum_free(baum, result);
  baum_free(baum, mpimg);

  baum_dispose(baum);

  exit(0);
}
