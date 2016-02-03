package org.naokishibata.baum;

import java.util.*;
import java.io.*;
import java.nio.*;

class BaumJNI {
    static native byte[] baum_init(int did, int iw, int ih);
    static native void   baum_dispose(byte[] thiz);
    static native int    baum_enqueueTask(byte[] thiz, ByteBuffer result, long bufSize, ByteBuffer img, int iw, int ih, int ws);
    static native int    baum_poll(byte[] thiz, int waitFlag);
    static native int    baum_queueLen(byte[] thiz);
    static native void   baum_createPlan(byte[] thiz, byte[] path, ByteBuffer result, long bufSize, ByteBuffer img, int iw, int ih, int ws);
    static native int    baum_loadPlan(byte[] thiz, byte[] path);
    static native byte[] baum_getDeviceName(byte[] thiz);
    static native byte[] baum_getDeviceName(int did);
    static native ByteBuffer baum_malloc(byte[] thiz, long z);
    static native void   baum_free(byte[] thiz, ByteBuffer bb);

    static boolean isNULL(byte[] ptr) {
	for(int i=0;i<ptr.length;i++) {
	    if (ptr[i] != 0) return false;
	}

	return true;
    }

    static {
	String fn = System.getProperty("org.naokishibata.baum.jnilibpath");
	try {
	    if (fn == null) {
		System.loadLibrary("baumjni");
	    } else {
		File f = new File(fn);
		f.setExecutable(true);
		System.load(f.getAbsolutePath());
	    }
	} catch(UnsatisfiedLinkError e) {
	    System.out.println("BaumJNI : could not load dll.");
	    System.out.println("Set a correct dll file path to org.naokishibata.baum.jnilibpath property.");
	    System.out.println("Make sure that the dll file has an appropriate permission.");
	    System.out.println();
	}
    }
}
