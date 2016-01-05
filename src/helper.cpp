// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This software is in public domain. You can use and modify this code
// for any purpose without any obligation.


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <assert.h>

#ifdef _MSC_VER
#include <Windows.h>
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include "helper.h"

void exitf(int code, const char *mes, ...) {
  va_list ap;
  va_start(ap, mes);
  vfprintf(stderr, mes, ap);
  va_end(ap);
  fflush(stderr);
  exit(code);
}

char *readFileAsStr(const char *fn, int maxSize) {
  FILE *fp = fopen(fn, "r");
  if (fp == NULL) exitf(-1, "Couldn't open file %s\n", fn);

  long size;

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (size > maxSize) exitf(-1, "readFileAsStr : file too large (%d bytes)\n", size);

  char *buf = (char *)malloc(size+10);

  size = fread(buf, 1, size, fp);
  buf[size] = '\0';

  fclose(fp);

  return buf;
}

void String_trim(char *str) {
  char *dst = str, *src = str, *pterm = src;

  while(*src != '\0' && isspace(*src)) src++;

  for(;*src != '\0';src++) {
    *dst++ = *src;
    if (!isspace(*src)) pterm = src+1;
  }

  *pterm = '\0';
}

int64_t currentTimeMillis() {
#ifdef _MSC_VER
  struct _timeb timebuffer;
  _ftime64_s( &timebuffer );
  return timebuffer.time * (int64_t)1000 + timebuffer.millitm;
#else
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * (int64_t)1000 + tp.tv_usec / 1000;
#endif
}

void sleepMillis(int ms) {
#ifdef _MSC_VER
  static int initialized = 0;

  if (!initialized) {
    initialized = 1;
    timeBeginPeriod(1);
  }

  Sleep(ms);
#else
  usleep(ms * 1000);
#endif
}

#define MAGIC_ARRAYMAPNODE 0xf73130fa
#define MAGIC_ARRAYMAP 0x8693bd21

typedef struct ArrayMapNode {
  uint32_t magic;
  uint64_t key;
  void *value;
} ArrayMapNode;

typedef struct ArrayMap {
  uint32_t magic;
  ArrayMapNode *array;
  int size, capacity;
} ArrayMap;

ArrayMap *initArrayMap() {
  ArrayMap *thiz = (ArrayMap *)calloc(1, sizeof(ArrayMap));
  thiz->magic = MAGIC_ARRAYMAP;
  thiz->capacity = 8;
  thiz->array = (ArrayMapNode *)malloc(thiz->capacity * sizeof(ArrayMapNode));
  thiz->size = 0;
  return thiz;
}

void ArrayMap_dispose(ArrayMap *thiz) {
  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);

  for(int i=0;i<thiz->size;i++) {
    assert(thiz->array[i].magic == MAGIC_ARRAYMAPNODE);
    thiz->array[i].magic = 0;
  }

  free(thiz->array);
  thiz->magic = 0;
  free(thiz);
}

int ArrayMap_size(ArrayMap *thiz) {
  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);
  return thiz->size;
}

uint64_t *ArrayMap_keyArray(ArrayMap *thiz) {
  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);
  uint64_t *a = (uint64_t *)malloc(sizeof(uint64_t) * thiz->size);
  for(int i=0;i<thiz->size;i++) {
    assert(thiz->array[i].magic == MAGIC_ARRAYMAPNODE);
    a[i] = thiz->array[i].key;
  }
  return a;
}

void **ArrayMap_valueArray(ArrayMap *thiz) {
  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);
  void **a = (void **)malloc(sizeof(void *) * thiz->size);
  for(int i=0;i<thiz->size;i++) {
    assert(thiz->array[i].magic == MAGIC_ARRAYMAPNODE);
    a[i] = thiz->array[i].value;
  }
  return a;
}

uint64_t ArrayMap_getKey(ArrayMap *thiz, int idx) {
  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);
  assert(0 <= idx && idx < thiz->size);
  assert(thiz->array[idx].magic == MAGIC_ARRAYMAPNODE);
  return thiz->array[idx].key;
}

void *ArrayMap_getValue(ArrayMap *thiz, int idx) {
  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);
  assert(0 <= idx && idx < thiz->size);
  assert(thiz->array[idx].magic == MAGIC_ARRAYMAPNODE);
  return thiz->array[idx].value;
}

void *ArrayMap_remove(ArrayMap *thiz, uint64_t key) {
  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);

  for(int i=0;i<thiz->size;i++) {
    assert(thiz->array[i].magic == MAGIC_ARRAYMAPNODE);
    if (thiz->array[i].key == key) {
      void *old = thiz->array[i].value;
      thiz->array[i].value = thiz->array[thiz->size-1].value;
      thiz->array[thiz->size-1].magic = 0;
      thiz->size--;
      return old;
    }
  }

  return NULL;
}

void *ArrayMap_put(ArrayMap *thiz, uint64_t key, void *value) {
  if (value == NULL) return ArrayMap_remove(thiz, key);

  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);

  for(int i=0;i<thiz->size;i++) {
    assert(thiz->array[i].magic == MAGIC_ARRAYMAPNODE);
    if (thiz->array[i].key == key) {
      void *old = thiz->array[i].value;
      thiz->array[i].value = value;
      return old;
    }
  }

  if (thiz->size >= thiz->capacity) {
    thiz->capacity *= 2;
    thiz->array = (ArrayMapNode *)realloc(thiz->array, thiz->capacity * sizeof(ArrayMapNode));
  }

  ArrayMapNode *n = &(thiz->array[thiz->size++]);
  n->magic = MAGIC_ARRAYMAPNODE;
  n->key = key;
  n->value = value;

  return NULL;
}

void *ArrayMap_get(ArrayMap *thiz, uint64_t key) {
  assert(thiz != NULL && thiz->magic == MAGIC_ARRAYMAP);

  for(int i=0;i<thiz->size;i++) {
    assert(thiz->array[i].magic == MAGIC_ARRAYMAPNODE);
    if (thiz->array[i].key == key) {
      return thiz->array[i].value;
    }
  }

  return NULL;
}
