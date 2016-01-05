// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This software is in public domain. You can use and modify this code
// for any purpose without any obligation.


#ifdef _MSC_VER
#include <Windows.h>
#include <WinBase.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <memory.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#pragma push_macro("NDEBUG")
#define NDEBUG
#include <GL/freeglut.h>
#pragma pop_macro("NDEBUG")

#include <cv.h>
#include <highgui.h>

#include "helper.h"
#include "poly.h"
#include "baum.h"

#define ESCAPE 27
#define MAPSIZE 2048

int window;
GLuint texName;

double tw, th;
int iw, ih, ws, ws3;

CvCapture *capture = NULL;
IplImage *iimg = NULL;

#define RESULTSIZE (1920 * 1080 * 4)
baum_t *baum;
int *result[2], resultPage;
void *mpimg;

int frameCount = 0, totalFrameCount = 0, lastFrameNum = 0;
long long int tm;

void init() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 0, 1, -1, 1);

  glGenTextures(1, &texName);

  glBindTexture(GL_TEXTURE_2D, texName);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, MAPSIZE, MAPSIZE, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void reshape(int width, int height) {
  glViewport(0, 0, width, height);

  double height0 = (double)height / ih * iw;
  double size = width < height0 ? width : height0;
  double hpad = (double)(width   - size) / size / 2;
  double vpad = (double)(height0 - size) / size / 2;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-hpad, 1 + hpad, -vpad, 1 + vpad, -1, 1);
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, texName);

  // Draw the frame captured at the last display
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ws3, ih, GL_BGR, GL_UNSIGNED_BYTE, iimg->imageData);

  glEnable(GL_TEXTURE_2D);

  glBegin( GL_QUADS );
  glTexCoord2d(0.0,th ); glVertex3d(0.0,0.0, -1.0);
  glTexCoord2d(tw ,th ); glVertex3d(1.0,0.0, -1.0);
  glTexCoord2d(tw ,0.0); glVertex3d(1.0,1.0, -1.0);
  glTexCoord2d(0.0,0.0); glVertex3d(0.0,1.0, -1.0);
  glEnd();

  glDisable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);

  if (capture != NULL) {
    // Capture a new frame
    iimg = cvRetrieveFrame(capture);

    // Rewind the video if it reaches the last frame
    int curFrameNum = cvGetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES);
    if (lastFrameNum == curFrameNum) {
      cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, 0);
      iimg = cvQueryFrame(capture);
      curFrameNum = cvGetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES);
    }
    lastFrameNum = curFrameNum;

    if (iimg == NULL) {
      cvReleaseCapture(&capture);
      capture = NULL;
      exit(0);
    }
    cvGrabFrame(capture);
  }

  memcpy(mpimg, iimg->imageData, ws * ih); // Copy image data to pinned memory

  // Enqueue the current frame
  baum_enqueueTask(baum, result[(resultPage++) & 1], RESULTSIZE, mpimg, iw, ih, ws); 

  // Poll the result for the previous frame
  baum_poll(baum, 1); 

#if 0
  printf("decode = %d, detect = %d\n", result[resultPage & 1][0], result[resultPage & 1][1]);
  fflush(stdout);
#endif

#define SIZE_OF_DECODE_T (4 * 8)

  uint8_t *rdata = (uint8_t *)result[resultPage & 1];

  //

#if 1
  // Display a line segment for each detected barcode

  glColor3f(1.0f, 0.0f, 0.0f);

  for(int i=1;i<result[resultPage & 1][0];i++) {
    float *fldata = (float *)(rdata + i * SIZE_OF_DECODE_T);
    glBegin( GL_LINES );
    glVertex2f(fldata[0] * (float)(1.0 / iw), 1.0f - fldata[1] * (float)(1.0 / ih));
    glVertex2f(fldata[2] * (float)(1.0 / iw), 1.0f - fldata[3] * (float)(1.0 / ih));
    glEnd();
  }
#endif

  //

#if 1
  // Display a polygon (convex hull) on the detected barcodes

  ArrayMap *am = initArrayMap();

  for(int i=1;i<result[resultPage & 1][0];i++) {
    int *idata = (int *)(rdata + i * SIZE_OF_DECODE_T);
    float *fldata = (float *)(rdata + i * SIZE_OF_DECODE_T);

    int d = idata[4];

    Poly12 *p = (Poly12 *)ArrayMap_get(am, d);
    if (p == NULL) {
      p = initPoly12(d);
      ArrayMap_put(am, d, p);
    }

    Poly12_add(p, fldata[0] * (float)(1.0 / iw), 1.0f - fldata[1] * (float)(1.0 / ih));
    Poly12_add(p, fldata[2] * (float)(1.0 / iw), 1.0f - fldata[3] * (float)(1.0 / ih));
  }

  int size = ArrayMap_size(am);
  uint64_t *kaam = ArrayMap_keyArray(am);
  Poly12 **vaam = (Poly12 **)ArrayMap_valueArray(am);

  for(int i=0;i<size;i++) {
    if (Poly12_size(vaam[i]) >= 4) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(1.0f, 0.5f, 0.0f, 0.5f);
      Poly12_drawGL(vaam[i], GL_POLYGON, 1.0f, 0.0f, 0.0f);
      glDisable(GL_BLEND);

      glColor3f(0.0f, 0.0f, 0.5f);
      glLineWidth(2.0f);
      Poly12_drawGL(vaam[i], GL_LINE_LOOP, 1.0f, 0.0f, 0.0f);

      glColor3f(0.0f, 1.0f, 1.0f);
      float gc[2];
      Poly12_center(gc, vaam[i]);
      glRasterPos2f(gc[0], gc[1]);
      unsigned char str[20];
      sprintf((char *)str, "%d", (int)kaam[i]);
      glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, str);
    }

    Poly12_dispose(vaam[i]);
  }

  free(vaam);
  free(kaam);
  ArrayMap_dispose(am);
