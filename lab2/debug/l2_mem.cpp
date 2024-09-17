#include "l2_mem.hpp"

// l2 cache mem
std::array<std::array<cache_block, L2_CACHE_WAYS>, L2_CACHE_SETS> l2_cache_mem;

// initialize l2 cache
void init_l2_cache()
{
    // Traverse the std::array of cache_block using range bound for loop
    for (auto &set : l2_cache_mem)
    {
        for (auto &block : set)
        {
            block.key.dirty = 0;
            block.key.valid = 0;
            block.key.lru_cnt = 0;
            block.key.tag = 0;
        }
    }
}

// hit or miss
hit_or_miss cache_hit_or_miss(const uint32_t tag, const uint32_t index)
{
    // Traverse the std::array of cache_block using range bound for loop
    bool is_hit = std::any_of(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                              [tag](const cache_block &block)
                              {
                                  return block.key.tag == tag && block.key.valid == 1;
                              });

    return is_hit ? HIT : MISS;
}

uint32_t find_ways_of_least_recently_used_block(const uint32_t index)
{
    // Given an index, find the way of the least recently used block
    uint32_t lru_way = 0;
    uint32_t lru_cnt = L2_CACHE_WAYS - 1;

    for (uint32_t i = 0; i < L2_CACHE_WAYS; i++)
    {
        if (l2_cache_mem[index][i].key.lru_cnt < lru_cnt)
        {
            lru_cnt = l2_cache_mem[index][i].key.lru_cnt;
            lru_way = i;
        }
    }

    return lru_way;
}

// write to l2 cache
void write_l2_cache_mem(const uint32_t tag, const uint32_t index, const uint32_t offset, const uint32_t data)
{
    // Traverse the std::array of cache_block using range bound for loop
    auto block = std::find_if(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                              [tag](const cache_block &block)
                              {
                                  return block.key.tag == tag && block.key.valid == 1;
                              });

    // Write the data to the block
    block->value.value[offset] = data;
    block->key.dirty = 1;

    update_l2_cache_lru(tag, index);
}

// read from l2 cache
uint32_t read_l2_cache_mem(const uint32_t tag, const uint32_t index, const uint32_t offset)
{
    // Traverse the std::array of cache_block using range bound for loop
    auto block = std::find_if(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                              [tag](const cache_block &block)
                              {
                                  return block.key.tag == tag && block.key.valid == 1;
                              });

    update_l2_cache_lru(tag, index);

    // Return the data
    return block->value.value[offset];
}

// update l2 cache lru
void update_l2_cache_lru(const uint32_t tag, const uint32_t index)
{
    // Given tag and index, set the given block's lru_cnt to highest priority,
    // decrement all other blocks' lru_cnt
    for (auto &block : l2_cache_mem[index])
        if (block.key.tag != tag && block.key.valid == 1 && block.key.lru_cnt > 0)
            block.key.lru_cnt--;
        else
            block.key.lru_cnt = L2_CACHE_WAYS - 1;
}

// reads a word from memory
uint32_t read_mem(const req_cache _req_cache, const uint32_t addr)
{
    // Given the requested cache type and address, read from different memory
    // Returns a 32-bit word
    uint32_t word = _req_cache == D_CACHE ? test_data_mem[addr / WORD] : test_inst_mem[addr / WORD]; // uses word address
    return word;
}

// read a block from memory
cache_block read_block_from_mem(const req_cache _req_cache,
                                const cache_block selected_block,
                                const uint32_t tag,
                                const uint32_t index)
{
    cache_block block = selected_block;

    uint32_t tag_bit_shift = (uint32_t)(log2(L2_CACHE_SIZE / L2_CACHE_WAYS));
    uint32_t index_bit_shift = log2(L2_CACHE_BLOCK_SIZE);

    // Rebuild the address from the tag and index
    uint32_t block_addr_start = (tag << tag_bit_shift) | (index << index_bit_shift);

    // Read the whole block of data from the specified memory type
    for (int i = 0; i < L2_CACHE_WORDS_IN_BLOCK; i++)
    {
        uint32_t read_addr = block_addr_start + uint32_t((i * WORD));
        uint32_t data = read_mem(_req_cache, read_addr);

        block.value.value[i] = data;
    }

    return block;
}