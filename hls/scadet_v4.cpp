#include "scadet_v4.h"

void scadet_v4 (
    meta_in_stream& inStream,
    meta_out_stream& outStream,
    BUS_TYPE* tagPort,
    BUS_TYPE* cntPort 
)
{
    #pragma HLS stream variable=inStream depth=129 type=fifo
    #pragma HLS stream variable=outStream depth=1 type=fifo

    #pragma HLS interface m_axi offset = off latency = 18 num_write_outstanding = 1 num_read_outstanding = \
        1 max_write_burst_length = 32 max_read_burst_length = 32 bundle = gmem0 port = tagPort  depth = 19456
    #pragma HLS interface m_axi offset = off latency = 18 num_write_outstanding = 1 num_read_outstanding = \
        1 max_write_burst_length = 32 max_read_burst_length = 32 bundle = gmem1 port = cntPort  depth = 8192

    static BUS_TYPE 				    tag_dataArray[CACHE_WAY][TAG_BLK_NUM] = {0};
    static BUS_TYPE                     cnt_dataArray[CACHE_WAY][CNT_BLK_NUM] = {0};

    static TS_TYPE prev_ts_array [TOTAL_SETS] = {0};
    static uint8 uni_tags_array [TOTAL_SETS] = {0};
    static uint8 uni_dqpns_array [TOTAL_SETS] = {0};
   
    static BITMAP_TYPE sets_bitmap [2] = {0};

    static meta_in_t new_tsk = 0;
    static TS_TYPE new_ts = 0;
    static TS_TYPE prev_ts = 0;
    static uint8 uni_tags = 0;
    static uint8 tag_loop_iter_cnt = 0;
    static uint8 uni_dqpns = 0;
    static uint8 cnt_loop_iter_cnt = 0;

    static uint8 evict_uni_tags = 0;
    static uint8 evict_tag_loop_iter_cnt = 0;
    static uint8 evict_uni_dqpns = 0;
    static uint8 evict_cnt_loop_iter_cnt = 0;

    static QPN_TYPE new_dqpn = 0;
    static SET_TYPE new_set = 0;
    static SET_TYPE evict_set = 0;
    static TAG_TYPE new_tag = 0;
    static uint1 new_bm_idx = 0;
    static uint1 hit_set = 0;
    static ap_uint<SET_BITMAP_SHIFT> new_bm_oft = 0;
    static BITMAP_TYPE cur_bitmap = 0;
    static BUS_TYPE tags_beat = 0;
    static BUS_TYPE cnt_beat = 0;
    static TAG_ELE_T tags_ele = 0;
    static CNT_ELE_T cnt_ele = 0;
    static unsigned int tags_addr_blk = 0;
    static unsigned int cnt_addr_blk = 0;
    static unsigned int evict_tags_addr_blk = 0;
    static unsigned int evict_cnt_addr_blk = 0;
    static TS_TYPE ts_diff = 0;
    static bool tag_hit = 0;
    static bool dqpn_hit = 0;
    static uint8 evict_idx = 0;
    static uint8 evict_idy = 0;
    static uint8 evict_cnt = 0;
    static QPN_TYPE evict_dqpn = 0;
    static uint8 tag_insert_idx = 0;
    static uint8 tag_insert_idy = 0;
    static uint8 cnt_insert_idx = 0;
    static uint8 cnt_insert_idy = 0;
    static uint8 cnt_insert_val = 0;
    static bool cnt_insert_find = 0;
    static TAG_ELE_T tags_insert_ele = 0;
    static CNT_ELE_T cnt_insert_ele = 0;
    static BUS_TYPE tags_insert_line = 0;
    static BUS_TYPE cnt_insert_line = 0;
    static bool find_evict_tag = 0;
    static TAG_ELE_T tags_evict_ele = 0;
    static BUS_TYPE tags_evict_line = 0;
    static uint8 tags_evict_idx = 0;
    static uint8 tags_evict_idy = 0;
    static CNT_ELE_T cnt_evict_ele = 0;
    static BUS_TYPE cnt_evict_line = 0;
    static uint8 cnt_evict_idx = 0;
    static uint8 cnt_evict_idy = 0;
    static uint8 dqpn_hit_idx = 0;
    static uint8 dqpn_hit_idy = 0;
    static uint8 dqpn_hit_cnt = 0;
    static uint8 last_cnt_ele_idx = 0;
    static uint8 last_cnt_ele_idy = 0;
    static CNT_ELE_T last_cnt_ele = 0;
    
    static uint8 cnt_w_idx = 0;

    static BUS_TYPE tags_line = 0;
    static TAG_ELE_T tag_ele_temp = 0;
    static QPN_TYPE tag_ele_dqpn_temp = 0;
    static TAG_TYPE tag_ele_tag_temp = 0;
    static BUS_TYPE cnt_line = 0;
    static CNT_ELE_T cnt_ele_temp = 0;
    static QPN_TYPE cnt_ele_dqpn_temp = 0;
    static uint8 cnt_ele_cnt_temp = 0;
    static meta_out_t dqpn_attacker_ele = 0;
    static unsigned int tsk_cnt = 0;

    //*******Cache*******//
    static ap_uint<CACHE_WAY> 			validArray = 0;
	static ap_uint<CACHE_TAG_WIDTH> 	tagArray[CACHE_WAY] = {0};
    #pragma HLS ARRAY_PARTITION variable=tagArray complete dim=0
	static ap_uint<CACHE_WAY> 			mruArray = 0;
    static ap_uint<CACHE_WAY>           tag_dirt = 0;
    static ap_uint<CACHE_WAY>           cnt_dirt = 0;

    static ap_uint<CACHE_WAY> isHit = 0;
    static ap_uint<CACHE_IDX_W> cache_evict_idx = 0;
    static bool isEvict;
    bool find_attacker = false;

    Read_Task_Loop: 
    #if (MULTI_TRAN_MODE == 0)
    while (true && !find_attacker)
    {
        if (!inStream.empty()) {
            new_tsk = inStream.read();
            #ifndef __SYNTHESIS__
            std::cout << "read a new tsk " << tsk_cnt << std::endl;
            #endif
            tsk_cnt++;
        } else {
            break;
        }
    #else
    {   
        new_tsk = inStream.read();
        #ifndef __SYNTHESIS__
        std::cout << "read a new tsk " << tsk_cnt << std::endl;
        #endif
        tsk_cnt++;
    #endif
        // note: assignment of new_dqpn, ..., new_ts, cnt_ele should execute in parallel in verilog 
        new_dqpn = new_tsk.range(QPN_LEN-1, 0);
        new_set = new_tsk.range(QPN_LEN+SET_LEN-1, QPN_LEN);
        new_tag = new_tsk.range(QPN_LEN+SET_LEN+TAG_LEN-1, QPN_LEN+SET_LEN);
        new_ts = new_tsk.range(QPN_LEN+SET_LEN+TAG_LEN+TS_LEN-1, QPN_LEN+SET_LEN+TAG_LEN);
        new_bm_oft = new_set.range(SET_BITMAP_SHIFT-1, 0);
        new_bm_idx = new_set >> SET_BITMAP_SHIFT;
        tags_ele.range(QPN_LEN-1, 0) = new_dqpn;
        tags_ele.range(TAG_ELE_LEN-1, QPN_LEN) = new_tag;
        cnt_ele.range(8-1, 0) = 1;
        cnt_ele.range(CNT_ELE_LEN-1, 8) = new_dqpn;
        tags_addr_blk = new_set * TAG_BLK_NUM;
        cnt_addr_blk = new_set * CNT_BLK_NUM;

        prev_ts = prev_ts_array[new_set];
        uni_tags = uni_tags_array[new_set];
        tag_loop_iter_cnt = (uni_tags + TAG_ELE_NUM - 1) / TAG_ELE_NUM;
        
        uni_dqpns = uni_dqpns_array[new_set];
        cnt_loop_iter_cnt = (uni_dqpns + CNT_ELE_NUM - 1) / CNT_ELE_NUM;
        
        cur_bitmap = sets_bitmap[new_bm_idx];
        hit_set = cur_bitmap[new_bm_oft];
        tag_hit = false;
        dqpn_hit = false;
        evict_idx = 0;
        #ifndef __SYNTHESIS__
        // std::cout << "uni_tags=" << uni_tags << ", tag_loop_iter_cnt=" << tag_loop_iter_cnt << std::endl;
        // std::cout << "new_bm_idx=" << new_bm_idx << ", new_bm_oft=" << new_bm_oft << std::endl;
        // std::cout << "hit_set=" << hit_set << std::endl;
        #endif
        
        Flow_Cache_Region: {
        isHit = 0;
        evict_set = 0;
        cache_evict_idx = 0;
        // Check Flow Cache
        for (unsigned int i = 0; i < CACHE_WAY; i++) {
            #pragma HLS unroll
            isHit[i] = validArray[i] && (tagArray[i] == new_set);
        }

        if (isHit.or_reduce()) {
            // hit
            #ifndef __SYNTHESIS__
            // std::cout << "Flow Cache Hit" << std::endl;
            #endif
            for (unsigned int i = 0; i < CACHE_WAY; i++) {
                #pragma HLS unroll
                if (isHit[i] == 1) {
                    cache_evict_idx = i;
                    break;
                } 
            }
        } else {
            // miss
            #ifndef __SYNTHESIS__
            // std::cout << "Flow Cache Miss" << std::endl;
            #endif
            // evict a line if cache full, else load
            if (validArray.and_reduce() == false) {
                // cache is not full
                for (unsigned int i = 0; i < CACHE_WAY; i++) {
                    #pragma HLS unroll
                    if (validArray[i] == 0) {
                        cache_evict_idx = i;
                        isEvict = false;
                    }
                }
            } else {
                // cache is full
                for (unsigned int i = 0; i < CACHE_WAY; i++) {
                    #pragma HLS unroll
                    if (mruArray[i] == 0) {
                        cache_evict_idx = i;
                        isEvict = true;
                        evict_set = tagArray[i];
                    }
                }
            }
            // 
            if (isEvict == true && (tag_dirt[cache_evict_idx] == 1)) {
                // write back tag
                evict_uni_tags = uni_tags_array[evict_set];
                evict_tag_loop_iter_cnt = (evict_uni_tags + TAG_ELE_NUM - 1) / TAG_ELE_NUM;
                evict_tags_addr_blk = evict_set * TAG_BLK_NUM;
                Evict_Tag_List_Loop: 
                for (unsigned int i = 0; i < evict_tag_loop_iter_cnt; i++) {
                    #pragma HLS pipeline II=1
                    #pragma HLS loop_flatten off
                    // should in burst transfer
                    #ifndef __SYNTHESIS__
                    // std::cout << "Write tagPort[" << evict_tags_addr_blk+i << "]" << std::endl;
                    #endif
                    tagPort[evict_tags_addr_blk+i] = tag_dataArray[cache_evict_idx][i];
                }
            }
            if (isEvict == true && (cnt_dirt[cache_evict_idx] == 1)) {
                // write back cnt
                evict_uni_dqpns = uni_dqpns_array[evict_set];
                evict_cnt_loop_iter_cnt = (evict_uni_dqpns + CNT_ELE_NUM - 1) / CNT_ELE_NUM;
                evict_cnt_addr_blk = evict_set * CNT_BLK_NUM;
                for (unsigned int i = 0; i < evict_cnt_loop_iter_cnt; i++) {
                    #pragma HLS pipeline II=1
                    #pragma HLS loop_flatten off
                    #ifndef __SYNTHESIS__
                    // std::cout << "Write cntPort[" << evict_cnt_addr_blk+i << "]" << std::endl;
                    #endif
                    cntPort[evict_cnt_addr_blk+i] = cnt_dataArray[cache_evict_idx][i];
                }
            }
            // load from dram
            Load_Tag_List_Loop: 
            for (unsigned int i = 0; i < tag_loop_iter_cnt; i++) {
                #pragma HLS pipeline II=1
                #pragma HLS loop_flatten off
                // should in burst transfer
                #ifndef __SYNTHESIS__
                std::cout << "Read tagPort[" << tags_addr_blk+i << "]" << std::endl;
                #endif
                tag_dataArray[cache_evict_idx][i] = tagPort[tags_addr_blk+i];
            }
            Load_Cnt_List_Loop:
            for (unsigned int i = 0; i < cnt_loop_iter_cnt; i++) {
                #pragma HLS pipeline II=1
                #pragma HLS loop_flatten off
                #ifndef __SYNTHESIS__
                // std::cout << "Read cntPort[" << cnt_addr_blk+i << "]" << std::endl;
                #endif
                cnt_dataArray[cache_evict_idx][i] = cntPort[cnt_addr_blk+i];
            }            
            tag_dirt[cache_evict_idx] = 0;
            cnt_dirt[cache_evict_idx] = 0;
            validArray[cache_evict_idx] = 1;
            tagArray[cache_evict_idx] = new_set;
        }

        // update mru
        mruArray[cache_evict_idx] = 1;
        if (mruArray.and_reduce() == true) {
            for (unsigned int i = 0; i < CACHE_WAY; i++) {
                #pragma HLS unroll
                mruArray[i] = (i == cache_evict_idx);
            }
        }
        }// Flow Cache End  

        if (hit_set == 1) {
            // std::cout << "hit_set == 1" << std::endl;
            // calculate the timing difference
            Cal_Time_Diff: {
            if (new_ts < prev_ts) {
                ts_diff = prev_ts - new_ts;
            } else {
                ts_diff = new_ts - prev_ts;
            }
            }
            // std::cout << "ts_diff = " << ts_diff << std::endl;
            if (ts_diff < INTRA_GROUP_TH) {
                Search_Tag_List_Loop_1: 
                for (unsigned int i = 0; i < tag_loop_iter_cnt && !tag_hit; i++) {
                    #pragma HLS loop_flatten off
                    // load a line
                    tags_line = tag_dataArray[cache_evict_idx][i];
                    Search_Tag_Line_Loop_1: 
                    // should be parallelized
                    for (unsigned int j = 0; j < TAG_ELE_NUM && ((i*TAG_ELE_NUM+j) < uni_tags); j++) {
                        #pragma HLS pipeline II=1
                        #pragma HLS loop_flatten off
                        #ifndef __SYNTHESIS__
                        // std::cout << "Search_Tag_Line_Loop_Iter_" << i << "_" << j << std::endl;
                        #endif
                        tag_ele_tag_temp = tags_line.range(((j+1) * TAG_ELE_LEN) -1, j * TAG_ELE_LEN + QPN_LEN);
                        tag_ele_dqpn_temp = tags_line.range(j * TAG_ELE_LEN + QPN_LEN - 1, j * TAG_ELE_LEN);
                        if (tag_ele_tag_temp == new_tag && tag_ele_dqpn_temp == new_dqpn) {
                            tag_hit = true;
                            break;
                        }
                    }
                }
                Search_Cnt_List_Loop_1:
                for (unsigned int i = 0; i < cnt_loop_iter_cnt && !dqpn_hit; i++) {
                    #pragma HLS loop_flatten off
                    cnt_line = cnt_dataArray[cache_evict_idx][i];
                    Search_Cnt_Line_Loop_For_Evict: 
                    for (unsigned int j = 0; j < CNT_ELE_NUM && ((i*CNT_ELE_NUM+j) < uni_dqpns); j++) {
                        #pragma HLS pipeline II=1
                        #pragma HLS loop_flatten off
                        #ifndef __SYNTHESIS__
                        // std::cout << "Search_Cnt_Line_Loop_For_Evict_Iter_" << i << "_" << j << std::endl;
                        #endif
                        cnt_ele_cnt_temp = cnt_line.range(j*CNT_ELE_LEN + 8-1, j*CNT_ELE_LEN);
                        cnt_ele_dqpn_temp = cnt_line.range(((j+1) * CNT_ELE_LEN)-1, j*CNT_ELE_LEN + 8);
                        if (cnt_ele_dqpn_temp == new_dqpn) {
                            cnt_insert_idx = i;
                            cnt_insert_idy = j;
                            cnt_insert_val = cnt_ele_cnt_temp + 1;
                            dqpn_hit = true;
                            break;
                        }
                    }
                }
                // std::cout << "tag_hit = " << tag_hit << std::endl;
                // std::cout << "dqpn_hit = " << dqpn_hit << std::endl;  
                if (!tag_hit && uni_tags == WAYS_NUM) {
                    // evict a tag
                    // find the dqpn with miminum counter
                    evict_cnt = WAYS_NUM;
                    Search_Min_Cnt_List_Loop_For_Evict: 
                    for (unsigned int i = 0; i < cnt_loop_iter_cnt; i++) {
                        // std::cout << "I am inside Search_Min_Cnt_List_Loop_For_Evict" << std::endl;
                        #pragma HLS loop_flatten off
                        cnt_line = cnt_dataArray[cache_evict_idx][i];
                        Search_Min_Cnt_Line_Loop_For_Evict: 
                        // shoud be parallelized
                        for (unsigned int j = 0; j < CNT_ELE_NUM && ((i*CNT_ELE_NUM+j) < uni_dqpns); j++) {
                            #pragma HLS pipeline II=1
                            #pragma HLS loop_flatten off
                            cnt_ele_cnt_temp = cnt_line.range(j*CNT_ELE_LEN + 8-1, j*CNT_ELE_LEN);
                            cnt_ele_dqpn_temp = cnt_line.range(((j+1) * CNT_ELE_LEN)-1, j*CNT_ELE_LEN + 8);
                            if (!(dqpn_hit && cnt_ele_dqpn_temp == new_dqpn) && cnt_ele_cnt_temp <= evict_cnt) {
                                evict_cnt = cnt_ele_cnt_temp;
                                evict_dqpn = cnt_ele_dqpn_temp;
                                evict_idx = i;
                                evict_idy = j;
                            }
                            if (dqpn_hit && cnt_ele_dqpn_temp == new_dqpn) {
                                dqpn_hit_idx = i;
                                dqpn_hit_idy = j;
                                dqpn_hit_cnt = cnt_ele_cnt_temp;
                            }
                        }
                    }
                    Evict_Write_Back_Cnt_List:{
                    // TODO:
                    // decrement the counter and remove the "zero" entry if necessary 
                    if (!dqpn_hit && evict_cnt == 1) {
                        // substitute the "zero" entry
                        if (evict_idy > 0) {
                            cnt_evict_line.range(evict_idy*CNT_ELE_LEN-1, 0) = cnt_dataArray[cache_evict_idx][evict_idx].range(evict_idy*CNT_ELE_LEN-1, 0);
                        }
                        cnt_evict_line.range((evict_idy+1)*CNT_ELE_LEN-1,evict_idy*CNT_ELE_LEN) = cnt_ele;
                        if (evict_idy < CNT_ELE_NUM - 1) {
                            cnt_evict_line.range(DRAM_BUS_LEN-1, (evict_idy+1)*CNT_ELE_LEN) = cnt_dataArray[cache_evict_idx][evict_idx].range(DRAM_BUS_LEN-1, (evict_idy+1)*CNT_ELE_LEN);
                        }
                        cnt_dataArray[cache_evict_idx][evict_idx] = cnt_evict_line;
                        cnt_dirt[cache_evict_idx] = 1;
                        uni_dqpns_array[new_set] = uni_dqpns; // unchanged
                    } else if (!dqpn_hit && evict_cnt > 1) {
                        // decrement and append, update two
                        // update evict element first
                        cnt_dataArray[cache_evict_idx][evict_idx].range(evict_idy*CNT_ELE_LEN+8-1, evict_idy*CNT_ELE_LEN) = evict_cnt - 1;
                        if (uni_dqpns == cnt_loop_iter_cnt * CNT_ELE_NUM) {
                            cnt_evict_idx = cnt_loop_iter_cnt;
                            cnt_evict_idy = 0;
                        } else {
                            cnt_evict_idx = cnt_loop_iter_cnt - 1;
                            cnt_evict_idy = uni_dqpns - ((cnt_loop_iter_cnt-1) * CNT_ELE_NUM);
                        }
                        cnt_dataArray[cache_evict_idx][cnt_evict_idx].range((cnt_evict_idy+1)*CNT_ELE_LEN-1, cnt_evict_idy*CNT_ELE_LEN) = cnt_ele;
                        cnt_dirt[cache_evict_idx] = 1;
                        uni_dqpns_array[new_set] = uni_dqpns + 1;
                    } else if (dqpn_hit && evict_cnt == 1) {
                        // remove and increment, complex update, do not delete, lazy?
                        cnt_dataArray[cache_evict_idx][dqpn_hit_idx].range(dqpn_hit_idy*CNT_ELE_LEN+8-1, dqpn_hit_idy*CNT_ELE_LEN) = dqpn_hit_cnt + 1;
                        // move the last element to the evicted position
                        last_cnt_ele_idx = cnt_loop_iter_cnt - 1;
                        last_cnt_ele_idy = uni_dqpns - ((cnt_loop_iter_cnt - 1)*CNT_ELE_NUM) - 1;
                        last_cnt_ele = cnt_dataArray[cache_evict_idx][last_cnt_ele_idx].range((last_cnt_ele_idy+1)*CNT_ELE_LEN-1, last_cnt_ele_idy*CNT_ELE_LEN);
                        cnt_dataArray[cache_evict_idx][evict_idx].range((evict_idy+1)*CNT_ELE_LEN-1, evict_idy*CNT_ELE_LEN) = last_cnt_ele;
                        cnt_dirt[cache_evict_idx] = 1;
                        uni_dqpns_array[new_set] = uni_dqpns - 1;
                    } else {
                        // decrement and increment, update two lines
                        cnt_dataArray[cache_evict_idx][evict_idx].range(evict_idy*CNT_ELE_LEN+8-1, evict_idy*CNT_ELE_LEN) = evict_cnt - 1;
                        cnt_dataArray[cache_evict_idx][dqpn_hit_idx].range(dqpn_hit_idy*CNT_ELE_LEN+8-1, dqpn_hit_idy*CNT_ELE_LEN) = dqpn_hit_cnt + 1;
                        cnt_dirt[cache_evict_idx] = 1;
                        uni_dqpns_array[new_set] = uni_dqpns; // unchanged
                    }
                    }
                    Evict_Write_Back_Tag_List:{
                    // find the tag that has the qpn and insert the new element
                    find_evict_tag = false;
                    Search_Tag_List_Loop_For_Evict: 
                    for (unsigned int i = 0; i < tag_loop_iter_cnt && !find_evict_tag; i++) {
                        #pragma HLS loop_flatten off
                        tags_line = tag_dataArray[cache_evict_idx][i];
                        Search_Tag_Line_Loop_For_Evict: 
                        for (unsigned int j = 0; j < TAG_ELE_NUM && ((i*TAG_ELE_NUM+j) < uni_tags) && !find_evict_tag; j++) {
                            #pragma HLS pipeline II=1
                            #pragma HLS loop_flatten off
                            tag_ele_tag_temp = tags_line.range(((j+1) * TAG_ELE_LEN) -1, j * TAG_ELE_LEN + QPN_LEN);
                            tag_ele_dqpn_temp = tags_line.range(j * TAG_ELE_LEN + QPN_LEN - 1, j * TAG_ELE_LEN);
                            if (tag_ele_dqpn_temp == evict_dqpn) {
                                // evict this tag slot
                                find_evict_tag = true;
                                tags_evict_idx = i;
                                tags_evict_idy = j;
                            }
                        }
                    }
                    tags_evict_line = tag_dataArray[cache_evict_idx][tags_evict_idx];
                    tags_evict_line.range((tags_evict_idy+1) * TAG_ELE_LEN-1, tags_evict_idy*TAG_ELE_LEN) = tags_ele;
                    tag_dataArray[cache_evict_idx][tags_evict_idx] = tags_evict_line;
                    tag_dirt[cache_evict_idx] = 1;
                    }
                    // uni_tags_array[new_set] = uni_tags + 1; 
                } else if (!tag_hit && uni_tags < WAYS_NUM) {
                    Normal_Write_Back_Cnt_List: {
                    // search cnt list and increment counter
                    cnt_insert_find = false;
                    if (dqpn_hit) {
                        // search 
                        // Search_Cnt_List_Loop_For_Evict: 
                        // for (unsigned int i = 0; i < cnt_loop_iter_cnt && !cnt_insert_find; i++) {
                        //     #pragma HLS loop_flatten off
                        //     cnt_line = cnt_dataArray[cache_evict_idx][i];
                        //     Search_Cnt_Line_Loop_For_Evict: 
                        //     for (unsigned int j = 0; j < CNT_ELE_NUM && ((i*CNT_ELE_NUM+j) < uni_dqpns) && !cnt_insert_find; j++) {
                        //         #pragma HLS pipeline II=1
                        //         #pragma HLS loop_flatten off
                        //         #ifndef __SYNTHESIS__
                        //         // std::cout << "Search_Cnt_Line_Loop_For_Evict_Iter_" << i << "_" << j << std::endl;
                        //         #endif
                        //         cnt_ele_cnt_temp = cnt_line.range(j*CNT_ELE_LEN + 8-1, j*CNT_ELE_LEN);
                        //         cnt_ele_dqpn_temp = cnt_line.range(((j+1) * CNT_ELE_LEN)-1, j*CNT_ELE_LEN + 8);
                        //         if (cnt_ele_dqpn_temp == new_dqpn) {
                        //             cnt_insert_idx = i;
                        //             cnt_insert_idy = j;
                        //             cnt_insert_val = cnt_ele_cnt_temp + 1;
                        //             cnt_insert_find = true;
                        //         }
                        //     }
                        // }
                    } else {
                        if (uni_dqpns == cnt_loop_iter_cnt * CNT_ELE_NUM) {
                            cnt_insert_idx = cnt_loop_iter_cnt;
                            cnt_insert_idy = 0;
                        } else {
                            cnt_insert_idx = cnt_loop_iter_cnt - 1;
                            cnt_insert_idy = uni_dqpns - ((cnt_loop_iter_cnt-1) * CNT_ELE_NUM);
                        }
                        cnt_insert_val = 1;
                    }
                    cnt_insert_ele.range(8-1, 0) = cnt_insert_val;
                    cnt_insert_ele.range(CNT_ELE_LEN-1, 8) = new_dqpn;
                    if (cnt_insert_idy == 0) {
                        cnt_insert_line.range(CNT_ELE_LEN-1, 0) = cnt_insert_ele;
                    } else {
                        cnt_insert_line.range(cnt_insert_idy*CNT_ELE_LEN-1, 0) = cnt_dataArray[cache_evict_idx][cnt_insert_idx].range(cnt_insert_idy*CNT_ELE_LEN-1, 0);
                        cnt_insert_line.range((cnt_insert_idy+1)*CNT_ELE_LEN-1, cnt_insert_idy*CNT_ELE_LEN) = cnt_insert_ele;
                    }
                    cnt_dataArray[cache_evict_idx][cnt_insert_idx] = cnt_insert_line;
                    cnt_dirt[cache_evict_idx] = 1;
                    }
                    Normal_Write_Back_Tag_List: {
                    if (uni_tags == tag_loop_iter_cnt * TAG_ELE_NUM) {
                        tags_line = tag_dataArray[cache_evict_idx][tag_loop_iter_cnt];
                        tag_insert_idx = tag_loop_iter_cnt;
                        tag_insert_idy = 0;
                    } else {
                        tags_line = tag_dataArray[cache_evict_idx][tag_loop_iter_cnt - 1];
                        tag_insert_idx = tag_loop_iter_cnt - 1;
                        tag_insert_idy = uni_tags - ((tag_loop_iter_cnt-1) * TAG_ELE_NUM);
                    }
                    
                    tags_insert_ele.range(QPN_LEN-1, 0) = new_dqpn;
                    tags_insert_ele.range(TAG_ELE_LEN-1, QPN_LEN) = new_tag;
                    #ifndef __SYNTHESIS__
                    // std::cout << "tag_insert_idx = " << tag_insert_idx << std::endl;
                    // std::cout << "tag_insert_idy = " << tag_insert_idy << std::endl;
                    #endif
                    if (tag_insert_idy == 0) {
                        tags_insert_line.range(TAG_ELE_LEN-1, 0) = tags_insert_ele;
                    } else {
                        tags_insert_line.range(tag_insert_idy*TAG_ELE_LEN-1, 0) = tag_dataArray[cache_evict_idx][tag_insert_idx].range(tag_insert_idy*TAG_ELE_LEN-1, 0);
                        tags_insert_line.range((tag_insert_idy+1)*TAG_ELE_LEN-1, tag_insert_idy*TAG_ELE_LEN) = tags_insert_ele;
                    }
                    tag_dataArray[cache_evict_idx][tag_insert_idx] = tags_insert_line;
                    tag_dirt[cache_evict_idx] = 1;
                    }
                    uni_tags_array[new_set] = uni_tags + 1;
                    if (!dqpn_hit) {
                        uni_dqpns_array[new_set] = uni_dqpns + 1;
                    }
                } else {
                    // tag is hit, update timestamp only
                }
            }else {
                // std::cout << "ts_diff >= INTRA_GROUP_TH, tag_loop_iter_cnt = " << tag_loop_iter_cnt << std::endl;
                // std::cout << "uni_tags = " << uni_tags << std::endl;
                if (uni_tags == WAYS_NUM) {
                    // clearly, there are only one dqpn left if there is a attacker.
                    // load the first line only
                    // cnt_dataArray[cache_evict_idx][0] = cnt_dataArray[cache_evict_idx][0];
                    dqpn_attacker_ele = cnt_dataArray[cache_evict_idx][0].range(CNT_ELE_LEN-1, 8);
                    // std::cout << "dqpn_attacker_ele: " << dqpn_attacker_ele << std::endl;
                    if (dqpn_attacker_ele == new_dqpn) {
                        // std::cout << "detect attacker with dqpn = " << dqpn_attacker_ele << std::endl;
                        outStream.write(dqpn_attacker_ele);
                    }
                    #ifndef __SYNTHESIS__
                    std::cout << "Find Attacker" << std::endl;
                    #endif
                    find_attacker = true;
                }
                // clear this ClusterGenPerSet
                sets_bitmap[new_bm_idx][new_bm_oft] = 0;
                #ifndef __SYNTHESIS__
                // std::cout << "clear sets_bitmap[" << new_bm_idx << "]" << "[" << new_bm_oft << "]" << std::endl;
                #endif
                // validArray[cache_evict_idx] = 0;
            }
        } else {
            // std::cout << "hit_set == 0" << std::endl;
            Init_Write_Back_Tag_List: {
            tags_beat.range(TAG_ELE_LEN-1, 0) = tags_ele;
            tag_dataArray[cache_evict_idx][0] = tags_beat;
            tag_dirt[cache_evict_idx] = 1;
            }
            Init_Write_Back_Cnt_List: {
            cnt_beat.range(CNT_ELE_LEN-1, 0) = cnt_ele;
            cnt_dataArray[cache_evict_idx][0] = cnt_beat;
            cnt_dirt[cache_evict_idx] = 1;
            }
            validArray[cache_evict_idx] = 1;
            sets_bitmap[new_bm_idx][new_bm_oft] = 1;
            uni_tags_array[new_set] = 1;
            uni_dqpns_array[new_set] = 1;
        }
        prev_ts_array[new_set] = new_ts;
    #ifndef __SYNTHESIS__
    // std::cout << "end check: hit_set=" << sets_bitmap[new_bm_idx][new_bm_oft] << std::endl;
    #endif
    }
}