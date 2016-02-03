package org.naokishibata.ocvimgio;

import java.util.*;

/**
   An instance of this class encapsulates an CvCapture instance.
 */
public class CvCapture {
    public static final int CV_CAP_PROP_POS_MSEC =       0;
    public static final int CV_CAP_PROP_POS_FRAMES =     1;
    public static final int CV_CAP_PROP_POS_AVI_RATIO =  2;
    public static final int CV_CAP_PROP_FRAME_WIDTH =    3;
    public static final int CV_CAP_PROP_FRAME_HEIGHT =   4;
    public static final int CV_CAP_PROP_FPS =            5;
    public static final int CV_CAP_PROP_FOURCC =         6;
    public static final int CV_CAP_PROP_FRAME_COUNT =    7;
    public static final int CV_CAP_PROP_FORMAT =         8;
    public static final int CV_CAP_PROP_MODE =           9;
    public static final int CV_CAP_PROP_BRIGHTNESS =    10;
    public static final int CV_CAP_PROP_CONTRAST =      11;
    public static final int CV_CAP_PROP_SATURATION =    12;
    public static final int CV_CAP_PROP_HUE =           13;
    public static final int CV_CAP_PROP_GAIN =          14;
    public static final int CV_CAP_PROP_EXPOSURE      = 15;
    public static final int CV_CAP_PROP_CONVERT_RGB   = 16;
    public static final int CV_CAP_PROP_WHITE_BALANCE_BLUE_U = 17;
    public static final int CV_CAP_PROP_RECTIFICATION = 18;
    public static final int CV_CAP_PROP_MONOCROME     = 19;
    public static final int CV_CAP_PROP_SHARPNESS     = 20;
    public static final int CV_CAP_PROP_AUTO_EXPOSURE = 21;
    public static final int CV_CAP_PROP_GAMMA         = 22;
    public static final int CV_CAP_PROP_TEMPERATURE   = 23;
    public static final int CV_CAP_PROP_TRIGGER       = 24;
    public static final int CV_CAP_PROP_TRIGGER_DELAY = 25;
    public static final int CV_CAP_PROP_WHITE_BALANCE_RED_V = 26;
    public static final int CV_CAP_PROP_ZOOM          =27;
    public static final int CV_CAP_PROP_FOCUS         =28;
    public static final int CV_CAP_PROP_GUID          =29;
    public static final int CV_CAP_PROP_ISO_SPEED     =30;
    public static final int CV_CAP_PROP_MAX_DC1394    =31;
    public static final int CV_CAP_PROP_BACKLIGHT     =32;
    public static final int CV_CAP_PROP_PAN           =33;
    public static final int CV_CAP_PROP_TILT          =34;
    public static final int CV_CAP_PROP_ROLL          =35;
    public static final int CV_CAP_PROP_IRIS          =36;
    public static final int CV_CAP_PROP_SETTINGS      =37;

    private byte[] pointer;

    static HashMap<Integer, CvCapture> instanceMap = new HashMap<Integer, CvCapture>();

    static class Releaser implements Runnable {
	public void run() {
	    synchronized(instanceMap) {
		for(CvCapture c : instanceMap.values()) {
		    synchronized(c) {
			OpenCVJNI.cvReleaseCapture(c.pointer);
			c.pointer = null;
		    }
		}
	    }
	}
    }

    static {
	Runtime.getRuntime().addShutdownHook(new Thread(new Releaser()));
    }

    private CvCapture(byte[] p) {
	pointer = new byte[p.length];

	for(int i=0;i<p.length;i++) {
	    pointer[i] = p[i];
	}
    }

    /**
     * Create an CvCapture instance by calling cvCreateCameraCapture.
     * @param index The index of camera
     */
    public static CvCapture cvCreateCameraCapture(int index) {
	if (index < 0) index = 0;

	synchronized(instanceMap) {
	    CvCapture c = instanceMap.get(index);
	    if (c == null) {
		byte[] p = OpenCVJNI.cvCreateCameraCapture(index);
		if (OpenCVJNI.isNULL(p)) return null;

		c = new CvCapture(p);

		instanceMap.put(index, c);
	    }

	    return c;
	}
    }

    /**
     * Create an CvCapture instance by calling cvCreateFileCapture.
     * @param fn File name to read
     */
    public static CvCapture cvCreateFileCapture(String fn) {
	byte[] bfn = fn.getBytes();
	byte[] bfn1k = new byte[1024];
	for(int i=0;i<bfn.length && i < 1023;i++) {
	    bfn1k[i] = bfn[i];
	}

	byte[] p = OpenCVJNI.cvCreateFileCapture(bfn1k);
	if (OpenCVJNI.isNULL(p)) return null;

	CvCapture c = new CvCapture(p);
	return c;
    }

    /**
     * Release the resource allocated for this instance by calling
     * cvReleaseCapture.
     */
    public synchronized void close() {
	if (pointer == null) return;

	synchronized(instanceMap) {
	    OpenCVJNI.cvReleaseCapture(pointer);
	    pointer = null;
	    instanceMap.remove(this);
	}
    }

    public synchronized int cvGrabFrame() {
	if (pointer == null) return 0;
	return OpenCVJNI.cvGrabFrame(pointer);
    }

    public synchronized IplImage cvRetrieveFrame() {
	if (pointer == null) return null;

	byte[] p = OpenCVJNI.cvRetrieveFrame(pointer);
	if (OpenCVJNI.isNULL(p)) return null;

	return new IplImage(p, IplImage.MODE_NO_MODIFICATION);
    }

    public synchronized IplImage cvQueryFrame() {
	if (pointer == null) return null;

	byte[] p = OpenCVJNI.cvQueryFrame(pointer);
	if (OpenCVJNI.isNULL(p)) return null;

	return new IplImage(p, IplImage.MODE_NO_MODIFICATION);
    }

    public synchronized double cvGetCaptureProperty(int property_id) {
	if (pointer == null) return 0;

	return OpenCVJNI.cvGetCaptureProperty(pointer, property_id);
    }

    public synchronized int cvSetCaptureProperty(int property_id, double value) {
	if (pointer == null) return 0;

	return OpenCVJNI.cvSetCaptureProperty(pointer, property_id, value);
    }
}
