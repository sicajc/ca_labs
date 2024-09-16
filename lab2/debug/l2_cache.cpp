#include "cache.hpp"
#include <algorithm>
#include <assert.h>

// L2 cache
std::array<std::array<cache_block, L2_CACHE_WAYS>, L2_CACHE_SETS> l2_cache_mem;

// l2 cache hit or miss
static hit_or_miss cache_hit_or_miss(const uint32_t tag, const uint32_t index)
{

    // Traverse the std::array of cache_block using range bound for loop
    bool is_hit = std::any_of(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                              [tag](const cache_block &block)
                              {
                                  return block.key.tag == tag && block.key.valid == 1;
                              });

    return is_hit ? HIT : MISS;
}

static void access_mshr(const uint32_t addr, const bool done, const cache_status status)
{
    // if done is true, invalidates the mshr entry
    if (done == true)
    {
        // Find the entry in the MSHR
        auto entry = std::find_if(mshr.begin(), mshr.end(),
                                  [addr](const mshr_entry_t &entry)
                                  {
                                      return entry.addr == addr;
                                  });

        // Invalidate the entry
        entry->valid = false;
        entry->done = true;
    }
    else
    {
        // Access the MSHR
        // First traverse the mshr to check if the address exists in the MSHR
        bool addr_presented = std::any_of(mshr.begin(), mshr.end(),
                                          [addr](const mshr_entry_t &entry)
                                          {
                                              return entry.addr == addr;
                                          });

        if (addr_presented == false)
        {
            // Find the first invalid entry in the MSHR
            auto entry = std::find_if(mshr.begin(), mshr.end(),
                                      [](const mshr_entry_t &entry)
                                      {
                                          return entry.valid == false;
                                      });

            // Update the entry
            entry->valid = true;
            entry->done = false;
            entry->status = status;
            entry->addr = addr;
        }
    }

    // Update the cycle time
    // Update the status
    // Return the updated memory request
}

static void send_request_to_dram(const memory_request_t memory_request, const fill_request_t fill_request)
{
    // Send the request to the dram
    // Update the cycle time
    // Update the status
    // Return the updated memory request
}

cache_return_data_l2_t l2_cache(const memory_request_t memory_request, const fill_request_t fill_request)
{
    // Extract the address into tag, index and offset
    // tag has 19 bits, index has 8 bits and offset has 5 bits

    cache_info_t cache_info = calculate_cache_info(memory_request.addr,
                                                   WORD_SIZE,
                                                   L2_CACHE_SIZE,
                                                   L2_CACHE_WAYS,
                                                   L2_CACHE_BLOCK_SIZE);

    uint32_t tag = cache_info.tags;
    uint32_t index = cache_info.index;
    uint32_t offset = cache_info.offset;

    // cache hit
    // Compare with all the tags with the given index
    hit_or_miss hit = cache_hit_or_miss(tag, index);

    cache_return_data_l2_t return_data;
    // initialize return data
    return_data.fill_l1_req.addr = 0;
    return_data.fill_l1_req.data = 0;
    return_data.fill_l1_req.cycle_time = 0;
    return_data.fill_l1_req.status = L1D_RD;

    return_data.access_to_dram_req.addr = 0;
    return_data.access_to_dram_req.data = 0;
    return_data.access_to_dram_req.cycle_time = 0;
    return_data.access_to_dram_req.status = L1D_RD;

    // cycle time
    uint32_t new_cycle_time = 0;

    // hit case
    if (hit == HIT)
    {
        // L2 CACHE HIT, updates the LRU counter and returns the value
        new_cycle_time = memory_request.cycle_time + L2_HIT_LATENCY;

        // Find the block with the given tag
        auto block = std::find_if(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                                  [tag](const cache_block &block)
                                  {
                                      return block.key.tag == tag;
                                  });

        // Decrement the LRU counter for all the other blocks except the current block
        for (auto &block_iter : l2_cache_mem[index])
        {

            if (&block_iter != &(*block))
            {
                if (block_iter.key.lru_cnt > 0)
                {
                    block_iter.key.lru_cnt--;
                }
            }
        }

        // Set the LRU counter to 0 for the block that was hit
        block->key.lru_cnt = L2_CACHE_WAYS - 1;

        return_data.fill_l1_req.addr = memory_request.addr;
        return_data.fill_l1_req.data = block->value.value[offset];
        return_data.fill_l1_req.cycle_time = new_cycle_time;
        return_data.fill_l1_req.status = memory_request.status;

        // return the value
        return return_data;
    }

    // L2 CACHE MISS,updates the MSHR and sends memory request to MEM, updates the cache state only when memory
    // returned the data, i.e. memory send the fill request back
    access_mshr(memory_request.addr, false, memory_request.status);

    // send request to dram
    send_request_to_dram(memory_request, fill_request);
}
