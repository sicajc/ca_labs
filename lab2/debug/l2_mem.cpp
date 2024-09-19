#include "l2_mem.hpp"

// l2 cache mem
std::array<std::array<l2_cache_block_t, L2_CACHE_WAYS>, L2_CACHE_SETS> l2_cache_mem;

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

// update l2 cache lru
void update_l2_cache_lru(const uint32_t tag, const uint32_t index, const req_cache _req_cache)
{
    // Given tag and index, set the given block's lru_cnt to highest priority,
    // decrement all other blocks' lru_cnt
    for (auto &block : l2_cache_mem[index])
        if (block.key.tag != tag && block.key.req_cache_type != _req_cache && block.key.valid == 1 && block.key.lru_cnt > 0)
            block.key.lru_cnt--;
        else
            block.key.lru_cnt = L2_CACHE_WAYS - 1;
}

// hit or miss
hit_or_miss cache_hit_or_miss(const uint32_t tag, const uint32_t index, const req_cache _req_cache)
{
    // Traverse the std::array of cache_block using range bound for loop
    bool is_hit = std::any_of(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                              [tag, _req_cache](const l2_cache_block_t &block)
                              {
                                  return block.key.tag == tag && block.key.valid == 1 && block.key.req_cache_type == _req_cache;
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

// L1 write to l2 cache
void l1_write_l2_cache_mem(const uint32_t tag, const uint32_t index, const uint32_t offset, const cache_block _block, const req_cache _req_cache)
{
    // Traverse the std::array of cache_block using range bound for loop
    auto block = std::find_if(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                              [tag, _req_cache](const l2_cache_block_t &block)
                              {
                                  return block.key.tag == tag && block.key.valid == 1 && block.key.req_cache_type == _req_cache;
                              });

    if (block == l2_cache_mem[index].end())
    {
        std::cerr << "Block not found in L2 cache" << std::endl;
        assert(false);
    }
    else
    {
        // Write the data to the block
        block->value = _block.value;

        block->key.dirty = 1;
        // Update the lru
        update_l2_cache_lru(tag, index, _req_cache);
    }
}

// dram writes l2 cache mem
void dram_writes_l2_cache_mem(const uint32_t tag, const uint32_t index, const uint32_t offset, const req_cache _req_cache, const cache_block _block)
{
    // _block's cache type is not the same as the cache type in the l2 cache

    // First search for in valid block in set
    // If found, write to that block
    // else find the least recently used block and write to that block
    auto block = std::find_if(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                              [tag](const l2_cache_block_t &block)
                              {
                                  return block.key.valid == 0;
                              });

    if (block != l2_cache_mem[index].end())
    {
        block->key.tag = tag;
        block->key.valid = 1;
        block->value = _block.value;
        block->key.dirty = 0;
        block->key.req_cache_type = _req_cache;
    }
    else // No invalid block found, must replace the least recently used block
    {
        uint32_t lru_way = find_ways_of_least_recently_used_block(index);
        // Must write back to the memory if the block is dirty, here it is writing back to dram
        if (l2_cache_mem[index][lru_way].key.dirty == 1)
        {
            cache_block _block;
            _block.value = l2_cache_mem[index][lru_way].value;

            // Need to write block back to dram
            write_data_block_to_mem(l2_cache_mem[index][lru_way].key.req_cache_type, l2_cache_mem[index][lru_way].key.tag, index, _block);
        }

        l2_cache_mem[index][lru_way].key.tag = tag;
        l2_cache_mem[index][lru_way].key.valid = 1;
        l2_cache_mem[index][lru_way].value = _block.value;
        l2_cache_mem[index][lru_way].key.dirty = 0;
        l2_cache_mem[index][lru_way].key.req_cache_type = _req_cache;
    }
}

// read from l2 cache after a hit
cache_block read_l2_cache_mem(const uint32_t tag, const uint32_t index, const uint32_t offset, const req_cache _req_cache)
{
    // Traverse the std::array of cache_block using range bound for loop
    auto block = std::find_if(l2_cache_mem[index].begin(), l2_cache_mem[index].end(),
                              [tag, _req_cache](const l2_cache_block_t &block)
                              {
                                  return block.key.tag == tag && block.key.valid == 1 && block.key.req_cache_type == _req_cache;
                              });

    if (block == l2_cache_mem[index].end())
    {
        std::cerr << "Block not found in L2 cache" << std::endl;
        assert(false);
    }
    else
    {
        cache_block _cache_block;

        _cache_block.value = block->value;

        _cache_block.key.dirty = block->key.dirty;
        _cache_block.key.valid = block->key.valid;
        _cache_block.key.lru_cnt = L2_CACHE_WAYS - 1;
        update_l2_cache_lru(tag, index, _req_cache);
        // Return the data
        return _cache_block;
    }
}

// reads a word from memory
uint32_t read_mem(const req_cache _req_cache, const uint32_t addr)
{
    // Given the requested cache type and address, read from different memory
    // Returns a 32-bit word
    uint32_t word = _req_cache == D_CACHE ? test_data_mem[addr / WORD] : test_inst_mem[addr / WORD]; // uses word address
    return word;
}

// read a block from memory, should only read the block from memory
cache_block read_block_from_mem(const req_cache _req_cache,
                                const uint32_t tag,
                                const uint32_t index)
{
    cache_block block;

    block.key.dirty = 0;
    block.key.valid = 1;
    block.key.lru_cnt = L2_CACHE_WAYS - 1;
    block.key.tag = tag;

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

// writes a word to memory
void write_mem(const req_cache _req_cache, const uint32_t addr, const uint32_t data)
{
    // Given the requested cache type, address, and data, write to different memory
    // Returns the written data
    if (_req_cache == D_CACHE)
    {
        test_data_mem[addr / WORD] = data;
    }
    else
    {
        test_inst_mem[addr / WORD] = data;
    }
}

// write block to memory
void write_data_block_to_mem(const req_cache _req_cache, const uint32_t tag, const uint32_t index, const cache_block _block)
{
    uint32_t tag_bit_shift = (uint32_t)(log2(L2_CACHE_SIZE / L2_CACHE_WAYS));
    uint32_t index_bit_shift = log2(L2_CACHE_BLOCK_SIZE);

    // Rebuild the address from the tag and index
    uint32_t block_addr_start = (tag << tag_bit_shift) | (index << index_bit_shift);

    // Write the whole block of data to the specified memory type
    for (int i = 0; i < L2_CACHE_WORDS_IN_BLOCK; i++)
    {
        uint32_t write_addr = block_addr_start + uint32_t((i * WORD));
        write_mem(_req_cache, write_addr, _block.value.value[i]);
    }
}
