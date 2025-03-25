#include "scadet_v1.h"

void scadet_v1 (
    #if (USE_TAPA == 0)
    meta_in_stream& inStream,
    meta_out_stream& outStream,
    BUS_TYPE* tagPort,
    BUS_TYPE* cntPort 
    #else
    tapa::istream<meta_in_t>& inStream,
    tapa::ostream<meta_out_t>& outStream,
    tapa::mmap<BUS_TYPE> tagPort,
    tapa::mmap<BUS_TYPE> cntPort
    #endif
)
{
#if (USE_TAPA == 0)
    #pragma HLS stream variable=inStream depth=256 type=fifo
    #pragma HLS stream variable=outStream depth=2 type=fifo
    #pragma HLS interface m_axi offset = off latency = 18 num_write_outstanding = 1 num_read_outstanding = \
        1 max_write_burst_length = 2 max_read_burst_length = 32 bundle = gmem0 port = tagPort  depth = 19456
    #pragma HLS interface m_axi offset = off latency = 18 num_write_outstanding = 1 num_read_outstanding = \
        1 max_write_burst_length = 2 max_read_burst_length = 32 bundle = gmem1 port = cntPort  depth = 8192
#endif
    static BITMAP_TYPE sets_bitmap [SET_BITMAP_LEN];
    static TS_TYPE prev_ts_arr [TOTAL_SETS];
    static uint8 uni_tags_arr [TOTAL_SETS];
    static uint8 uni_dqpns_arr [TOTAL_SETS];

    static meta_in_t new_tsk;
    static TS_TYPE new_ts;
    static TS_TYPE prev_ts;
    static uint8 uni_tags;
    static uint8 tag_loop_iter_cnt;
    static uint8 uni_dqpns;
    static uint8 cnt_loop_iter_cnt;
    static QPN_TYPE new_dqpn;
    static SET_TYPE new_set;
    static TAG_TYPE new_tag;
    static uint1 new_bm_idx;
    static uint1 hit_set;
    static ap_uint<SET_BITMAP_SHIFT> new_bm_oft;
    static BITMAP_TYPE cur_bitmap;
    static BUS_TYPE tags_beat;
    static BUS_TYPE cnt_beat;
    static TAG_ELE_T tags_ele;
    static CNT_ELE_T cnt_ele;
    static unsigned int tags_addr_blk;
    static unsigned int cnt_addr_blk;
    static TS_TYPE ts_diff;
    static bool tag_hit;
    static bool dqpn_hit;
    static uint8 evict_idx;
    static uint8 evict_idy;
    static uint8 evict_cnt;
    static QPN_TYPE evict_dqpn;
    static uint8 tag_insert_idx;
    static uint8 tag_insert_idy;
    static uint8 cnt_insert_idx;
    static uint8 cnt_insert_idy;
    static uint8 cnt_insert_val;
    static bool cnt_insert_find;
    static TAG_ELE_T tags_insert_ele;
    static CNT_ELE_T cnt_insert_ele;
    static BUS_TYPE tags_insert_line;
    static BUS_TYPE cnt_insert_line;
    static bool find_evict_tag;
    static TAG_ELE_T tags_evict_ele;
    static BUS_TYPE tags_evict_line;
    static uint8 tags_evict_idx;
    static uint8 tags_evict_idy;
    static CNT_ELE_T cnt_evict_ele;
    static BUS_TYPE cnt_evict_line;
    static uint8 cnt_evict_idx;
    static uint8 cnt_evict_idy;
    static uint8 dqpn_hit_idx;
    static uint8 dqpn_hit_idy;
    static uint8 dqpn_hit_cnt;
    static uint8 last_cnt_ele_idx;
    static uint8 last_cnt_ele_idy;
    static CNT_ELE_T last_cnt_ele;
    
    static uint8 cnt_w_idx;

    static BUS_TYPE tags_list_buf [TAG_BLK_NUM];
    static BUS_TYPE tags_line;
    static TAG_ELE_T tag_ele_temp;
    static QPN_TYPE tag_ele_dqpn_temp;
    static TAG_TYPE tag_ele_tag_temp;
    static BUS_TYPE cnt_list_buf [CNT_BLK_NUM];
    static BUS_TYPE cnt_list_buf_temp [CNT_BLK_NUM];
    static BUS_TYPE cnt_line;
    static CNT_ELE_T cnt_ele_temp;
    static QPN_TYPE cnt_ele_dqpn_temp;
    static uint8 cnt_ele_cnt_temp;
    static meta_out_t dqpn_attacker_ele;

    static unsigned int tsk_cnt = 0;
    Read_Task_Loop: 
    #if (MULTI_TRAN_MODE == 0)
    while (true)
    {
        if (!inStream.empty()) {
            new_tsk = inStream.read();
            // std::cout << "read a new tsk " << tsk_cnt << std::endl;
            tsk_cnt++;
        } else {
            // std::cout << "empty" << std::endl;
            break;
        }
    #else
    {
        new_tsk = inStream.read();
        tsk_cnt++;
    #endif
        #ifndef __SYNTHESIS__
        std::cout << "read a new tsk " << tsk_cnt << std::endl;
        #endif
        // note: assignment of new_dqpn, ..., new_ts, cnt_ele should execute in parallel in verilog 
        new_dqpn = new_tsk.range(QPN_LEN-1, 0);
        new_set = new_tsk.range(QPN_LEN+SET_LEN-1, QPN_LEN);
        new_tag = new_tsk.range(QPN_LEN+SET_LEN+TAG_LEN-1, QPN_LEN+SET_LEN);
        new_ts = new_tsk.range(QPN_LEN+SET_LEN+TAG_LEN+TS_LEN-1, QPN_LEN+SET_LEN+TAG_LEN);
        // std::cout << "new_dqpn: " << new_dqpn << std::endl;
        // std::cout << "new_set: " << new_set << std::endl;
        // std::cout << "new_tag: " << new_tag << std::endl;
        // std::cout << "new_ts: " << new_ts << std::endl;
        new_bm_oft = new_set.range(SET_BITMAP_SHIFT-1, 0);
        new_bm_idx = new_set >> SET_BITMAP_SHIFT;
        tags_ele.range(QPN_LEN-1, 0) = new_dqpn;
        tags_ele.range(TAG_ELE_LEN-1, QPN_LEN) = new_tag;
        cnt_ele.range(8-1, 0) = 1;
        cnt_ele.range(CNT_ELE_LEN-1, 8) = new_dqpn;
        tags_addr_blk = new_set * TAG_BLK_NUM;
        cnt_addr_blk = new_set * CNT_BLK_NUM;

        prev_ts = prev_ts_arr[new_set];
        uni_tags = uni_tags_arr[new_set];
        tag_loop_iter_cnt = (uni_tags + TAG_ELE_NUM - 1) / TAG_ELE_NUM;
        uni_dqpns = uni_dqpns_arr[new_set];
        cnt_loop_iter_cnt = (uni_dqpns + CNT_ELE_NUM - 1) / CNT_ELE_NUM;
        
        cur_bitmap = sets_bitmap[new_bm_idx];
        hit_set = cur_bitmap[new_bm_oft];
        tag_hit = false;
        dqpn_hit = false;
        evict_idx = 0;

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
                Load_Tag_List_Loop: 
                // std::cout << "ts_diff < INTRA_GROUP_TH, tag_loop_iter_cnt = " << tag_loop_iter_cnt << std::endl;
                for (unsigned int i = 0; i < tag_loop_iter_cnt; i++) {
                    #pragma HLS pipeline II=1
                    #pragma HLS loop_flatten off
                    // should in burst transfer
                    tags_list_buf[i] = tagPort[tags_addr_blk+i];
                }
                Load_Cnt_List_Loop:
                for (unsigned int i = 0; i < cnt_loop_iter_cnt; i++) {
                    #pragma HLS pipeline II=1
                    #pragma HLS loop_flatten off
                    cnt_list_buf[i] = cntPort[cnt_addr_blk+i];
                }
                
                Search_Tag_List_Loop_1: 
                for (unsigned int i = 0; i < tag_loop_iter_cnt && !tag_hit; i++) {
                    #pragma HLS loop_flatten off
                    // load a line
                    tags_line = tags_list_buf[i];
                    Search_Tag_Line_Loop_1: 
                    // should be parallelized
                    for (unsigned int j = 0; j < TAG_ELE_NUM && ((i*TAG_ELE_NUM+j) < uni_tags); j++) {
                        #pragma HLS pipeline II=1
                        #pragma HLS loop_flatten off
                        #ifndef __SYNTHESIS__
                        std::cout << "Search_Tag_Line_Loop_Iter_" << i << "_" << j << std::endl;
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
                    cnt_line = cnt_list_buf[i];
                    Search_Cnt_Line_Loop_For_Evict: 
                    for (unsigned int j = 0; j < CNT_ELE_NUM && ((i*CNT_ELE_NUM+j) < uni_dqpns); j++) {
                        #pragma HLS pipeline II=1
                        #pragma HLS loop_flatten off
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
                #ifndef __SYNTHESIS__
                std::cout << "tag_hit = " << tag_hit << std::endl;
                std::cout << "dqpn_hit = " << dqpn_hit << std::endl;
                #endif  
                if (!tag_hit && uni_tags == WAYS_NUM) {
                    // evict a tag
                    // find the dqpn with miminum counter
                    #ifndef __SYNTHESIS__
                    std::cout << "Trap in Evict Phase, at the " << tsk_cnt << "trace" << std::endl;
                    #endif
                    evict_cnt = WAYS_NUM;
                    Search_Min_Cnt_List_Loop_For_Evict: 
                    for (unsigned int i = 0; i < cnt_loop_iter_cnt; i++) {
                        // std::cout << "I am inside Search_Min_Cnt_List_Loop_For_Evict" << std::endl;
                        #pragma HLS loop_flatten off
                        cnt_line = cnt_list_buf[i];
                        Search_Min_Cnt_Line_Loop_For_Evict: 
                        // shoud be parallelized
                        for (unsigned int j = 0; j < CNT_ELE_NUM && ((i*CNT_ELE_NUM+j) < uni_dqpns); j++) {
                            #pragma HLS pipeline II=1
                            #pragma HLS loop_flatten off
                            #ifndef __SYNTHESIS__
                            std::cout << "Search_Min_Cnt_Line_Loop_For_Evict_Iter_" << i << "_" << j << std::endl;
                            #endif
                            cnt_ele_cnt_temp = cnt_line.range(j*CNT_ELE_LEN + 8-1, j*CNT_ELE_LEN);
                            cnt_ele_dqpn_temp = cnt_line.range(((j+1) * CNT_ELE_LEN)-1, j*CNT_ELE_LEN + 8);
                            if (!(dqpn_hit && cnt_ele_dqpn_temp == new_dqpn) && cnt_ele_cnt_temp <= evict_cnt) {
                                evict_cnt = cnt_ele_cnt_temp;
                                evict_dqpn = cnt_ele_dqpn_temp;
                                evict_idx = i;
                                evict_idy = j;
                            }
                            if (dqpn_hit && cnt_ele_dqpn_temp == new_dqpn) {
                                // TODO: Remove this redundant op
                                dqpn_hit_idx = i;
                                dqpn_hit_idy = j;
                                dqpn_hit_cnt = cnt_ele_cnt_temp;
                            }
                        }
                        #ifndef __SYNTHESIS__
                        std::cout << "evict_dqpn = " << evict_dqpn << ";";
                        std::cout << "evict_cnt = " << evict_cnt << ";";
                        std::cout << "evict_idx = " << evict_idx << ";";
                        std::cout << "evict_idy = " << evict_idy << ";";
                        #endif
                    }
                    Evict_Write_Back_Cnt_List:{
                    // TODO:
                    // decrement the counter and remove the "zero" entry if necessary 
                    if (!dqpn_hit && evict_cnt == 1) {
                        // substitute the "zero" entry
                        if (evict_idy > 0){
                            cnt_evict_line.range(evict_idy*CNT_ELE_LEN-1, 0) = cnt_list_buf[evict_idx].range(evict_idy*CNT_ELE_LEN-1, 0);
                        }
                        cnt_evict_line.range((evict_idy+1)*CNT_ELE_LEN-1,evict_idy*CNT_ELE_LEN) = cnt_ele;
                        if (evict_idy < CNT_ELE_NUM - 1){
                            cnt_evict_line.range(DRAM_BUS_LEN-1, (evict_idy+1)*CNT_ELE_LEN) = cnt_list_buf[evict_idx].range(DRAM_BUS_LEN-1, (evict_idy+1)*CNT_ELE_LEN);
                        }
                        cntPort[cnt_addr_blk + evict_idx] = cnt_evict_line;
                        uni_dqpns_arr[new_set] = uni_dqpns; // unchanged
                    } else if (!dqpn_hit && evict_cnt > 1) {
                        // decrement and append, update two
                        // update evict element first
                        cnt_list_buf[evict_idx].range(evict_idy*CNT_ELE_LEN+8-1, evict_idy*CNT_ELE_LEN) = evict_cnt - 1;
                        if (uni_dqpns == cnt_loop_iter_cnt * CNT_ELE_NUM) {
                            cnt_evict_idx = cnt_loop_iter_cnt;
                            cnt_evict_idy = 0;
                        } else {
                            cnt_evict_idx = cnt_loop_iter_cnt - 1;
                            cnt_evict_idy = uni_dqpns - ((cnt_loop_iter_cnt-1) * CNT_ELE_NUM);
                        }
                        cnt_list_buf[cnt_evict_idx].range((cnt_evict_idy+1)*CNT_ELE_LEN-1, cnt_evict_idy*CNT_ELE_LEN) = cnt_ele;
                        if (evict_idx == cnt_evict_idx) {
                            cntPort[cnt_addr_blk + evict_idx] = cnt_list_buf[evict_idx];
                        } else {
                            cntPort[cnt_addr_blk + evict_idx] = cnt_list_buf[evict_idx];
                            cntPort[cnt_addr_blk + cnt_evict_idx] = cnt_list_buf[cnt_evict_idx];
                        }
                        uni_dqpns_arr[new_set] = uni_dqpns + 1;
                    } else if (dqpn_hit && evict_cnt == 1) {
                        #ifndef __SYNTHESIS__
                        std::cout << "dqpn_hit && evict == 1" << std::endl;
                        std::cout << "dqpn_hit_idx = " << dqpn_hit_idx << std::endl;
                        std::cout << "dqpn_hit_idy = " << dqpn_hit_idy << std::endl;
                        std::cout << "dqpn_hit_cnt = " << dqpn_hit_cnt << std::endl;
                        #endif
                        // remove and increment, complex update, do not delete, lazy?
                        cnt_list_buf[dqpn_hit_idx].range(dqpn_hit_idy*CNT_ELE_LEN+8-1, dqpn_hit_idy*CNT_ELE_LEN) = dqpn_hit_cnt + 1;
                        // move the last element to the evicted position
                        last_cnt_ele_idx = cnt_loop_iter_cnt - 1;
                        last_cnt_ele_idy = uni_dqpns - ((cnt_loop_iter_cnt - 1)*CNT_ELE_NUM) - 1;
                        #ifndef __SYNTHESIS__
                        std::cout << "last_cnt_ele_idx = " << last_cnt_ele_idx << std::endl;
                        std::cout << "last_cnt_ele_idy = " << last_cnt_ele_idy << std::endl;
                        std::cout << "evict_idx = " << evict_idx << std::endl;
                        std::cout << "evict_idy = " << evict_idy << std::endl;
                        #endif
                        last_cnt_ele = cnt_list_buf[last_cnt_ele_idx].range((last_cnt_ele_idy+1)*CNT_ELE_LEN-1, last_cnt_ele_idy*CNT_ELE_LEN);
                        cnt_list_buf[evict_idx].range((evict_idy+1)*CNT_ELE_LEN-1, evict_idy*CNT_ELE_LEN) = last_cnt_ele;
                        if (evict_idx == dqpn_hit_idx) {
                            cntPort[cnt_addr_blk + evict_idx] = cnt_list_buf[evict_idx];
                        } else {
                            cntPort[cnt_addr_blk + evict_idx] = cnt_list_buf[evict_idx];
                            cntPort[cnt_addr_blk + dqpn_hit_idx] = cnt_list_buf[dqpn_hit_idx]; 
                        }
                        uni_dqpns_arr[new_set] = uni_dqpns - 1;
                    } else {
                        // decrement and increment, update two lines
                        cnt_list_buf[evict_idx].range(evict_idy*CNT_ELE_LEN+8-1, evict_idy*CNT_ELE_LEN) = evict_cnt - 1;
                        cnt_list_buf[dqpn_hit_idx].range(dqpn_hit_idy*CNT_ELE_LEN+8-1, dqpn_hit_idy*CNT_ELE_LEN) = dqpn_hit_cnt + 1;
                        if (evict_idx == dqpn_hit_idx) {
                            cntPort[cnt_addr_blk + evict_idx] = cnt_list_buf[evict_idx];
                        } else {
                            cntPort[cnt_addr_blk + evict_idx] = cnt_list_buf[evict_idx];
                            cntPort[cnt_addr_blk + dqpn_hit_idx] = cnt_list_buf[dqpn_hit_idx]; 
                        }
                        uni_dqpns_arr[new_set] = uni_dqpns; // unchanged
                    }
                    }
                    Evict_Write_Back_Tag_List:{
                    // find the tag that has the qpn and insert the new element
                    find_evict_tag = false;
                    Search_Tag_List_Loop_For_Evict: 
                    for (unsigned int i = 0; i < tag_loop_iter_cnt && !find_evict_tag; i++) {
                        #pragma HLS loop_flatten off
                        tags_line = tags_list_buf[i];
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
                    tags_evict_line = tags_list_buf[tags_evict_idx];
                    tags_evict_line.range((tags_evict_idy+1) * TAG_ELE_LEN-1, tags_evict_idy*TAG_ELE_LEN) = tags_ele;
                    tagPort[tags_addr_blk+tags_evict_idx] = tags_evict_line;
                    }
                    // uni_tags_arr[new_set] = uni_tags + 1; 
                } else if (!tag_hit && uni_tags < WAYS_NUM) {
                    Normal_Write_Back_Cnt_List: {
                    // search cnt list and increment counter
                    cnt_insert_find = false;
                    if (dqpn_hit) {
                        // search 
                        // Search_Cnt_List_Loop_For_Evict: 
                        // for (unsigned int i = 0; i < cnt_loop_iter_cnt && !cnt_insert_find; i++) {
                        //     #pragma HLS loop_flatten off
                        //     cnt_line = cnt_list_buf[i];
                        //     Search_Cnt_Line_Loop_For_Evict: 
                        //     for (unsigned int j = 0; j < CNT_ELE_NUM && ((i*CNT_ELE_NUM+j) < uni_dqpns) && !cnt_insert_find; j++) {
                        //         #pragma HLS pipeline II=1
                        //         #pragma HLS loop_flatten off
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
                        #ifndef __SYNTHESIS__
                        std::cout << "new_dpqn = " << new_dqpn << ";";
                        std::cout << "cnt_insert_idx = " << cnt_insert_idx << ";";
                        std::cout << "cnt_insert_idy = " << cnt_insert_idy << std::endl;
                        #endif
                        cnt_insert_val = 1;
                    }
                    cnt_insert_ele.range(8-1, 0) = cnt_insert_val;
                    cnt_insert_ele.range(CNT_ELE_LEN-1, 8) = new_dqpn;
                    if (cnt_insert_idy == 0) {
                        cnt_insert_line.range(CNT_ELE_LEN-1, 0) = cnt_insert_ele;
                    } else {
                        cnt_insert_line.range(cnt_insert_idy*CNT_ELE_LEN-1, 0) = cnt_list_buf[cnt_insert_idx].range(cnt_insert_idy*CNT_ELE_LEN-1, 0);
                        cnt_insert_line.range((cnt_insert_idy+1)*CNT_ELE_LEN-1, cnt_insert_idy*CNT_ELE_LEN) = cnt_insert_ele;
                    }
                    cntPort[cnt_addr_blk+cnt_insert_idx] = cnt_insert_line;
                    }
                    Normal_Write_Back_Tag_List: {
                    if (uni_tags == tag_loop_iter_cnt * TAG_ELE_NUM) {
                        tags_line = tags_list_buf[tag_loop_iter_cnt];
                        tag_insert_idx = tag_loop_iter_cnt;
                        tag_insert_idy = 0;
                    } else {
                        tags_line = tags_list_buf[tag_loop_iter_cnt - 1];
                        tag_insert_idx = tag_loop_iter_cnt - 1;
                        tag_insert_idy = uni_tags - ((tag_loop_iter_cnt-1) * TAG_ELE_NUM);
                    }
                    
                    tags_insert_ele.range(QPN_LEN-1, 0) = new_dqpn;
                    tags_insert_ele.range(TAG_ELE_LEN-1, QPN_LEN) = new_tag;

                    // std::cout << "tag_insert_idx = " << tag_insert_idx << std::endl;
                    // std::cout << "tag_insert_idy = " << tag_insert_idy << std::endl;

                    if (tag_insert_idy == 0) {
                        tags_insert_line.range(TAG_ELE_LEN-1, 0) = tags_insert_ele;
                    } else {
                        tags_insert_line.range(tag_insert_idy*TAG_ELE_LEN-1, 0) = tags_list_buf[tag_insert_idx].range(tag_insert_idy*TAG_ELE_LEN-1, 0);
                        tags_insert_line.range((tag_insert_idy+1)*TAG_ELE_LEN-1, tag_insert_idy*TAG_ELE_LEN) = tags_insert_ele;
                    }
                    tagPort[tags_addr_blk+tag_insert_idx] = tags_insert_line;
                    }
                    uni_tags_arr[new_set] = uni_tags + 1;
                    if (!dqpn_hit) {
                        uni_dqpns_arr[new_set] = uni_dqpns + 1;
                    }
                } else {
                    // tag is hit, update timestamp only
                    #ifndef __SYNTHESIS__
                    std::cout << "new_dqpn = " << new_dqpn << " hit the same tag" << std::endl;
                    #endif
                }
            }else {
                #ifndef __SYNTHESIS__
                std::cout << "ts_diff >= INTRA_GROUP_TH, tag_loop_iter_cnt = " << tag_loop_iter_cnt << std::endl;
                std::cout << "uni_tags = " << uni_tags << std::endl;
                #endif
                if (uni_tags == WAYS_NUM) {
                    // clearly, there are only one dqpn left if there is a attacker.
                    // load the first line only
                    cnt_list_buf[0] = cntPort[cnt_addr_blk + 0];
                    dqpn_attacker_ele = cnt_list_buf[0].range(CNT_ELE_LEN-1, 8);
                    #ifndef __SYNTHESIS__
                    std::cout << "dqpn_attacker_ele: " << dqpn_attacker_ele << std::endl;
                    #endif
                    if (dqpn_attacker_ele == new_dqpn) {
                        // std::cout << "detect attacker with dqpn = " << dqpn_attacker_ele << std::endl;
                        outStream.write(dqpn_attacker_ele);
                    }
                }
                // clear this ClusterGenPerSet
                sets_bitmap[new_bm_idx][new_bm_oft] = 0;
            }
        } else {
            // std::cout << "hit_set == 0" << std::endl;
            Init_Write_Back_Tag_List: {
            tags_beat.range(TAG_ELE_LEN-1, 0) = tags_ele;
            tagPort[tags_addr_blk] = tags_beat;
            }
            Init_Write_Back_Cnt_List: {
            cnt_beat.range(CNT_ELE_LEN-1, 0) = cnt_ele;
            cntPort[cnt_addr_blk] = cnt_beat;
            }
            sets_bitmap[new_bm_idx][new_bm_oft] = 1;
            uni_tags_arr[new_set] = 1;
            uni_dqpns_arr[new_set] = 1;
            #ifndef __SYNTHESIS__
            std::cout << "new_dqpn = " << new_dqpn << " is first inserted into new_set = " << new_set << std::endl;
            #endif
        }
        prev_ts_arr[new_set] = new_ts;
        #ifndef __SYNTHESIS__
        std::cout << "finish a tsk " << tsk_cnt << std::endl;
        #endif
    }
}


#if (USE_TAPA == 1)
void scadet_v1_top (
    meta_in_stream& inStream,
    meta_out_stream& outStream,
    tapa::mmap<BUS_TYPE> tagPort,
    tapa::mmap<BUS_TYPE> cntPort
) {
    tapa::task().invoke(scadet_v1, inStream, outStream, tagPort, cntPort);
}
#endif