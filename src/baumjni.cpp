#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jni.h>

#include <CL/cl.h>
#include "oclhelper.h"
#include "baum.h"

#ifdef __cplusplus
extern "C" {
#endif

union conv {
  void *ptr;
  signed char array[sizeof(void *)];
};

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_init
 * Signature: (III)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1init(JNIEnv *env, jclass cls, jint did, jint iw, jint ih) {
  conv cnv;
  cnv.ptr = baum_init(did, iw, ih);

  jbyteArray ret = env->NewByteArray(sizeof(void *));
  env->SetByteArrayRegion(ret, 0, sizeof(void *), (const signed char *)cnv.array);

  return ret;
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_dispose
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1dispose(JNIEnv *env, jclass cls, jbyteArray thiz) {
  conv cnv;

  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  baum_dispose((baum_t *)cnv.ptr);
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_enqueueTask
 * Signature: ([BLjava/nio/ByteBuffer;JLjava/nio/ByteBuffer;III)I
 */
JNIEXPORT jint JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1enqueueTask(JNIEnv *env, jclass cls, jbyteArray thiz, jobject bbResultPtr, jlong bufSize, jobject bbImgPtr, jint iw, jint ih, jint ws) {
  conv cnv;

  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  void *resultPtr = env->GetDirectBufferAddress(bbResultPtr);
  void *imgPtr    = env->GetDirectBufferAddress(bbImgPtr);

  return baum_enqueueTask((baum_t *)cnv.ptr, resultPtr, bufSize, imgPtr, iw, ih, ws);
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_poll
 * Signature: ([BI)I
 */
JNIEXPORT jint JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1poll(JNIEnv *env, jclass cls, jbyteArray thiz, jint waitFlag) {
  conv cnv;

  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  return baum_poll((baum_t *)cnv.ptr, waitFlag);
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_queueLen
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1queueLen(JNIEnv *env, jclass cls, jbyteArray thiz) {
  conv cnv;

  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  return baum_queueLen((baum_t *)cnv.ptr);
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_createPlan
 * Signature: ([B[BLjava/nio/ByteBuffer;JLjava/nio/ByteBuffer;III)V
 */
JNIEXPORT void JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1createPlan(JNIEnv *env, jclass cls, jbyteArray thiz, jbyteArray baPath, jobject bbResultPtr, jlong bufSize, jobject bbImgPtr, jint iw, jint ih, jint ws) {
  conv cnv;

  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  jsize baPathLen = env->GetArrayLength(baPath);
  const char *path = (const char *)calloc(1, baPathLen+10);
  env->GetByteArrayRegion(baPath, 0, baPathLen, (jbyte *)path);

  void *resultPtr = env->GetDirectBufferAddress(bbResultPtr);
  void *imgPtr    = env->GetDirectBufferAddress(bbImgPtr);

  baum_createPlan((baum_t *)cnv.ptr, path, resultPtr, bufSize, imgPtr, iw, ih, ws);

  free((void *)path);
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_loadPlan
 * Signature: ([B[B)I
 */
JNIEXPORT jint JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1loadPlan(JNIEnv *env, jclass cls, jbyteArray thiz, jbyteArray baPath) {
  conv cnv;
  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  jsize baPathLen = env->GetArrayLength(baPath);
  const char *path = (const char *)calloc(1, baPathLen+10);
  env->GetByteArrayRegion(baPath, 0, baPathLen, (jbyte *)path);

  jint ret = baum_loadPlan((baum_t *)cnv.ptr, path);
  free((void *)path);

  return ret;
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_getDeviceName
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1getDeviceName___3B(JNIEnv *env, jclass cls, jbyteArray thiz) {
  conv cnv;

  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  char *dn = baum_getDeviceName((baum_t *)cnv.ptr);

  jbyteArray ret = env->NewByteArray(strlen(dn)+1);
  env->SetByteArrayRegion(ret, 0, strlen((const char *)dn)+1, (jbyte *)dn);

  free(dn);

  return ret;
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_getDeviceName
 * Signature: (I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1getDeviceName__I(JNIEnv *env, jclass cls, jint did) {
  //cl_device_id devices[did+1];
  cl_device_id *devices = (cl_device_id *)calloc(did+1, sizeof(cl_device_id));
  int nd = simpleGetDevices(devices, did+1);
  if (nd < did+1) { free(devices); return NULL; }

  char *dn = getDeviceName(devices[did]);

  jbyteArray ret = env->NewByteArray(strlen(dn)+1);
  env->SetByteArrayRegion(ret, 0, strlen((const char *)dn), (jbyte *)dn);

  free(dn);
  free(devices);
  
  return ret;
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_malloc
 * Signature: ([BJ)Ljava/nio/ByteBuffer;
 */
JNIEXPORT jobject JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1malloc(JNIEnv *env, jclass cls, jbyteArray thiz, jlong z) {
  conv cnv;

  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  return env->NewDirectByteBuffer(baum_malloc((baum_t *)cnv.ptr, z), z);
}

/*
 * Class:     org_naokishibata_baum_BaumJNI
 * Method:    baum_free
 * Signature: ([BLjava/nio/ByteBuffer;)V
 */
JNIEXPORT void JNICALL Java_org_naokishibata_baum_BaumJNI_baum_1free(JNIEnv *env, jclass cls, jbyteArray thiz, jobject bbMem) {
  conv cnv;

  env->GetByteArrayRegion(thiz, 0, sizeof(void *), (jbyte *)cnv.array);

  baum_free((baum_t *)cnv.ptr, env->GetDirectBufferAddress(bbMem));
}

#ifdef __cplusplus
}
#endif
