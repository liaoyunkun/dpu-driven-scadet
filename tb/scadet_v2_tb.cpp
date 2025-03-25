#include "../hls/scadet_v2.h"
#include <iostream>
#include <fstream>
#include <algorithm>

#define CUR_ADDR 0x00007f63aed71010
#define REMOTE_ADDR 0x00007f629ed71010
#define MEM_POOL_SIZE_2 0x0000000010000000

// #define LINE_BITS 15
#define LINE_BITS 12
#define SET_BITS 10
#define SET_NUM 1024
#define SET_MASK ((1 << SET_BITS) - 1)
#define CACHE_ASSOC 128
#define TRACE_CNT (CACHE_ASSOC + 1)


template <typename T>
T* aligned_alloc(std::size_t num) {
     void* ptr = NULL;
 
     if (posix_memalign(&ptr, 4096, num * sizeof(T))) throw std::bad_alloc();
     // ptr = (void*)malloc(num * sizeof(T));
     return reinterpret_cast<T*>(ptr);
}

uint32_t va2setId(uint64_t virtAddr) {
	return (virtAddr >> LINE_BITS) & SET_MASK;
}

uint64_t va2tagId(uint64_t virtAddr) {
	return virtAddr >> (SET_BITS + LINE_BITS);
}

int main() {
    BUS_TYPE* tagMem = aligned_alloc<BUS_TYPE>(TOTAL_SETS * TAG_BLK_NUM);
    if (tagMem == nullptr) {
        std::cerr << "Fail to allocate memory for tagMem" << std::endl;
    }
    for (size_t i = 0; i < TOTAL_SETS * TAG_BLK_NUM; i++) {
        tagMem[i] = 0;
    }
    BUS_TYPE* cntMem = aligned_alloc<BUS_TYPE>(TOTAL_SETS * CNT_BLK_NUM);
    if (cntMem == nullptr) {
        std::cerr << "Fail to allocate memory for cntMem" << std::endl;
    }
    for (size_t i = 0; i < TOTAL_SETS * CNT_BLK_NUM; i++) {
        cntMem[i] = 0;
    }

    // generate eviction set
    uint64_t cur = CUR_ADDR;
    uint64_t evict_set_addr[SET_NUM][CACHE_ASSOC];
    uint32_t count_tag[SET_NUM];
    memset(count_tag, 0, sizeof(uint32_t)*SET_NUM);
    for (int i = 0; i < SET_NUM; i++) {
        for (int j = 0; j < CACHE_ASSOC; j++) {
            uint64_t addr = cur;
            uint32_t setId = va2setId(addr);
            evict_set_addr[setId][count_tag[setId]] = addr;
            count_tag[setId]++;
            cur += (1 << LINE_BITS);
        }
    }
    // load timestamp file
    uint32_t ts_array[CACHE_ASSOC];
    // open timestamp file
    std::ifstream tsfp("./timestamp.txt");
    if (!tsfp.is_open()) {
        std::cerr << "Error opening the file!" << std::endl;
        return 1;
    }
    uint32_t rd_line_cnt = 0;
    uint32_t rd_val;
    while (rd_line_cnt < CACHE_ASSOC) {
        /* code */
        tsfp >> rd_val;
        ts_array[rd_line_cnt] = rd_val / PERIOD;
        rd_line_cnt++;
    }
    tsfp.close();
    
    // generate victim address
    uint64_t victimAddr = REMOTE_ADDR + (rand() % (MEM_POOL_SIZE_2));
    std::cout << "VictimAddr = " << std::hex << victimAddr << std::endl;
    uint32_t secId = va2setId(victimAddr);

    // generate trace set
    meta_in_t trace_set [TRACE_CNT];
    TS_TYPE ts_temp;
    TAG_TYPE tag_temp;
    SET_TYPE set_temp;
    QPN_TYPE dqpn_temp;
    QPN_TYPE attacker_dqpn = 20;
    QPN_TYPE victim_dqpn = 21;
    TS_TYPE cur_time = 0;
    // 1. fill trace_set with the attacker evict set
    for (size_t i = 0; i < CACHE_ASSOC; i++) {
        ts_temp = cur_time;
        cur_time += ts_array[i];
        tag_temp = va2tagId(evict_set_addr[secId][i]);
        set_temp = secId;
        dqpn_temp = attacker_dqpn;
        trace_set[i].range(QPN_LEN-1, 0) = dqpn_temp;
        trace_set[i].range(QPN_LEN+SET_LEN-1, QPN_LEN) = set_temp;
        trace_set[i].range(QPN_LEN+SET_LEN+TAG_LEN-1, QPN_LEN+SET_LEN) = tag_temp;
        trace_set[i].range(QPN_LEN+SET_LEN+TAG_LEN+TS_LEN-1, QPN_LEN+SET_LEN+TAG_LEN) = ts_temp;
    }
    // cur_time += (INTRA_GROUP_TH / 2);
    // // 2. fill trace_set with the victim access 
    // ts_temp = cur_time;
    // tag_temp = va2tagId(victimAddr);
    // set_temp = secId;
    // dqpn_temp = victim_dqpn;
    // trace_set[CACHE_ASSOC].range(QPN_LEN-1, 0) = dqpn_temp;
    // trace_set[CACHE_ASSOC].range(QPN_LEN+SET_LEN-1, QPN_LEN) = set_temp;
    // trace_set[CACHE_ASSOC].range(QPN_LEN+SET_LEN+TAG_LEN-1, QPN_LEN+SET_LEN) = tag_temp;
    // trace_set[CACHE_ASSOC].range(QPN_LEN+SET_LEN+TAG_LEN+TS_LEN-1, QPN_LEN+SET_LEN+TAG_LEN) = ts_temp;
    // cur_time += (INTRA_GROUP_TH / 2);
    // 3. fill trace_set with the attacker load access
    cur_time += INTRA_GROUP_TH;
    ts_temp = cur_time;
    cur_time += ts_array[1];
    tag_temp = va2tagId(victimAddr);
    set_temp = secId;
    dqpn_temp = attacker_dqpn;
    trace_set[CACHE_ASSOC].range(QPN_LEN-1, 0) = dqpn_temp;
    trace_set[CACHE_ASSOC].range(QPN_LEN+SET_LEN-1, QPN_LEN) = set_temp;
    trace_set[CACHE_ASSOC].range(QPN_LEN+SET_LEN+TAG_LEN-1, QPN_LEN+SET_LEN) = tag_temp;
    trace_set[CACHE_ASSOC].range(QPN_LEN+SET_LEN+TAG_LEN+TS_LEN-1, QPN_LEN+SET_LEN+TAG_LEN) = ts_temp;
    meta_in_stream inStream;
    meta_out_stream outStream;
    std::cout << "secId - " << std::dec << secId << std::endl;
    // generate inStream
    for (size_t i = 0; i < TRACE_CNT; i++) {
        inStream.write(trace_set[i]); 
        std::cout << "write a new tsk " << std::dec << i << std::endl;   
    }
    meta_out_t res;
    bool notifited = false;
    // while (true && !notifited) {
    //     res = outStream.read();
    //     if (res == attacker_dqpn) {
    //         std::cout << res << " is attacker!" << std::endl;
    //         notifited = true;
    //     }
    // }

    scadet_v2(inStream, outStream);

    std::cout << "wait" << std::endl;
    while (outStream.empty()) {
        /* code */
        std::cout << "wait" << std::endl;
    }

    res = outStream.read();
    if (res == attacker_dqpn) {
        std::cout << res << " is attacker!" << std::endl;
        notifited = true;
    }

    std::free(tagMem);
    std::free(cntMem);
    return 0;
}