#endif

  glutSwapBuffers();

  //

  frameCount++;
  totalFrameCount++;

  long long int t = currentTimeMillis();
  if (t - tm > 1000) {
    printf("%.1f fps\n", frameCount * 1000.0 / (t - tm));
    fflush(stdout);
    tm += 1000;
    frameCount = 0;
  }

  //

  { // This is a workaround to reduce CPU usage by GLUI
    static int64_t lastDisplay = 0;
    int t2s = 17 - (int)(currentTimeMillis() - lastDisplay);
    if (lastDisplay > 0 && t2s > 0) sleepMillis(t2s);
    lastDisplay = currentTimeMillis();
  }
}

void windowClosed() { exit(0); }

void whenexits() {
  if (capture != NULL) cvReleaseCapture(&capture);
  capture = NULL;
}

int main(int argc, char **argv) {
#ifdef _MSC_VER
  SetProcessDPIAware();
#endif

  glutInit(&argc, argv);
  atexit(whenexits);

  if (argc <= 1) {
    fprintf(stderr, "Usage : %s <device number> [<file name>]\n", argv[0]);
    fprintf(stderr, "This program shows detection results of the BAUM markers.\n");
    fprintf(stderr, "You can specify a video file or a still image file as the last argument.\n");
    fprintf(stderr, "If no file is specified, the first camera is selected as the video source.\n");
    fprintf(stderr, "You need ffmpeg DLL to decode videos.\n");
    fflush(stderr);
    baum_init(-1, 0, 0);
    exit(-1);
  }

  if (argc >= 3) {
    iimg = cvLoadImage(argv[2], CV_LOAD_IMAGE_COLOR);

    if (iimg == NULL) {
      capture = cvCreateFileCapture(argv[2]);
      if( !capture ) exitf(-1, "Could not open video file %s\n", argv[2]);
      iimg = cvQueryFrame(capture);
    }
  } else {
    capture = cvCreateCameraCapture(0);
    if ( !capture) exitf(-1, "Camera not available\n");
    atexit(whenexits);
    
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH , 1920);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 1080);
  
    iimg = cvQueryFrame(capture);
  }
  
  if (iimg->nChannels != 3) exitf(-1, "nChannels != 3\n");

  iw = iimg->width;
  ih = iimg->height;
  ws = iimg->widthStep;
  ws3 = (ws + 2) / 3;

  //

  int did = 0;
  if (argc >= 2) did = atoi(argv[1]);

  baum = baum_init(did, iw, ih);
  if (baum == NULL) {
    fprintf(stderr, "Could not build OpenCL kernels.\n");
    exit(-1);
  }

  printf("OpenCL device : %s\n", baum_getDeviceName(baum));

  if (baum_loadPlan(baum, "baum_plan.txt") != 0) {
    printf("Could not load plan.\n");
  }

  fflush(stdout);

  // baum_malloc allocates pinned memory, that can be quickly accessed from the OpenCL device
  mpimg = baum_malloc(baum, ws * ih);
  result[0] = (int *)baum_malloc(baum, RESULTSIZE);
  result[1] = (int *)baum_malloc(baum, RESULTSIZE);

  baum_enqueueTask(baum, result[(resultPage++) & 1], RESULTSIZE, iimg->imageData, iw, ih, ws);

  //

  tw = (double)iw / MAPSIZE;
  th = (double)ih / MAPSIZE;

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

  glutInitWindowSize(1024, 768);
  glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-glutGet(GLUT_INIT_WINDOW_WIDTH))/2,
			 (glutGet(GLUT_SCREEN_HEIGHT)-glutGet(GLUT_INIT_WINDOW_HEIGHT))/2);

  window = glutCreateWindow("BAUM Demo");

  tm = currentTimeMillis();

  glutDisplayFunc(&display);  
  glutIdleFunc(&display);
  glutReshapeFunc(&reshape);
  glutCloseFunc(&windowClosed);

  init();

  glutMainLoop();  

  exit(0);
}
