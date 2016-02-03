package org.naokishibata.ocvimgio;

import java.util.*;
import java.awt.image.*;
import java.nio.*;
import java.io.*;

public class CvUtil {
    public static final int CV_BGR2BGRA    = 0;
    public static final int CV_BGRA2BGR    = 1;
    public static final int CV_BGR2RGBA    = 2;
    public static final int CV_RGBA2BGR    = 3;
    public static final int CV_BGR2RGB     = 4;
    public static final int CV_BGRA2RGBA   = 5;
    public static final int CV_BGR2GRAY    = 6;
    public static final int CV_RGB2GRAY    = 7;
    public static final int CV_GRAY2BGR    = 8;
    public static final int CV_GRAY2BGRA   = 9;
    public static final int CV_BGRA2GRAY   = 10;
    public static final int CV_RGBA2GRAY   = 11;
    public static final int CV_BGR2BGR565  = 12;
    public static final int CV_RGB2BGR565  = 13;
    public static final int CV_BGR5652BGR  = 14;
    public static final int CV_BGR5652RGB  = 15;
    public static final int CV_BGRA2BGR565 = 16;
    public static final int CV_RGBA2BGR565 = 17;
    public static final int CV_BGR5652BGRA = 18;
    public static final int CV_BGR5652RGBA = 19;
    public static final int CV_GRAY2BGR565 = 20;
    public static final int CV_BGR5652GRAY = 21;
    public static final int CV_BGR2BGR555  = 22;
    public static final int CV_RGB2BGR555  = 23;
    public static final int CV_BGR5552BGR  = 24;
    public static final int CV_BGR5552RGB  = 25;
    public static final int CV_BGRA2BGR555 = 26;
    public static final int CV_RGBA2BGR555 = 27;
    public static final int CV_BGR5552BGRA = 28;
    public static final int CV_BGR5552RGBA = 29;
    public static final int CV_GRAY2BGR555 = 30;
    public static final int CV_BGR5552GRAY = 31;
    public static final int CV_BGR2XYZ     = 32;
    public static final int CV_RGB2XYZ     = 33;
    public static final int CV_XYZ2BGR     = 34;
    public static final int CV_XYZ2RGB     = 35;
    public static final int CV_BGR2YCrCb   = 36;
    public static final int CV_RGB2YCrCb   = 37;
    public static final int CV_YCrCb2BGR   = 38;
    public static final int CV_YCrCb2RGB   = 39;
    public static final int CV_BGR2HSV     = 40;
    public static final int CV_RGB2HSV     = 41;
    public static final int CV_BGR2Lab     = 44;
    public static final int CV_RGB2Lab     = 45;
    public static final int CV_BayerBG2BGR = 46;
    public static final int CV_BayerGB2BGR = 47;
    public static final int CV_BayerRG2BGR = 48;
    public static final int CV_BayerGR2BGR = 49;
    public static final int CV_BGR2Luv     = 50;
    public static final int CV_RGB2Luv     = 51;
    public static final int CV_BGR2HLS     = 52;
    public static final int CV_RGB2HLS     = 53;
    public static final int CV_HSV2BGR     = 54;
    public static final int CV_HSV2RGB     = 55;
    public static final int CV_Lab2BGR     = 56;
    public static final int CV_Lab2RGB     = 57;
    public static final int CV_Luv2BGR     = 58;
    public static final int CV_Luv2RGB     = 59;
    public static final int CV_HLS2BGR     = 60;
    public static final int CV_HLS2RGB     = 61;
    public static final int CV_BayerBG2BGR_VNG = 62;
    public static final int CV_BayerGB2BGR_VNG = 63;
    public static final int CV_BayerRG2BGR_VNG = 64;
    public static final int CV_BayerGR2BGR_VNG = 65;

    public static final int CV_GRAY2RGBA   = CV_GRAY2BGRA;
    public static final int CV_RGB2RGBA    = CV_BGR2BGRA;
    public static final int CV_RGBA2RGB    = CV_BGRA2BGR;
    public static final int CV_RGB2BGRA    = CV_BGR2RGBA;
    public static final int CV_BGRA2RGB    = CV_RGBA2BGR;
    public static final int CV_RGB2BGR     = CV_BGR2RGB;
    public static final int CV_RGBA2BGRA   = CV_BGRA2RGBA;
    public static final int CV_GRAY2RGB    = CV_GRAY2BGR;
    public static final int CV_BayerBG2RGB = CV_BayerRG2BGR;
    public static final int CV_BayerGB2RGB = CV_BayerGR2BGR;
    public static final int CV_BayerRG2RGB = CV_BayerBG2BGR;
    public static final int CV_BayerGR2RGB = CV_BayerGB2BGR;

    public static void cvCvtColor(IplImage src, IplImage dst, int mode) {
	if ((dst.mode & IplImage.MODE_NO_MODIFICATION) != 0) return;

	OpenCVJNI.cvtColor(src.pointer, dst.pointer, mode, 0);
    }

    public static void cvFlip(IplImage src, IplImage dst, int mode) {
	if ((dst.mode & IplImage.MODE_NO_MODIFICATION) != 0) return;

	OpenCVJNI.cvFlip(src.pointer, dst.pointer, mode);
    }

    public static String versionString() {
	try {
	    return new String(OpenCVJNI.cvVersion(), "US-ASCII");
	} catch(UnsupportedEncodingException ex) {}
	return null;
    }
}
