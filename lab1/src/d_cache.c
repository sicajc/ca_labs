#include "cache.h"
#include <assert.h>

cache_block d_cache_mem[D_CACHE_SETS][D_CACHE_WAYS];

uint32_t d_cache(bool read,
                     uint32_t data,
                     uint32_t addr,
                     uint32_t *time)
{
    // Extract the address into tag, index and offset
    // tag has 19 bits, index has 8 bits and offset has 5 bits

    cache_info_t cache_info = calculate_cache_info(addr,
                                                   WORD_SIZE,
                                                   D_CACHE_SIZE,
                                                   D_CACHE_WAYS,
                                                   D_CACHE_BLOCK_SIZE);

    uint32_t tag = cache_info.tags;
    uint32_t index = cache_info.index;
    uint32_t offset = cache_info.offset;

#ifdef DEBUG
    if (index == 12)
    {
        std::cerr << "======================================'\n";
        std::cerr << "Read/Write: " << read << '\n';
        std::cerr << "Addr: " << addr << '\n';
        std::cerr << "Index: " << index << " Tag: " << tag << " Offset: " << offset << '\n';
        std::cerr << "Data: " << data << '\n';
        std::cerr << "======================================'\n";
    }
#endif

    // cache hit
    // Compare with all the tags with the given index
    for (int set_iter = 0; set_iter < D_CACHE_WAYS; set_iter++)
    {
        // hits
        if (d_cache_mem[index][set_iter].key.tag == tag && d_cache_mem[index][set_iter].key.valid == 1)
        {
            // during a hit, update the LRU counter, decrement other blocks lru counter
            d_cache_mem[index][set_iter].key.lru_cnt = D_CACHE_WAYS - 1;
            // Decrement the LRU counter for other blocks
            for (int set_iter2 = 0; set_iter2 < D_CACHE_WAYS; set_iter2++)
            {
                if (set_iter2 != set_iter)
                {
                    if (d_cache_mem[index][set_iter2].key.lru_cnt > 0)
                    {
                        d_cache_mem[index][set_iter2].key.lru_cnt--;
                    }
                }
            }

            if (read == false)
            {
                // write to the cache
                d_cache_mem[index][set_iter].value.value[offset] = data;
                // mark the dirt bit
                d_cache_mem[index][set_iter].key.dirty = 1;
            }

            return d_cache_mem[index][set_iter].value.value[offset];
        }
    }

    // CACHE MISS
    int least_lru_cnt_val = D_CACHE_WAYS - 1;
    int least_lru_block_num = 0;
    *time = *time + 50;

    // search for the lru block as a victim block for replacement
    for (int set_iter = 0; set_iter < D_CACHE_WAYS; set_iter++)
    {
        // Fill in the invalid cache block first, if there is invalid cache block, fill it first
        if (d_cache_mem[index][set_iter].key.valid == 0)
        {
            least_lru_block_num = set_iter;
            break;
        }

        // search for the LRU block
        // Traverse the block of the set, record the number
        // of the LRU block, later replace this block with new mem block
        if (d_cache_mem[index][set_iter].key.lru_cnt < least_lru_cnt_val)
        {
            least_lru_cnt_val = d_cache_mem[index][set_iter].key.lru_cnt;
            least_lru_block_num = set_iter;
        }
    }

    // Replace the LRU block with the new value
    d_cache_mem[index][least_lru_block_num].key.valid = 1;
    d_cache_mem[index][least_lru_block_num].key.lru_cnt = D_CACHE_WAYS - 1;
    d_cache_mem[index][least_lru_block_num].key.tag = tag;

    // Decrements the LRU counter for other blocks
    for (int set_iter = 0; set_iter < D_CACHE_WAYS; set_iter++)
    {
        if (set_iter != least_lru_block_num)
        {
            if (d_cache_mem[index][set_iter].key.lru_cnt > 0)
            {
                d_cache_mem[index][set_iter].key.lru_cnt--;
            }
        }
    }

    // The blocks is going to be replaced, if this is a dirty blocks, writes it back to memory
    if (d_cache_mem[index][least_lru_block_num].key.dirty == 1)
    {
        // write the whole block back to memory
        for (int word_iter = 0; word_iter < D_CACHE_WORDS_IN_BLOCK; word_iter++)
        {
            uint32_t word = d_cache_mem[index][least_lru_block_num].value.value[word_iter];

            uint32_t wb_block_addr = (d_cache_mem[index][least_lru_block_num].key.tag << cache_info.tag_bit_shift) | (index << cache_info.index_bit_shift);

            // Replace this with memory writes function
            // pseudo_data_mem[wb_block_addr + word_iter] = word;
        }

        // set the block to clean
        d_cache_mem[index][least_lru_block_num].key.dirty = 0;
    }

    // Fetch the whole block from the memory
    for (int word_iter = 0; word_iter < D_CACHE_WORDS_IN_BLOCK; word_iter++)
    {
        // Fetch the data from memory, use 1234 as the value first, fetch 32bits at once
        // Notice that to fetch the data from memory, you have to normalize and fetch from the beginning of the block
        // Not from the start, you dumb dumb
        uint32_t word_addr = addr / WORD;
        uint32_t block_addr_start = word_addr / BLOCK;

        // replace this with memory read function
        // uint32_t word = pseudo_data_mem[block_addr_start * BLOCK + word_iter];
        uint32_t word = 0;
        d_cache_mem[index][least_lru_block_num].value.value[word_iter] = word;
    }

    if (read == false) // write
    {
        // write to the cache block
        d_cache_mem[index][least_lru_block_num].value.value[offset] = data;
        d_cache_mem[index][least_lru_block_num].key.dirty = 1;
    }

    return d_cache_mem[index][least_lru_block_num].value.value[offset];
}