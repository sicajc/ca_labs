#include "cache.hpp"
#include "l2_cache.hpp"
#include "mshr.hpp"
#include "l2_mem.hpp"

void init_L2_cache()
{
    init_l2_cache();
    init_mshr();
}

memory_request_t L1_cache_access_L2_cache(memory_request_t L1_req)
{
    decoded_addr_info_t decoded_addr = decode_addr(L1_req.addr,
                                                    WORD_SIZE,
                                                    L2_CACHE_SIZE,
                                                    L2_CACHE_WAYS,
                                                    L2_CACHE_BLOCK_SIZE);

    // First check if hit or miss in L2 cache
    hit_or_miss l2_cache_status = cache_hit_or_miss(decoded_addr.tags, decoded_addr.index);

    if(l2_cache_status==HIT)
    {
        read_l2_cache_mem(decoded_addr.tags, decoded_addr.index, decoded_addr.offset);
        L1_req.valid = false;
        return L1_req;
    }
    else // MISS
    {
        // check if request is in MSHR already, waiting for dram fill
        if (is_request_in_mshr(decoded_addr.block_addr) == true) {
            L1_req.valid = false;
            return L1_req;
        }

        // check if MSHR is full, mshr full means we have to stall the request
        if (is_mshr_full() == true) {
            L1_req.valid = false;
            return L1_req;
        }

        // send request to DRAM
        memory_request_t dram_req = L1_req;
        dram_req.addr = L1_req.addr;
        dram_req.data = 0;
        dram_req.cycle_time = L1_req.cycle_time;
        dram_req.status = L1_req.status;

        // add request to MSHR
        add_request_to_mshr(decoded_addr.block_addr, L1_req.status);

        return dram_req;
    }
}

void DRAM_fill_notification(fill_request_t dram_fill_req)
{
    decoded_addr_info_t decoded_addr = decode_addr(dram_fill_req.addr,
                                                    WORD_SIZE,
                                                    L2_CACHE_SIZE,
                                                    L2_CACHE_WAYS,
                                                    L2_CACHE_BLOCK_SIZE);

    // invalidate MSHR entry
    invalidate_mshr_entry(decoded_addr.block_addr);
}

void L2_cache_access_dram()
{
    // get the first request in MSHR
    mshr_entry_t mshr_entry = mshr[0];

    // send request to DRAM
    memory_request_t dram_req;
    dram_req.addr = mshr_entry.block_addr;
    dram_req.cycle_time = 0;
    dram_req.status = mshr_entry.status;
}

void L2_fill_notification()
{
    // get the first request in MSHR
    mshr_entry_t mshr_entry = mshr[0];

    // send request to L1 cache
    fill_request_t l1_fill_req;
    l1_fill_req.addr = mshr_entry.block_addr;
    l1_fill_req.status = mshr_entry.status;

    // send request to L1 cache
    // L1_cache_access_L2_cache(l1_fill_req);
}

void DRAM_fill_notification(fill_request_t dram_fill_req)
{
    decoded_addr_info_t decoded_addr = decode_addr(dram_fill_req.addr,
                                                    WORD_SIZE,
                                                    L2_CACHE_SIZE,
                                                    L2_CACHE_WAYS,
                                                    L2_CACHE_BLOCK_SIZE);

    // invalidate MSHR entry
    invalidate_mshr_entry(decoded_addr.block_addr);
}
