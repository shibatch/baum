package org.naokishibata.ocvimgio;

import java.util.*;

public class CvVideoWriter {
    private byte[] pointer;

    static HashSet<CvVideoWriter> instanceSet = new HashSet<CvVideoWriter>();

    static class Releaser implements Runnable {
	public void run() {
	    synchronized(instanceSet) {
		for(CvVideoWriter c : instanceSet) {
		    synchronized(c) {
			OpenCVJNI.cvReleaseVideoWriter(c.pointer);
			c.pointer = null;
		    }
		}
	    }
	}
    }

    static {
	Runtime.getRuntime().addShutdownHook(new Thread(new Releaser()));
    }

    private CvVideoWriter(byte[] p) {
	pointer = new byte[p.length];

	for(int i=0;i<p.length;i++) {
	    pointer[i] = p[i];
	}
    }

    /*
       PIM1 MPEG-1
       MJPG motion-jpeg
       MP42 MPEG-4.2
       DIV3 MPEG-4.3
       DIVX MPEG-4
       U263 H263
       I263 H263I
       FLV1 FLV1
     */
    public static CvVideoWriter cvCreateVideoWriter(String fn, String fourcc, double fps, int width, int height, int is_color) {
	byte[] bfc = fourcc.getBytes();
	int ifc = 0;
	for(int i=0;i<Math.min(bfc.length, 4);i++) {
	    ifc += (bfc[i] & 0xff) << (i * 8);
	}

	synchronized(instanceSet) {
	    byte[] bfn = fn.getBytes();
	    byte[] bfn1k = new byte[1024];
	    for(int i=0;i<bfn.length && i < 1023;i++) {
		bfn1k[i] = bfn[i];
	    }

	    byte[] p = OpenCVJNI.cvCreateVideoWriter(bfn1k, ifc, fps, width, height, is_color);

	    if (OpenCVJNI.isNULL(p)) return null;

	    CvVideoWriter c = new CvVideoWriter(p);

	    instanceSet.add(c);

	    return c;
	}
    }

    public synchronized void close() {
	if (pointer == null) return;

	synchronized(instanceSet) {
	    OpenCVJNI.cvReleaseVideoWriter(pointer);
	    pointer = null;
	    instanceSet.remove(this);
	}
    }

    public int cvWriteFrame(IplImage img) {
	if (pointer == null || img.pointer == null) return -1;

	return OpenCVJNI.cvWriteFrame(pointer, img.pointer);
    }
}
