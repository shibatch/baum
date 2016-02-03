// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This software is in public domain. You can use and modify this code
// for any purpose without any obligation.


#ifdef _MSC_VER
#include <Windows.h>
#include <WinBase.h>
#include <Commdlg.h>
#include <atltypes.h>
#else
#include <gtk/gtk.h>
#define GL_GLEXT_PROTOTYPES
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

#include <CL/cl.h>

#include <GL/glui.h>

#include <cv.h>
#include <highgui.h>

#include "helper.h"
#include "poly.h"
#include "oclhelper.h"
#include "baum.h"

#define ESCAPE 27
#define MAPSIZE 2048

uint8_t *mappedBuffer;
GLuint texName, pboName;
GLsync syncName = 0;
int fallBack = 0;

//

double tw, th;
int iw, ih, ws, ws3;

CvCapture *capture = NULL;
IplImage *iimg = NULL;

int window;
GLUI *glui;
GLUI_Button *butConfig, *butCamConfig;
GLUI_Listbox *lbCam, *lbDevice;
GLUI_Checkbox *cbPause, *cbSlow;
GLUI_StaticText *stMessage;
int lvPause=0, lvSlow=0;

char curDir[1024] = "";

//

#define MAXDEVICES 10
cl_device_id devices[MAXDEVICES];
int nDevices = 0;

#define RESULTSIZE (MAPSIZE * MAPSIZE * 4)
int selectedDevice = -1;
baum_t *baum[MAXDEVICES];
int *result[MAXDEVICES][2], resultPage[MAXDEVICES];
void *mpimg[MAXDEVICES];

//

int frameCount = 0, totalFrameCount = 0, lastFrameNum = -1;
long long int tm, tm2;
unsigned char message[256];

//

#ifdef _MSC_VER
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;
PFNGLBUFFERSTORAGEPROC glBufferStorage;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;

PFNGLFENCESYNCPROC glFenceSync;
PFNGLDELETESYNCPROC glDeleteSync;
PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
PFNGLGETSYNCIVPROC glGetSynciv;

OPENFILENAME ofn;
char fn[100];

void showMessageBox(const char *title, const char *message) {
  MessageBox (NULL, message, title, MB_OK);
}

UINT_PTR CALLBACK OFNHookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_INITDIALOG) {
    RECT r;
    int w, h, x, y;
    GetWindowRect(GetParent(hWnd), &r);
    w = r.right - r.left;
    h = r.bottom - r.top;
    GetWindowRect(GetDesktopWindow(), &r);
    MoveWindow(GetParent(hWnd), (r.right-w)/2,(r.bottom-h)/2, w, h, TRUE);
    return TRUE;
  }
  return 0;
}

char *showFileChooser() {
  ofn.lStructSize = sizeof (ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = fn;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof( fn );
  ofn.lpstrFilter = "All\0*.*\0\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir=NULL;
  ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_ENABLEHOOK|OFN_EXPLORER;
  ofn.lpfnHook = OFNHookProc;

  if (!GetOpenFileName( &ofn )) return NULL;

  return ofn.lpstrFile;
}
#else
void showMessageBox(const char *title, const char *message) {
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(NULL,
				  GTK_DIALOG_DESTROY_WITH_PARENT,
				  GTK_MESSAGE_INFO,
				  GTK_BUTTONS_OK,
				  "%s", message);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  while(gtk_events_pending()) gtk_main_iteration();
}

char *showFileChooser() {
  GtkWidget *fileChooserDialog;

  fileChooserDialog = gtk_file_chooser_dialog_new("Open File", NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
						  "Cancel", GTK_RESPONSE_CANCEL,
						  "Open", GTK_RESPONSE_ACCEPT, NULL);

  gint res = gtk_dialog_run(GTK_DIALOG (fileChooserDialog));
  if (res == GTK_RESPONSE_ACCEPT) {
      char *filename;
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (fileChooserDialog);
      filename = gtk_file_chooser_get_filename(chooser);
      gtk_widget_destroy(fileChooserDialog);
      while(gtk_events_pending()) gtk_main_iteration();
      return filename;
      //g_free (filename);
    }
  while(gtk_events_pending()) gtk_main_iteration();
  gtk_widget_destroy(fileChooserDialog);
  return NULL;
}
#endif

void init() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 0, 1, -1, 1);

  if (!fallBack) {
    glGenBuffers(1, &pboName);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboName);
    glBufferStorage(GL_PIXEL_UNPACK_BUFFER, MAPSIZE * MAPSIZE * 3, NULL,
		    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT /* | GL_MAP_COHERENT_BIT*/);
    mappedBuffer = (uint8_t *)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, MAPSIZE * MAPSIZE * 3, 
					       GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT /* | GL_MAP_COHERENT_BIT*/);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }

  glGenTextures(1, &texName);

  glBindTexture(GL_TEXTURE_2D, texName);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, MAPSIZE, MAPSIZE, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  tm = tm2 = currentTimeMillis();
}

