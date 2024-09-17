#include "cache.hpp"
#include <assert.h>

decoded_addr_info_t decode_addr(uint32_t addr,
                                  uint32_t word_size,
                                  uint32_t cache_size,
                                  uint32_t cache_ways,
                                  uint32_t cache_block_size)
{
    decoded_addr_info_t cache_info;

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

    cache_info.block_addr = addr/cache_block_size;
    cache_info.word_addr = addr/WORD;

    return cache_info;
}
