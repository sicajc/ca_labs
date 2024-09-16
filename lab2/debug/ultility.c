#include "cache.h"
#include <assert.h>

void init_cache()
{
    // Initialize the cache
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            i_cache[i][j].key.dirty = 0;
            i_cache[i][j].key.valid = 0;
            i_cache[i][j].key.lru_cnt = 0;
            i_cache[i][j].key.tag = 0;
            for (int k = 0; k < 8; k++)
            {
                i_cache[i][j].value.value[k] = 0;
            }
        }
    }

    for (int i = 0; i < 256; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            d_cache[i][j].key.dirty = 0;
            d_cache[i][j].key.valid = 0;
            d_cache[i][j].key.lru_cnt = 0;
            d_cache[i][j].key.tag = 0;
            for (int k = 0; k < 8; k++)
            {
                d_cache[i][j].value.value[k] = 0;
            }
        }
    }
}

cache_info_t calculate_cache_info(uint32_t addr,
                                  uint32_t word_size,
                                  uint32_t cache_size,
                                  uint32_t cache_ways,
                                  uint32_t cache_block_size)
{
    cache_info_t cache_info;

    uint32_t tag_width    = uint32_t(word_size - uint32_t(log2(cache_size / cache_ways)));
    uint32_t index_width  = uint32_t(log2(cache_size / (cache_ways * cache_block_size)));
    uint32_t offset_width = uint32_t(log2(cache_block_size));

    uint32_t tag_bit_shift   = uint32_t(log2(cache_size/cache_ways));
    uint32_t index_bit_shift = log2(cache_block_size);

    cache_info.tags =   (addr  >> tag_bit_shift)   & BIT_MASK(tag_width);
    cache_info.index =  (addr  >> index_bit_shift) & BIT_MASK(index_width);
    cache_info.offset = (addr & BIT_MASK(offset_width))/WORD;
    cache_info.tag_bit_shift = tag_bit_shift;
    cache_info.index_bit_shift = index_bit_shift;

    return cache_info;
}