void reshape(int width, int height) {
  int x, y;
  GLUI_Master.get_viewport_area(&x, &y, &width, &height);
  glViewport(x, y, width, height);
  
  double height0 = (double)height / ih * iw;
  double size = width < height0 ? width : height0;
  double hpad = (double)(width   - size) / size / 2;
  double vpad = (double)(height0 - size) / size / 2;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-hpad, 1 + hpad, -vpad, 1 + vpad, -1, 1);
  //glFrustum(-hpad, 1 + hpad, -vpad, 1 + vpad,  1, 10000);
}

void display() {
  glutSetWindow(window);

  glClear( GL_COLOR_BUFFER_BIT );

  if (iimg != NULL) {
    if (!fallBack) {
      if(syncName != 0) {
	for(;;) {
#if 0
	  GLenum ret = glClientWaitSync( syncName, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	  if (ret == GL_ALREADY_SIGNALED || ret == GL_CONDITION_SATISFIED) break;
	  sleepMillis(2);
#else
	  GLint ret;
	  glGetSynciv(syncName, GL_SYNC_STATUS, sizeof(GLint), NULL, &ret);
	  if (ret == GL_SIGNALED) break;
	  sleepMillis(2);
#endif
	}
	glDeleteSync(syncName);
	syncName = 0;
      }

      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboName);
      memcpy(mappedBuffer, iimg->imageData, ws * ih);
      glFlushMappedBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, ws * ih);
    
      glBindTexture(GL_TEXTURE_2D, texName);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ws3, ih, GL_BGR, GL_UNSIGNED_BYTE, NULL);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

      syncName = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    } else {
      glBindTexture(GL_TEXTURE_2D, texName);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ws3, ih, GL_BGR, GL_UNSIGNED_BYTE, iimg->imageData);
    }

    glEnable(GL_TEXTURE_2D);

    glBegin( GL_QUADS );
    glTexCoord2d(0.0,th ); glVertex3d(0.0,0.0, -1.0);
    glTexCoord2d(tw ,th ); glVertex3d(1.0,0.0, -1.0);
    glTexCoord2d(tw ,0.0); glVertex3d(1.0,1.0, -1.0);
    glTexCoord2d(0.0,0.0); glVertex3d(0.0,1.0, -1.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
  }
    
  //

  frameCount++;
  totalFrameCount++;

  long long int t = currentTimeMillis();
  if (t - tm > 1000) {
    sprintf((char *)message, "%.1f fps", frameCount * 1000.0 / (t - tm));
    tm += 1000;
    frameCount = 0;

    if (iimg != NULL) {
      stMessage->set_text((char *)message);
    } else {
      stMessage->set_text((char *)glGetString(GL_RENDERER));
    }
  }

  //
  
  if (capture != NULL) {
    int captureNewFrame = 1;

    if (lvSlow) {
      long long int t = currentTimeMillis();
      if (t - tm2 > 500) {
	tm2 = t;
      } else {
	captureNewFrame = 0;
      }
    }

    if (lvPause) captureNewFrame = 0;
    
    if (captureNewFrame) {
      iimg = cvRetrieveFrame(capture);
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
      } else {
	cvGrabFrame(capture);
      }
    }
  }

  if (iimg != NULL && selectedDevice != -1) {
    memcpy(mpimg[selectedDevice], iimg->imageData, ws * ih);
    baum_enqueueTask(baum[selectedDevice], result[selectedDevice][(resultPage[selectedDevice]++) & 1], RESULTSIZE, mpimg[selectedDevice], iw, ih, ws);
    if (baum_queueLen(baum[selectedDevice]) > 1) {
      baum_poll(baum[selectedDevice], 1);

#define SIZE_OF_DECODE_T (4 * 8)

      uint8_t *rdata = (uint8_t *)result[selectedDevice][resultPage[selectedDevice] & 1];

      ArrayMap *am = initArrayMap();

      for(int i=1;i<result[selectedDevice][resultPage[selectedDevice] & 1][0];i++) {
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
	  //glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, str);
	  glutBitmapString(GLUT_BITMAP_8_BY_13, str);
	}

	Poly12_dispose(vaam[i]);
      }

      free(vaam);
      free(kaam);
      ArrayMap_dispose(am);
    }
  }

  glutSwapBuffers();

  //
  
  if (!fallBack) { // This is a workaround to reduce CPU usage by GLUI
    static int64_t lastDisplay = 0;
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    int t2s = 16 - (int)(currentTimeMillis() - lastDisplay);
    if (lastDisplay > 0 && t2s > 0) sleepMillis(t2s);

    for(;;) {
      GLint ret;
      glGetSynciv(sync, GL_SYNC_STATUS, sizeof(GLint), NULL, &ret);
      if (ret == GL_SIGNALED) break;
      sleepMillis(2);
    }

    glDeleteSync(sync);
    
    lastDisplay = currentTimeMillis();
  } else {
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

void setupCapture() {
  iimg = cvQueryFrame(capture);
  cvGrabFrame(capture);

  if (iimg->nChannels != 3) {
    showMessageBox("Opening a capture device", "nChannels != 3");
    cvReleaseCapture(&capture);
    capture = NULL;
    return;
  }

  iw = iimg->width;
  ih = iimg->height;
  ws = iimg->widthStep;
  ws3 = (ws + 2) / 3;

  tw = (double)iw / MAPSIZE;
  th = (double)ih / MAPSIZE;

  glutSetWindow(window);
  reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

void lbDeviceCallback() {
  if (selectedDevice != -1) {
    while(baum_queueLen(baum[selectedDevice]) != 0) {
      baum_poll(baum[selectedDevice], 1);
    }
  }

  selectedDevice = lbDevice->get_int_val();
  if (selectedDevice == -1) return;

  if (baum[selectedDevice] == NULL) {
    baum[selectedDevice] = baum_init(selectedDevice, MAPSIZE, MAPSIZE);
    if (baum[selectedDevice] == NULL) {
      showMessageBox("Initializing kernels", "Could not build OpenCL kernels.");
      selectedDevice = -1;
      lbDevice->set_int_val(-1);
      return;
    }
#ifdef _MSC_VER
    SetCurrentDirectory(curDir);
#else
    if (chdir(curDir));
#endif
    if (baum_loadPlan(baum[selectedDevice], "baum_plan.txt") != 0) {
      showMessageBox("Loading a plan", "Could not load a plan.\nExecution speed may be slower.");
    }

    mpimg[selectedDevice] = baum_malloc(baum[selectedDevice], MAPSIZE * MAPSIZE * 3);
    result[selectedDevice][0] = (int *)baum_malloc(baum[selectedDevice], RESULTSIZE);
    result[selectedDevice][1] = (int *)baum_malloc(baum[selectedDevice], RESULTSIZE);
  }
}

void lbCamCallback() {
  if (capture != NULL) cvReleaseCapture(&capture);
  capture = NULL;
  iimg = NULL;

  int sel = lbCam->get_int_val();
  if (sel == -1) {
    char *fn = showFileChooser();
    if (fn == NULL) return;

    iimg = cvLoadImage(fn, CV_LOAD_IMAGE_COLOR);
    if( iimg != NULL ) {
      iw = iimg->width;
      ih = iimg->height;
      ws = iimg->widthStep;
      ws3 = (ws + 2) / 3;

      tw = (double)iw / MAPSIZE;
      th = (double)ih / MAPSIZE;

      glutSetWindow(window);
      reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

      return;
    }

#define CV_VERSION_NUM CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
    
    capture = cvCreateFileCapture(fn);
    if( capture == NULL ) {
      showMessageBox("Opening a video file", "Could not open the video file.\nYou need opencv_ffmpeg" CV_VERSION_NUM "_64.dll to decode videos.");
      return;
    }

    setupCapture();
  } else if (0 <= sel && sel <= 9) {
    capture = cvCreateCameraCapture(sel);
    if ( capture == NULL ) {
      showMessageBox("Opening a camera", "Camera not available");
      return;
    }
    
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH , 1920);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 1080);

    setupCapture();
  }
}

