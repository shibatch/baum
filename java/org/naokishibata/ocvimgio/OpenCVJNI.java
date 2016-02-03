package org.naokishibata.ocvimgio;

import java.util.*;
import java.io.*;
import java.nio.*;

class OpenCVJNI {
    static native byte[] cvCreateCameraCapture(int n);
    static native byte[] cvCreateFileCapture(byte[] ptrFileName);
    static native void   cvReleaseCapture(byte[] ptrCVCapture);
    static native int    cvGrabFrame(byte[] ptrCVCapture);
    static native byte[] cvRetrieveFrame(byte[] ptrCVCapture);
    static native byte[] cvQueryFrame(byte[] ptrCVCapture);
    static native double cvGetCaptureProperty(byte[] ptrCVCapture, int property_id);
    static native int    cvSetCaptureProperty(byte[] ptrCVCapture, int property_id, double value);

    static native byte[] cvCreateVideoWriter(byte[] ptrFileName, int fourcc, double fps, int width, int height, int is_color);
    static native void   cvReleaseVideoWriter(byte[] ptrWriter);
    static native int    cvWriteFrame(byte[] ptrWriter, byte[] ptrIplImage);

    static native byte[] cvCreateImage(int width, int height, int depth, int channels);
    static native void   cvReleaseImage(byte[] ptrIplImage);
    static native int[]  decodeIplImage(byte[] ptrIplImage);
    static native byte[] imageDataIplImage(byte[] ptrIplImage);
    static native ByteBuffer imageDataIplImageDirect(byte[] ptrIplImage);
    static native byte[] cvCloneImage(byte[] ptrIplImage);
    static native void   cvtColor(byte[] ptrInputArray, byte[] ptrOutputArray, int code, int dstCn);
    static native void   cvFlip(byte[] ptrInputArray, byte[] ptrOutputArray, int mode);

    static native byte[] cvVersion();

    static boolean isNULL(byte[] ptr) {
	for(int i=0;i<ptr.length;i++) {
	    if (ptr[i] != 0) return false;
	}

	return true;
    }

    static {
	String fn = System.getProperty("org.naokishibata.ocvimgio.jnilibpath");
	try {
	    if (fn == null) {
		System.loadLibrary("ocvimgio");
	    } else {
		File f = new File(fn);
		f.setExecutable(true);
		System.load(f.getAbsolutePath());
	    }
	} catch(UnsatisfiedLinkError e) {
	    System.out.println("OpenCVJNI : could not load dll.");
	    System.out.println("Set a correct dll file path to org.naokishibata.ocvimgio.jnilibpath property.");
	    System.out.println("Make sure that the dll file has an appropriate permission.");
	    System.out.println();
	}
    }
}
