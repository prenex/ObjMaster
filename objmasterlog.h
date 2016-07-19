//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_OBJMASTERLOG_H
#define NFTSIMPLEPROJ_OBJMASTERLOG_H

#include <android/log.h>
#define  OMLOG_TAG    "ObjMaster"
// Change these according to the target architecture for the ObjMaster library
// REM.: the usleep operations are necessary because android will just drop your log messages
// when they are coming faster than the system can serve it. The buffer is 64k in the kernel
// and in c++ you can easily outperform your kernel serving out the logcat.
// In most cases  we only log when "#define DEBUG 1" is added and in those cases we should not
// lose the data...
#ifdef DEBUG
#include <unistd.h>
#define  OMLOGD(...)  usleep(500); __android_log_print(ANDROID_LOG_DEBUG,OMLOG_TAG,__VA_ARGS__)
#define  OMLOGI(...)  usleep(550); __android_log_print(ANDROID_LOG_INFO,OMLOG_TAG,__VA_ARGS__)
#define  OMLOGW(...)  usleep(550); __android_log_print(ANDROID_LOG_WARN,OMLOG_TAG,__VA_ARGS__)
#define  OMLOGE(...)  usleep(500); __android_log_print(ANDROID_LOG_ERROR,OMLOG_TAG,__VA_ARGS__)
#endif
// In production we should not wait at all of course...
#ifndef DEBUG
#define  OMLOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,OMLOG_TAG,__VA_ARGS__)
#define  OMLOGI(...)  __android_log_print(ANDROID_LOG_INFO,OMLOG_TAG,__VA_ARGS__)
#define  OMLOGW(...)  __android_log_print(ANDROID_LOG_WARN,OMLOG_TAG,__VA_ARGS__)
#define  OMLOGE(...)  __android_log_print(ANDROID_LOG_ERROR,OMLOG_TAG,__VA_ARGS__)
#endif

#endif //NFTSIMPLEPROJ_OBJMASTERLOG_H
