#pragma once

#include <iostream>
#include <array>
#include <algorithm>
#include <assert.h>
#include "cache.hpp"

#define L2_CACHE_SIZE 256 * KILO
#define L2_CACHE_BLOCK_SIZE 32
#define L2_CACHE_WAYS 16

#define L2_CACHE_TAG_WIDTH WORD_SIZE - log2(L2_CACHE_SIZE / L2_CACHE_WAYS)
#define L2_CACHE_INDEX_WIDTH log2((L2_CACHE_SIZE / (L2_CACHE_WAYS * L2_CACHE_BLOCK_SIZE)))
#define L2_CACHE_OFFSET_WIDTH log2(L2_CACHE_BLOCK_SIZE)
#define L2_CACHE_WORDS_IN_BLOCK L2_CACHE_BLOCK_SIZE / WORD

#define L2_CACHE_SETS L2_CACHE_SIZE / (L2_CACHE_BLOCK_SIZE * L2_CACHE_WAYS)

// L2 cache
extern std::array<std::array<cache_block, L2_CACHE_WAYS>, L2_CACHE_SETS> l2_cache_mem;

// initialize l2 cache
void init_l2_cache();

// l2 cache hit or miss
hit_or_miss cache_hit_or_miss(const uint32_t tag, const uint32_t index);

// write l2 cache mem
void write_l2_cache_mem(const uint32_t tag, const uint32_t index, const uint32_t offset, const uint32_t data);

// read l2 cache
uint32_t read_l2_cache_mem(const uint32_t tag, const uint32_t index, const uint32_t offset);

// update l2 cache lru
void update_l2_cache_lru(const uint32_t tag, const uint32_t index);

uint32_t read_mem(const req_cache _req_cache,const uint32_t addr);

cache_block read_block_from_mem(const req_cache _req_cache,const cache_block selected_block, const uint32_t tag, const uint32_t index);


// instantiate memory
extern std::array<uint32_t,MEM_WORD_SIZE> test_inst_mem;
extern std::array<uint32_t,MEM_WORD_SIZE> test_data_mem;
extern std::array<uint32_t,MEM_WORD_SIZE> test_pseudo_data_mem;
