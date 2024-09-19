#include "cache.hpp"
#include "l2_cache.hpp"
#include "mshr.hpp"
#include "l2_mem.hpp"

void init_L2_cache()
{
    init_l2_cache();
    init_mshr();
}

// Two caches uses this function, L1D and L1I
// Note that L2 $ transfers block!!!
// Thus the return type should be a whole L1 block
cache_block L1_cache_access_L2_cache(memory_request_t L1_req)
{
    decoded_addr_info_t decoded_addr = decode_addr(L1_req.addr,
                                                    WORD_SIZE,
                                                    L2_CACHE_SIZE,
                                                    L2_CACHE_WAYS,
                                                    L2_CACHE_BLOCK_SIZE);

    // First check if hit or miss in L2 cache
    hit_or_miss l2_cache_status = cache_hit_or_miss(decoded_addr.tags, decoded_addr.index, L1_req.req_cache_type);

    cache_block block_temp;

    if(l2_cache_status==HIT)
    {
        // see if it is a read or write request
        if (L1_req.req_op_type == WRITE)
        {
            l1_write_l2_cache_mem(decoded_addr.tags, decoded_addr.index, decoded_addr.offset, L1_req.block,L1_req.req_cache_type);
            return block_temp;
        }
        else
        {
            // read from L2 cache, L2 cache shall returns the whole block!
            cache_block read_block = read_l2_cache_mem(decoded_addr.tags, decoded_addr.index, decoded_addr.offset,L1_req.req_cache_type);
            return read_block;
        }
    }
    else // MISS
    {
        // check if request is in MSHR already, waiting for dram fill
        if (is_request_in_mshr(decoded_addr.block_addr,L1_req.req_cache_type) == true) {

            return block_temp;
        }

        // check if MSHR is full, mshr full means we have to stall the request
        if (is_mshr_full() == true) {
            return block_temp;
        }

        // add request to MSHR
        add_request_to_mshr(decoded_addr.block_addr, L1_req.req_cache_type);

        // First mimics a memory to test if l2_cache & mshr are working
        // stalls the l1 cache request, it waits for the dram fill to returns
#ifdef TEST_L2_CACHE
        cache_block block = read_block_from_mem(L1_req.req_cache_type, decoded_addr.tags, decoded_addr.index);
        dram_writes_l2_cache_mem(decoded_addr.tags, decoded_addr.index, decoded_addr.offset,L1_req.req_cache_type,block);
        invalidate_mshr_entry(decoded_addr.block_addr);
#elif  FULL_INTEGRATION
        if(DRAM_fill_notification(dram_req)) dram_writes_l2_cache_mem(decoded_addr.tags, decoded_addr.index, decoded_addr.offset, dram_req.data);
#endif
        return block;
    }
}

bool DRAM_fill_notification(fill_request_t dram_fill_req)
{
    decoded_addr_info_t decoded_addr = decode_addr( dram_fill_req.addr,
                                                    WORD_SIZE,
                                                    L2_CACHE_SIZE,
                                                    L2_CACHE_WAYS,
                                                    L2_CACHE_BLOCK_SIZE);

    // invalidate MSHR entry
    invalidate_mshr_entry(decoded_addr.block_addr);

    // fill L2 cache
    dram_writes_l2_cache_mem(decoded_addr.tags, decoded_addr.index, decoded_addr.offset,dram_fill_req.req_cache_type,dram_fill_req.block);

    // send fill notification to L1 cache
    return true;
}
