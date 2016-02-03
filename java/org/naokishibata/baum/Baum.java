package org.naokishibata.baum;

import java.util.*;
import java.io.*;
import java.nio.*;
import java.awt.image.*;

public class Baum {
    byte[] thiz;

    public Baum(int did, int iw, int ih) {
	thiz = BaumJNI.baum_init(did, iw, ih);
	if (BaumJNI.isNULL(thiz)) {
	    System.err.println("Baum instance could not be initialized, did = " + did);
	    Thread.dumpStack();
	    System.exit(-1);
	}
    }

    public class Result {
	ByteBuffer bb;
	final int n;

	Result(int z) { n = z; bb = malloc(32 * (z + 1)); }

	public void dispose() { free(bb); bb = null; }

	public int size() { return Math.min(n, bb.getInt(0)-1); }

	public float[] get(int index) {
	    if (index >= size()) return null;
	    return new float[] {
		bb.getFloat(32*index + 0*4), 
		bb.getFloat(32*index + 1*4), 
		bb.getFloat(32*index + 2*4), 
		bb.getFloat(32*index + 3*4), 
		bb.getInt(32*index + 4*4)
	    };
	}
    }

    public Result createResult(int z) { return new Result(z); }

    public void dispose() {
	if (thiz == null) return;
	BaumJNI.baum_dispose(thiz);
	thiz = null;
    }

    public int enqueueTask(Result result, ByteBuffer img, int iw, int ih, int ws) {
	if (thiz == null) return -1;
	return BaumJNI.baum_enqueueTask(thiz, result.bb, result.bb.capacity(), img, iw, ih, ws);
    }

    public int enqueueTask(Result result, ByteBuffer img, int iw, int ih) {
	return enqueueTask(result, img, iw, ih, iw*3);
    }

    public int poll(boolean waitFlag) {
	if (thiz == null) return -1;
	return BaumJNI.baum_poll(thiz, waitFlag ? 1 : 0);
    }

    public int queueLen() {
	if (thiz == null) return -1;
	return BaumJNI.baum_queueLen(thiz);
    }

    public void createPlan(String path, Result result, ByteBuffer img, int iw, int ih, int ws) {
	if (thiz == null) return;
	try {
	    BaumJNI.baum_createPlan(thiz, path.getBytes("US-ASCII"), result.bb, result.bb.capacity(), img, iw, ih, ws);
	} catch(UnsupportedEncodingException ex) {}
    }

    public void createPlan(String path, Result result, ByteBuffer img, int iw, int ih) {
	createPlan(path, result, img, iw, ih, iw*3);
    }

    public int loadPlan(String path) {
	if (thiz == null) return -1;
	try {
	    return BaumJNI.baum_loadPlan(thiz, path.getBytes("US-ASCII"));
	} catch(UnsupportedEncodingException ex) {}
	return -1;
    }

    public String getDeviceName() {
	if (thiz == null) return null;
	try {
	    return new String(BaumJNI.baum_getDeviceName(thiz), "US-ASCII");
	} catch(UnsupportedEncodingException ex) {}
	return null;
    }

    public static String getDeviceName(int did) {
	try {
	    byte[] ba = BaumJNI.baum_getDeviceName(did);
	    if (ba != null) return new String(ba, "US-ASCII");
	} catch(UnsupportedEncodingException ex) {}
	return null;
    }

    //

    public ByteBuffer malloc(long z) {
	if (thiz == null) return null;
	ByteBuffer bb = BaumJNI.baum_malloc(thiz, z);
	bb.order(ByteOrder.nativeOrder());
	return bb;
    }

    public void free(ByteBuffer bb) {
	if (thiz == null) return;
	BaumJNI.baum_free(thiz, bb);
    }

    public static boolean bb2bgrimg(BufferedImage image, ByteBuffer bb) {
	if (image.getType() != BufferedImage.TYPE_3BYTE_BGR) return false;
	if (bb.capacity() < image.getWidth() * image.getHeight() * 3) return false;

	byte[] ba = ((DataBufferByte)(image.getTile(0, 0).getDataBuffer())).getData();
	bb.rewind();
	bb.get(ba, 0, image.getWidth() * image.getHeight() * 3);

	return true;
    }

    public static boolean bgrimg2bb(ByteBuffer bb, BufferedImage image) {
	if (image.getType() != BufferedImage.TYPE_3BYTE_BGR) return false;
	if (bb.capacity() < image.getWidth() * image.getHeight() * 3) return false;

	byte[] ba = ((DataBufferByte)(image.getTile(0, 0).getDataBuffer())).getData();
	bb.rewind();
	bb.put(ba, 0, image.getWidth() * image.getHeight() * 3);

	return true;
    }
}
