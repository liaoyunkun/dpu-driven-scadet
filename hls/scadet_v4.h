#ifndef __SCADET_H__
#define __SCADET_H__

#define GMP 0
#if GMP == 1
#include <gmp.h>
#define __gmp_const const
#endif

#include "ap_int.h"
#include "hls_stream.h"

#define TS_LEN 20
#define TAG_LEN 42  // 64 - (10 + 12) = 42
#define QPN_LEN 24
#define DRAM_ADDR_LEN 32
#define SET_LEN 10
#define DRAM_BUS_LEN 512
#define DRAM_BUS_LEN_BYTE 64
#define TAG_ELE_LEN (TAG_LEN + QPN_LEN)  // 66
#define CNT_ELE_LEN (QPN_LEN + 8)   // 32
#define TOTAL_SETS 1024
#define SET_BITMAP_LEN 512
#define SET_BITMAP_SHIFT 9
#define SET_BITMAP_NUM (TOTAL_SETS / SET_BITMAP_LEN) 
#define WAYS_NUM 128
#define TAG_ELE_NUM (DRAM_BUS_LEN / TAG_ELE_LEN)  // 7
#define CNT_ELE_NUM (DRAM_BUS_LEN / CNT_ELE_LEN)  // 16
#define TAG_BLK_NUM ((WAYS_NUM / TAG_ELE_NUM) + 1)  // 19
#define CNT_BLK_NUM (WAYS_NUM / CNT_ELE_NUM)  // 8
#define TAG_MEM_SIZE  (DRAM_BUS_LEN_BYTE * TAG_BLK_NUM)  // 1216 B 
#define CNT_MEM_SIZE   (DRAM_BUS_LEN_BYTE * CNT_BLK_NUM)  // 512 B
#define MEM_STRIDE_SIZE  (TAG_MEM_SIZE + CNT_MEM_SIZE)  // 1728 B 
#define MEM_STRIDE_IN_BLK  (MEM_STRIDE_SIZE / DRAM_BUS_LEN_BYTE)  // 27 
#define PERIOD 4  // ns
#define INTRA_GROUP_TH (10000 / PERIOD)

typedef ap_uint<1> uint1;
typedef ap_uint<8> uint8;
typedef ap_uint<TS_LEN> TS_TYPE;
typedef ap_uint<TAG_LEN> TAG_TYPE;
typedef ap_uint<QPN_LEN> QPN_TYPE;
typedef ap_uint<SET_LEN> SET_TYPE;
typedef ap_uint<DRAM_ADDR_LEN> ADDR_TYPE;
typedef ap_uint<DRAM_BUS_LEN> BUS_TYPE;
typedef ap_uint<SET_BITMAP_LEN> BITMAP_TYPE; 
typedef ap_uint<TS_LEN + TAG_LEN + SET_LEN + QPN_LEN> meta_in_t;
typedef ap_uint<QPN_LEN> meta_out_t; 
typedef ap_uint<TAG_ELE_LEN> TAG_ELE_T;
typedef ap_uint<CNT_ELE_LEN> CNT_ELE_T;

typedef hls::stream<meta_in_t> meta_in_stream;
typedef hls::stream<meta_out_t> meta_out_stream;


#define CACHE_TAG_WIDTH SET_LEN
#define CACHE_WAY       16
#define CACHE_IDX_W     4

#define MULTI_TRAN_MODE 1
#define TRACE_MODE 5

void scadet_v4 (
    meta_in_stream& inStream,
    meta_out_stream& outStream,
    BUS_TYPE* tagPort,
    BUS_TYPE* cntPort 
);


#endif