//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_OBJMASTERLOG_H
#define NFTSIMPLEPROJ_OBJMASTERLOG_H

#include <cstdio>
#define  OMLOG_TAG    "ObjMaster"
// Change these according to the target architecture for the ObjMaster library
// We just use stderr as default
#define  OMLOGD(...)  fprintf(stdout, __VA_ARGS__)
#define  OMLOGI(...)  fprintf(stdout, __VA_ARGS__)
#define  OMLOGW(...)  fprintf(stderr, __VA_ARGS__)
#define  OMLOGE(...)  fprintf(stderr, __VA_ARGS__)

#endif //NFTSIMPLEPROJ_OBJMASTERLOG_H