void butConfigCallback() {
  if (capture != NULL) cvSetCaptureProperty(capture, CV_CAP_PROP_SETTINGS, 1);
}

int main(int argc, char **argv) {
  int argc2 = argc;
  glutInit(&argc2, argv);

#ifdef _MSC_VER
  SetProcessDPIAware();
  if (GetModuleFileName(NULL, curDir, 1000)) {
    char *p = curDir;
    while(p - curDir < 1010 && *p) p++;
    while(p > curDir && *p != '\\') p--;
    *p = '\0';
    SetCurrentDirectory(curDir);
  }
#else
#ifdef __linux__
  if (readlink("/proc/self/exe", curDir, 1000) != -1) {
    char *p = curDir;
    while(p - curDir < 1010 && *p) p++;
    while(p > curDir && *p != '\\') p--;
    *p = '\0';
    if (chdir(curDir));
  }
#else
  {
    char *p = realpath(argv[0], NULL);
    if (p != NULL) {
      strncpy(curDir, p, 1000);
      free(p);
      p = curDir;
      while(p - curDir < 1010 && *p) p++;
      while(p > curDir && *p != '\\') p--;
      *p = '\0';
      if (chdir(curDir)) ;
    }
  }
#endif
  gtk_init(&argc, &argv);
#endif
  
  nDevices = simpleGetDevices(devices, MAXDEVICES);
  
  atexit(whenexits);
  
  iw = 640;
  ih = 480;

  tw = 1.0;
  th = 1.0;

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE );

  glutInitWindowSize(1024, 768);
  glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-glutGet(GLUT_INIT_WINDOW_WIDTH))/2,
			 (glutGet(GLUT_SCREEN_HEIGHT)-glutGet(GLUT_INIT_WINDOW_HEIGHT))/2);

  window = glutCreateWindow("BAUM Demo");

  GLUI_Master.set_glutDisplayFunc(&display);  
  GLUI_Master.set_glutIdleFunc(&display);
  GLUI_Master.set_glutReshapeFunc(&reshape);
  glutCloseFunc(&windowClosed);

  //

  glui = GLUI_Master.create_glui_subwindow(window, GLUI_SUBWINDOW_BOTTOM);

  lbCam = glui->add_listbox("", NULL, -1, (GLUI_Update_CB)lbCamCallback);
  lbCam->add_item( -2, " Choose Capture " );
  lbCam->add_item( -1, "      File      " );
  lbCam->add_item(  0, "    Camera 0    " );
  lbCam->add_item(  1, "    Camera 1    " );
  lbCam->add_item(  2, "    Camera 2    " );
  lbCam->add_item(  3, "    Camera 3    " );
  lbCam->add_item(  4, "    Camera 4    " );
  lbCam->add_item(  5, "    Camera 5    " );
  lbCam->add_item(  6, "    Camera 6    " );
  lbCam->add_item(  7, "    Camera 7    " );
  lbCam->add_item(  8, "    Camera 8    " );
  lbCam->add_item(  9, "    Camera 9    " );

  glui->add_column(0);
  
  butConfig = glui->add_button("Config", 0, (GLUI_Update_CB)butConfigCallback);

  glui->add_column(1);

  cbPause = glui->add_checkbox("Pause", &lvPause, -1, (GLUI_Update_CB)NULL);
  glui->add_column(0);
  cbSlow  = glui->add_checkbox("Slow" , &lvSlow , -1, (GLUI_Update_CB)NULL);

  glui->add_column(1);
  
  lbDevice = glui->add_listbox("", NULL, -1, (GLUI_Update_CB)lbDeviceCallback);
  lbDevice->add_item( -1, " Choose Device " );

  for(int i=0;i<nDevices;i++) {
    lbDevice->add_item( i, getDeviceName(devices[i]) );
  }

  glui->add_column(1);

  stMessage = glui->add_statictext("  0.0 fps");
  
  //
  
  glui->set_main_gfx_window( window );
  
  //

  float glVersion = 0;
  sscanf((const char *)glGetString(GL_VERSION), "%f", &glVersion);

  if (glVersion < 4.4) {
    char mes[100];
    sprintf(mes, "OpenGL version = %.1f", glVersion);
    showMessageBox("OpenGL version", mes);
    fallBack = 1;
  } else {
#ifdef _MSC_VER
    glGenBuffers = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
    glMapBuffer = (PFNGLMAPBUFFERPROC) wglGetProcAddress("glMapBuffer");
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) wglGetProcAddress("glUnmapBuffer");
    glBufferStorage = (PFNGLBUFFERSTORAGEPROC) wglGetProcAddress("glBufferStorage");
    glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) wglGetProcAddress("glMapBufferRange");
    glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) wglGetProcAddress("glFlushMappedBufferRange");
    glFenceSync = (PFNGLFENCESYNCPROC) wglGetProcAddress("glFenceSync");
    glDeleteSync = (PFNGLDELETESYNCPROC) wglGetProcAddress("glDeleteSync");
    glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) wglGetProcAddress("glClientWaitSync");
    glGetSynciv = (PFNGLGETSYNCIVPROC) wglGetProcAddress("glGetSynciv");

    assert(glGenBuffers != NULL);
    assert(glBindBuffer != NULL);
    assert(glBufferData != NULL);
    assert(glMapBuffer != NULL);
    assert(glUnmapBuffer != NULL);
    assert(glBufferStorage != NULL);
    assert(glMapBufferRange != NULL);
    assert(glFlushMappedBufferRange != NULL);
    assert(glFenceSync != NULL);
    assert(glDeleteSync != NULL);
    assert(glClientWaitSync != NULL);
    assert(glGetSynciv != NULL);
#endif
  }

  //
  
  init();

  glutMainLoop();  

  exit(0);
}
