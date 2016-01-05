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

#include "helper.h"
#include "baum.h"

int main(int argc, char **argv) {
  if (argc < 2) exit(-1);

  //

  IplImage *img = 0;
  img = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
  if( !img ) exitf(-1, "Could not load %s\n", argv[1]);

  if (img->nChannels != 3) exitf(-1, "nChannels != 3\n");

  int iw = img->width, ih = img->height, ws = img->widthStep;
  uint8_t *data = (uint8_t *)img->imageData;

  //

#define RESULTSIZE (1920 * 1080 * 4)

  //

  int did = 0;
  if (argc >= 3) did = atoi(argv[2]);

  printf("Creating plan for device %d\n", did);
  fflush(stdout);

  baum_t *baum = baum_init(did, iw, ih);

  if (baum == NULL) exit(-1);

  void *mpimg = baum_malloc(baum, ws * ih);
  int *result = (int *)baum_malloc(baum, RESULTSIZE);

  memcpy(mpimg, data, ws * ih);

  printf("Device name : %s\n", baum_getDeviceName(baum));
  fflush(stdout);

  baum_createPlan(baum, "baum_plan.txt", result, RESULTSIZE, mpimg, iw, ih, ws);

  baum_free(baum, result);
  baum_free(baum, mpimg);

  baum_dispose(baum);

  exit(0);
}
