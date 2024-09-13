#include "cache.h"
#include "shell.h"
#include <assert.h>

cache_block i_cache_mem[I_CACHE_SETS][I_CACHE_WAYS];

cache_return_data_t i_cache(const uint32_t addr,const uint32_t cycle_time)
{
    //@param addr: the pc address to be read, multiple of 4 bytes
    //@param cycle_time: the cycle cycle_time taken to read the data
    // Updates the cycle_time of cycle if miss, if miss, cycle_time = cycle_time + 50 else cycle_time = cycle_time
    // Updates the cache status of i cache
    //@return the value from the mem[addr]

    // Extract the address into tag, index and offset
    // tag has 21 bits, index has 6 bits and offset has 5 bits

    cache_info_t cache_info = calculate_cache_info(addr,
                                                   (uint32_t)WORD_SIZE,
                                                   (uint32_t)I_CACHE_SIZE,
                                                   (uint32_t)I_CACHE_WAYS,
                                                   (uint32_t)I_CACHE_BLOCK_SIZE);

    cache_return_data_t cache_return_data;

    uint32_t tag = cache_info.tags;
    uint32_t index = cache_info.index;
    uint32_t offset = cache_info.offset;

    // Compare with all the tags with the given index
    for (int set_iter = 0; set_iter < I_CACHE_WAYS; set_iter++)
    {
        // hits
        if (i_cache_mem[index][set_iter].key.tag == tag && i_cache_mem[index][set_iter].key.valid == 1)
        {
            // during a hit, update the LRU counter, decrement other blocks lru counter
            i_cache_mem[index][set_iter].key.lru_cnt = I_CACHE_WAYS - 1;

            // Decrement the LRU counter for other blocks
            for (int set_iter2 = 0; set_iter2 < I_CACHE_WAYS; set_iter2++)
            {
                if (set_iter2 != set_iter)
                {
                    if (i_cache_mem[index][set_iter2].key.lru_cnt > 0)
                    {
                        i_cache_mem[index][set_iter2].key.lru_cnt--;
                    }
                }
            }

            cache_return_data.cycle_time = cycle_time;
            cache_return_data.value = i_cache_mem[index][set_iter].value.value[offset];

            // Return the cycle time and data
            return cache_return_data;
        }
    }

    // Fetch the value from memory and insert it into the cache
    // Update the cache with the new value using LRU policy
    // For the given set, search for the LRU block and replace it with the new value
    // LRU block has the most recently used counter

    int least_lru_cnt_val = I_CACHE_WAYS - 1;
    int least_lru_block_num = 0;
    uint32_t new_cycle_time = cycle_time + 50;

    for (int set_iter = 0; set_iter < I_CACHE_WAYS; set_iter++)
    {
        // Fill in the invalid cache block first, if there is invalid cache block, fill it first
        if (i_cache_mem[index][set_iter].key.valid == 0)
        {
            least_lru_block_num = set_iter;
            break;
        }

        // search for the LRU block
        // Traverse the block of the set, record the number
        // of the LRU block, later replace this block with new mem block
        if (i_cache_mem[index][set_iter].key.lru_cnt < least_lru_cnt_val)
        {
            least_lru_cnt_val = i_cache_mem[index][set_iter].key.lru_cnt;
            least_lru_block_num = set_iter;
        }
    }

    // Replace the LRU block with the new value
    i_cache_mem[index][least_lru_block_num].key.valid = 1;
    i_cache_mem[index][least_lru_block_num].key.lru_cnt = 3;
    i_cache_mem[index][least_lru_block_num].key.tag = tag;

    // Decrements the LRU counter for other blocks
    for (int set_iter = 0; set_iter < I_CACHE_WAYS; set_iter++)
    {
        if (set_iter != least_lru_block_num)
        {
            if (i_cache_mem[index][set_iter].key.lru_cnt > 0)
            {
                i_cache_mem[index][set_iter].key.lru_cnt--;
            }
        }
    }

    uint32_t block_addr_start = addr - addr%I_CACHE_BLOCK_SIZE;

    // gets 32 bits = 4B from memory, to fill in the whole blok, which is 32B
    for (int word_iter = 0; word_iter < I_CACHE_WORDS_IN_BLOCK; word_iter++)
    {
        // Fetch the data from memory, use 1234 as the value first, fetch 32bits at once
        // Note the fetch addr is from the start of the whole block, 1 block is 32B which is 8 words
        // The addr is the start of the block, not the start of the word
        // uint32_t word_addr = addr / WORD;

        uint32_t byte_addr = block_addr_start + word_iter * WORD;

        // Replace with fetching word from the function
        i_cache_mem[index][least_lru_block_num].value.value[word_iter] = mem_read_32(byte_addr);
    }

    cache_return_data.value = i_cache_mem[index][least_lru_block_num].value.value[offset];
    cache_return_data.cycle_time = new_cycle_time;

    return cache_return_data;
}