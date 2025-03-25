// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef HLS_PRINT_H
#define HLS_PRINT_H
#ifndef __SYNTHESIS__
#include <stdio.h>
using namespace std;
#endif

namespace hls {
#ifdef __SYNTHESIS__
// Required because blackbox does not support double
typedef union {
    double d;
    unsigned long long l;
} convert;

void print(const char* fmt) { 
    // FIXME: replace with intrinsic call and first argument index into format table
    _ssdm_op_PrintNone(fmt); 
}
void print(const char* fmt, int v) { 
    // FIXME: replace with intrinsic call and first argument index into format table
    _ssdm_op_PrintInt(fmt, v); 
}
void print(const char* fmt, unsigned v) { 
    // FIXME: replace with intrinsic call and first argument index into format table
    _ssdm_op_PrintInt(fmt, v); 
}
void print(const char* fmt, double v) { 
    convert tmp;
    tmp.d = v;
    // FIXME: replace with intrinsic call and first argument index into format table
    _ssdm_op_PrintDouble(fmt, tmp.l);
}
#else
void print(const char* fmt) { 
    printf("HLS_PRINT: ");
    printf(fmt); 
}
void print(const char* fmt, int v) { 
    printf("HLS_PRINT: ");
    printf(fmt, v); 
}
void print(const char* fmt, unsigned v) { 
    printf("HLS_PRINT: ");
    printf(fmt, v); 
}
void print(const char* fmt, double v) { 
    printf("HLS_PRINT: ");
    printf(fmt, v); 
}
#endif
}
#endif
