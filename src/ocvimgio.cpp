#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <jni.h>

#ifndef _Included_org_naokishibata_ocvimgio_OpenCVJNI
#define _Included_org_naokishibata_ocvimgio_OpenCVJNI
#ifdef __cplusplus
extern "C" {
#endif

#define CV_VERSION_NUM CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

union conv {
  void *ptr;
  signed char array[sizeof(void *)];
};

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvCreateCameraCapture
 * Signature: (I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvCreateCameraCapture
(JNIEnv *env, jclass cls, jint jn) {
	conv cnv;
	cnv.ptr =cvCreateCameraCapture((int)jn);

	jbyteArray ret = env->NewByteArray(sizeof(void *));
	env->SetByteArrayRegion(ret, 0, sizeof(void *), (const signed char *)cnv.array);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvCreateFileCapture
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvCreateFileCapture
  (JNIEnv *env, jclass cls, jbyteArray jptrFileName) {
	char filename[1024];
	env->GetByteArrayRegion(jptrFileName, 0, 1023, (jbyte *)filename);
	filename[1023] = '\0';

	conv cnv;
	cnv.ptr = cvCreateFileCapture(filename);

	jbyteArray ret = env->NewByteArray(sizeof(void *));
	env->SetByteArrayRegion(ret, 0, sizeof(void *), (const signed char *)cnv.array);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvReleaseCapture
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvReleaseCapture
(JNIEnv *env, jclass cls, jbyteArray jptrCvCapture) {
	conv cnv;

	env->GetByteArrayRegion(jptrCvCapture, 0, sizeof(void *), (jbyte *)cnv.array);

	cvReleaseCapture((CvCapture **)&cnv.ptr);
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvGrabFrame
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvGrabFrame
  (JNIEnv *env, jclass cls, jbyteArray jptrCvCapture) {
	conv cnv;

	env->GetByteArrayRegion(jptrCvCapture, 0, sizeof(void *), (jbyte *)cnv.array);

	return (jint)cvGrabFrame((CvCapture *)cnv.ptr);
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvRetrieveFrame
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvRetrieveFrame
(JNIEnv *env, jclass cls, jbyteArray jptrCvCapture) {
	conv cnv;

	env->GetByteArrayRegion(jptrCvCapture, 0, sizeof(void *), (jbyte *)cnv.array);

	IplImage *img = cvRetrieveFrame((CvCapture *)cnv.ptr);

	cnv.ptr = img;

	jbyteArray ret = env->NewByteArray(sizeof(void *));
	env->SetByteArrayRegion(ret, 0, sizeof(void *), (const signed char *)cnv.array);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvQueryFrame
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvQueryFrame
(JNIEnv *env, jclass cls, jbyteArray jptrCvCapture) {
	conv cnv;

	env->GetByteArrayRegion(jptrCvCapture, 0, sizeof(void *), (jbyte *)cnv.array);

	IplImage *img = cvQueryFrame((CvCapture *)cnv.ptr);

	cnv.ptr = img;

	jbyteArray ret = env->NewByteArray(sizeof(void *));
	env->SetByteArrayRegion(ret, 0, sizeof(void *), (const signed char *)cnv.array);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvGetCaptureProperty
 * Signature: ([BI)D
 */
JNIEXPORT jdouble JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvGetCaptureProperty
(JNIEnv *env, jclass cls, jbyteArray jptrCvCapture, jint jproperty_id) {
	conv cnv;

	env->GetByteArrayRegion(jptrCvCapture, 0, sizeof(void *), (jbyte *)cnv.array);

	double ret = cvGetCaptureProperty((CvCapture *)cnv.ptr, (int)jproperty_id);

	return (jdouble)ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvSetCaptureProperty
 * Signature: ([BID)I
 */
JNIEXPORT jint JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvSetCaptureProperty
(JNIEnv *env, jclass cls, jbyteArray jptrCvCapture, jint jproperty_id, jdouble jvalue) {
	conv cnv;

	env->GetByteArrayRegion(jptrCvCapture, 0, sizeof(void *), (jbyte *)cnv.array);

	int ret = cvSetCaptureProperty((CvCapture *)cnv.ptr, (int)jproperty_id, (double)jvalue);

	return (jint)ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvCreateVideoWriter
 * Signature: ([BIDIII)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvCreateVideoWriter
(JNIEnv *env, jclass cls, jbyteArray jptrFileName, jint jfourcc, jdouble jfps, jint jwidth, jint jheight, jint jis_color) {
	char filename[1024];
	env->GetByteArrayRegion(jptrFileName, 0, 1023, (jbyte *)filename);
	filename[1023] = '\0';

	conv cnv;
	cnv.ptr = cvCreateVideoWriter(filename, jfourcc, jfps, cvSize(jwidth, jheight), jis_color);

	jbyteArray ret = env->NewByteArray(sizeof(void *));
	env->SetByteArrayRegion(ret, 0, sizeof(void *), (const signed char *)cnv.array);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvReleaseVideoWriter
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvReleaseVideoWriter
(JNIEnv *env, jclass cls, jbyteArray jptrWriter) {
	conv cnv;

	env->GetByteArrayRegion(jptrWriter, 0, sizeof(void *), (jbyte *)cnv.array);

	cvReleaseVideoWriter((CvVideoWriter **)&cnv.ptr);
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvWriteFrame
 * Signature: ([B[B)I
 */
JNIEXPORT jint JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvWriteFrame
(JNIEnv *env, jclass cls, jbyteArray jptrWriter, jbyteArray jptrIplImage) {
        conv cnv0, cnv1;

	env->GetByteArrayRegion(jptrWriter  , 0, sizeof(void *), (jbyte *)cnv0.array);
	env->GetByteArrayRegion(jptrIplImage, 0, sizeof(void *), (jbyte *)cnv1.array);

	int ret = cvWriteFrame((CvVideoWriter *)cnv0.ptr, (IplImage *)cnv1.ptr);
	return (jint)ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvCreateImage
 * Signature: (IIII)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvCreateImage
(JNIEnv *env, jclass cls, jint jwidth, jint jheight, jint jdepth, jint jchannels) {
	conv cnv;
	cnv.ptr =cvCreateImage(cvSize(jwidth, jheight), (int)jdepth, (int)jchannels);

	jbyteArray ret = env->NewByteArray(sizeof(void *));
	env->SetByteArrayRegion(ret, 0, sizeof(void *), (const signed char *)cnv.array);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvReleaseImage
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvReleaseImage
(JNIEnv *env, jclass cls, jbyteArray jptrIplImage) {
	conv cnv;

	env->GetByteArrayRegion(jptrIplImage, 0, sizeof(void *), (jbyte *)cnv.array);

	cvReleaseImage((IplImage **)&cnv.ptr);
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    decodeIplImage
 * Signature: ([B)[I
 */
JNIEXPORT jintArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_decodeIplImage
(JNIEnv *env, jclass cls, jbyteArray jptrIplImage) {
	conv cnv;

	env->GetByteArrayRegion(jptrIplImage, 0, sizeof(void *), (jbyte *)cnv.array);

	IplImage *p = (IplImage *)cnv.ptr;

	jintArray ret = env->NewIntArray(22);
	env->SetIntArrayRegion(ret, 0, 1, (const jint *)&p->nSize);
	env->SetIntArrayRegion(ret, 1, 1, (const jint *)&p->ID);
	env->SetIntArrayRegion(ret, 2, 1, (const jint *)&p->nChannels);
	env->SetIntArrayRegion(ret, 3, 1, (const jint *)&p->alphaChannel);
	env->SetIntArrayRegion(ret, 4, 1, (const jint *)&p->depth);
	env->SetIntArrayRegion(ret, 5, 1, (const jint *)&p->dataOrder);
	env->SetIntArrayRegion(ret, 6, 1, (const jint *)&p->origin);
	env->SetIntArrayRegion(ret, 7, 1, (const jint *)&p->align);
	env->SetIntArrayRegion(ret, 8, 1, (const jint *)&p->width);
	env->SetIntArrayRegion(ret, 9, 1, (const jint *)&p->height);
	env->SetIntArrayRegion(ret, 10, 1, (const jint *)&p->imageSize);
	env->SetIntArrayRegion(ret, 11, 1, (const jint *)&p->widthStep);

	int tmp;
	tmp = (0xff & (int)p->colorModel[3]);
	tmp = (0xff & (int)p->colorModel[2]) | (tmp << 8);
	tmp = (0xff & (int)p->colorModel[1]) | (tmp << 8);
	tmp = (0xff & (int)p->colorModel[0]) | (tmp << 8);
	env->SetIntArrayRegion(ret, 12, 1, (const jint *)&tmp);

	tmp = (0xff & (int)p->channelSeq[3]);
	tmp = (0xff & (int)p->channelSeq[2]) | (tmp << 8);
	tmp = (0xff & (int)p->channelSeq[1]) | (tmp << 8);
	tmp = (0xff & (int)p->channelSeq[0]) | (tmp << 8);
	env->SetIntArrayRegion(ret, 13, 1, (const jint *)&tmp);

	env->SetIntArrayRegion(ret, 14, 4, (const jint *)&p->BorderMode[0]);
	env->SetIntArrayRegion(ret, 18, 4, (const jint *)&p->BorderConst[0]);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    imageDataIplImage
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_imageDataIplImage
(JNIEnv *env, jclass cls, jbyteArray jptrIplImage) {
	conv cnv;

	env->GetByteArrayRegion(jptrIplImage, 0, sizeof(void *), (jbyte *)cnv.array);

	IplImage *img = (IplImage *)cnv.ptr;

	jbyteArray ret = env->NewByteArray(img->imageSize);
	env->SetByteArrayRegion(ret, 0, img->imageSize, (const signed char *)img->imageData);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    imageDataIplImageDirect
 * Signature: ([B)Ljava/nio/ByteBuffer;
 */
JNIEXPORT jobject JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_imageDataIplImageDirect
  (JNIEnv *env, jclass cls, jbyteArray jptrIplImage) {
	conv cnv;

	env->GetByteArrayRegion(jptrIplImage, 0, sizeof(void *), (jbyte *)cnv.array);

	IplImage *img = (IplImage *)cnv.ptr;

	jobject ret = env->NewDirectByteBuffer((void *)img->imageData, img->imageSize);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvCloneImage
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvCloneImage
(JNIEnv *env, jclass cls, jbyteArray jptrIplImage) {
	conv cnv;

	env->GetByteArrayRegion(jptrIplImage, 0, sizeof(void *), (jbyte *)cnv.array);

	cnv.ptr = cvCloneImage((const IplImage *)cnv.ptr);

	jbyteArray ret = env->NewByteArray(sizeof(void *));
	env->SetByteArrayRegion(ret, 0, sizeof(void *), (const signed char *)cnv.array);

	return ret;
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvtColor
 * Signature: ([B[BII)V
 */
JNIEXPORT void JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvtColor
  (JNIEnv *env, jclass cls, jbyteArray jptrInputArray, jbyteArray jptrOutputArray, jint code, jint dstCn) {
	conv cnv;

	env->GetByteArrayRegion(jptrInputArray, 0, sizeof(void *), (jbyte *)cnv.array);
    CvArr* src = cnv.ptr;

	env->GetByteArrayRegion(jptrOutputArray, 0, sizeof(void *), (jbyte *)cnv.array);
    CvArr* dst = cnv.ptr;

	cvCvtColor(src, dst, code);
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvtFlip
 * Signature: ([B[BI)V
 */
JNIEXPORT void JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvFlip
(JNIEnv *env, jclass cls, jbyteArray jptrInputArray, jbyteArray jptrOutputArray, jint mode) {
	conv cnv;

	env->GetByteArrayRegion(jptrInputArray, 0, sizeof(void *), (jbyte *)cnv.array);
    CvArr* src = cnv.ptr;

	env->GetByteArrayRegion(jptrOutputArray, 0, sizeof(void *), (jbyte *)cnv.array);
    CvArr* dst = cnv.ptr;

	cvFlip(src, dst, mode);
}

/*
 * Class:     org_naokishibata_ocvimgio_OpenCVJNI
 * Method:    cvVersion
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_ocvimgio_OpenCVJNI_cvVersion
(JNIEnv *env, jclass cls) {
  jbyteArray ret = env->NewByteArray(strlen(CV_VERSION_NUM)+1);
  env->SetByteArrayRegion(ret, 0, strlen(CV_VERSION_NUM)+1, (jbyte *)(CV_VERSION_NUM));

  return ret;
}

#ifdef __cplusplus
}
#endif
#endif
