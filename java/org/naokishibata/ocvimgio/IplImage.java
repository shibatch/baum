package org.naokishibata.ocvimgio;

import java.util.*;
import java.awt.image.*;
import java.nio.*;

/**
   An instance of this class encapsulates an IplImage instance.
 */
public class IplImage {
    public static final int IPL_DEPTH_SIGN = 0x80000000;
    public static final int IPL_DEPTH_1U = 1;
    public static final int IPL_DEPTH_8U = 8;
    public static final int IPL_DEPTH_16U = 16;
    public static final int IPL_DEPTH_32F = 32;

    public static final int IPL_DEPTH_8S  = (IPL_DEPTH_SIGN| 8);
    public static final int IPL_DEPTH_16S = (IPL_DEPTH_SIGN|16);
    public static final int IPL_DEPTH_32S = (IPL_DEPTH_SIGN|32);

    byte[] pointer;
    int mode;

    private int[] body = null;
    private byte[] imageData = null;

    static final int MODE_NO_MODIFICATION = 1;

    IplImage(byte[] p, int mode) {
	pointer = new byte[p.length];

	for(int i=0;i<p.length;i++) {
	    pointer[i] = p[i];
	}

	this.mode = mode;
    }

    /**
       Make an IplImage instance by calling cvCreateImage.
    */
    public IplImage(int width, int height) {
	pointer = OpenCVJNI.cvCreateImage(width, height, 8, 3);
	refreshBody();
    }

    /**
       Make an IplImage instance by calling cvCreateImage.
    */
    public IplImage(int width, int height, int depth, int nch) {
	pointer = OpenCVJNI.cvCreateImage(width, height, depth, nch);
	refreshBody();
    }

    /**
       Make a clone of IplImage instance by calling cvCloneImage.
    */
    public IplImage(IplImage img) {
	pointer = OpenCVJNI.cvCloneImage(img.pointer);
	refreshBody();
    }

    private void refreshBody() {
	if (body == null) {
	    synchronized(this) {
		if (body == null && pointer != null) {
		    body = OpenCVJNI.decodeIplImage(pointer);
		}
	    }
	}
    }

    private void refreshData() {
	if (imageData == null && pointer != null) {
	    synchronized(this) {
		if (imageData == null && pointer != null) {
		    imageData = OpenCVJNI.imageDataIplImage(pointer);
		}
	    }
	}
    }

    public int nSize() {
	refreshBody();
	return body[0];
    }

    public int ID() {
	refreshBody();
	return body[1];
    }

    public int nChannels() {
	refreshBody();
	return body[2];
    }

    public int depth() {
	refreshBody();
	return body[4];
    }

    public int dataOrder() {
	refreshBody();
	return body[5];
    }

    public int origin() {
	refreshBody();
	return body[6];
    }

    public int align() {
	refreshBody();
	return body[7];
    }

    public int width() {
	refreshBody();
	return body[8];
    }

    public int height() {
	refreshBody();
	return body[9];
    }

    public int imageSize() {
	refreshBody();
	return body[10];
    }

    public int widthStep() {
	refreshBody();
	return body[11];
    }

    public int colorMode() {
	refreshBody();
	return body[12];
    }

    public int channelSeq() {
	refreshBody();
	return body[13];
    }

    /**
       Returns a ByteBuffer instance that points the imageData of this IplImage object.
    */
    public ByteBuffer getDirect() {
	synchronized(this) {
	    if (pointer != null) return OpenCVJNI.imageDataIplImageDirect(pointer);
	}
	return null;
    }

    static boolean bb2bgrimg(BufferedImage image, ByteBuffer bb) {
	if (image.getType() != BufferedImage.TYPE_3BYTE_BGR) return false;
	if (bb.capacity() < image.getWidth() * image.getHeight() * 3) return false;

	byte[] ba = ((DataBufferByte)(image.getTile(0, 0).getDataBuffer())).getData();
	bb.rewind();
	bb.get(ba, 0, image.getWidth() * image.getHeight() * 3);

	return true;
    }

    static boolean bgrimg2bb(ByteBuffer bb, BufferedImage image) {
	if (image.getType() != BufferedImage.TYPE_3BYTE_BGR) return false;
	if (bb.capacity() < image.getWidth() * image.getHeight() * 3) return false;

	byte[] ba = ((DataBufferByte)(image.getTile(0, 0).getDataBuffer())).getData();
	bb.rewind();
	bb.put(ba, 0, image.getWidth() * image.getHeight() * 3);

	return true;
    }

    /**
       Copies the current image to a BufferedImage instance and return it.
    */
    public BufferedImage getImage() {
	int w = width(), h = height(), ws = widthStep();

	BufferedImage img = new BufferedImage(w, h, BufferedImage.TYPE_3BYTE_BGR);

	if (nChannels() != 3 || dataOrder() != 0 || depth() != 8 || imageSize() != widthStep()*height()) {
	    Thread.dumpStack();
	    System.exit(-1);
	}

	refreshData();

	bb2bgrimg(img, getDirect());

	return img;
    }
	
    /**
       Copies a BufferedImage instance to this IplImage instance
    */
    public void setImage(BufferedImage bimg) {
	if ((mode & MODE_NO_MODIFICATION) != 0) return;

	int w = Math.min(bimg.getWidth() , width());
	int h = Math.min(bimg.getHeight(), height());

	ByteBuffer bb = getDirect();

	if (!bgrimg2bb(bb, bimg)) {
	    int[] a = new int[w];

	    for(int y=0;y<h;y++) {
		bimg.getRGB(0, y, w, 1, a, 0, 0);
		for(int x=0;x<w;x++) {
		    bb.put((byte)(a[x] >>  0));
		    bb.put((byte)(a[x] >>  8));
		    bb.put((byte)(a[x] >> 16));
		}
	    }
	}
    }

    /**
       Release the resource allocated for this IplImage by calling
       cvReleaseImage.
    */
    public void dispose() {
	if ((mode & MODE_NO_MODIFICATION) != 0) return;

	synchronized(this) {
	    if (pointer != null) {
		OpenCVJNI.cvReleaseImage(pointer);
		pointer = null;
	    }
	}
    }
}
