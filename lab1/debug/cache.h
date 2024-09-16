#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>

// #define DEBUG

#define BYTE 4
#define WORD 4
#define BLOCK 8
#define BITS 1

#define MEM_WORD_SIZE 1 << 20
#define WORD_SIZE 32 * BITS

#define KILO 1024
#define MEGA 1024 * 1024

#define I_CACHE_SIZE 8 * KILO
#define I_CACHE_BLOCK_SIZE 32
#define I_CACHE_WAYS 4

#define I_CACHE_TAG_WIDTH WORD_SIZE - log2(I_CACHE_SIZE / I_CACHE_WAYS)
#define I_CACHE_INDEX_WIDTH log2((I_CACHE_SIZE / (I_CACHE_WAYS * I_CACHE_BLOCK_SIZE)))
#define I_CACHE_OFFSET_WIDTH log2(I_CACHE_BLOCK_SIZE)
#define I_CACHE_WORDS_IN_BLOCK I_CACHE_BLOCK_SIZE / WORD
#define I_CACHE_SETS I_CACHE_SIZE / (I_CACHE_BLOCK_SIZE * I_CACHE_WAYS)

#define D_CACHE_SIZE 64 * KILO
#define D_CACHE_BLOCK_SIZE 32
#define D_CACHE_WAYS 8

#define D_CACHE_TAG_WIDTH WORD_SIZE - log2(D_CACHE_SIZE / D_CACHE_WAYS)
#define D_CACHE_INDEX_WIDTH log2((D_CACHE_SIZE / (D_CACHE_WAYS * D_CACHE_BLOCK_SIZE)))
#define D_CACHE_OFFSET_WIDTH log2(D_CACHE_BLOCK_SIZE)
#define D_CACHE_WORDS_IN_BLOCK D_CACHE_BLOCK_SIZE / WORD
#define D_CACHE_SETS D_CACHE_SIZE / (D_CACHE_BLOCK_SIZE * D_CACHE_WAYS)

#define L2_CACHE_SIZE 256 * KILO
#define L2_CACHE_BLOCK_SIZE 32
#define L2_CACHE_WAYS 16

#define L2_CACHE_TAG_WIDTH WORD_SIZE - log2(L2_CACHE_SIZE / L2_CACHE_WAYS)
#define L2_CACHE_INDEX_WIDTH log2((L2_CACHE_SIZE / (L2_CACHE_WAYS * L2_CACHE_BLOCK_SIZE)))
#define L2_CACHE_OFFSET_WIDTH log2(L2_CACHE_BLOCK_SIZE)
#define L2_CACHE_WORDS_IN_BLOCK L2_CACHE_BLOCK_SIZE / WORD

#define L2_CACHE_SETS L2_CACHE_SIZE / (L2_CACHE_BLOCK_SIZE * L2_CACHE_WAYS)

// use unsigned int for the bit mask
#define BIT_MASK(size) ((1U << (unsigned)(size)) - 1)

// use enum to define caches status, rd/wr of l1d of l1i
enum cache_status
{
    L1D_RD,
    L1D_WR,
    L1I_RD
};

// Define d cache key, key = [dirty,valid,lru_cnt,tag] as struct
typedef struct cache_key
{
    int dirty;
    int valid;
    int lru_cnt;
    u_int32_t tag;
} cache_key_t;

typedef struct cache_data_block
{
    u_int32_t value[D_CACHE_WORDS_IN_BLOCK];
} cache_data_block_t;

// Define cache block, as key and value array
typedef struct cache_block
{
    cache_key_t key;
    // Each cache block is 32 bytes, number of sets
    cache_data_block_t value;
} cache_block;

// memory request
typedef struct memory_request
{
    uint32_t addr;
    uint32_t data;
    uint32_t cycle_time;
    enum cache_status status;
} memory_request_t, fill_request_t;

// Instantiate i_cache, i_cache is a 4-way associative cache with 64 sets
extern cache_block i_cache_mem[I_CACHE_SETS][I_CACHE_WAYS];

// Instantiate d_cache, d_cache is a 8-way associative cache with 256 sets
//  This is useful for sharing variables within the system
extern cache_block d_cache_mem[D_CACHE_SETS][D_CACHE_WAYS];

// Define the l2_cache_mem
extern cache_block l2_cache_mem[L2_CACHE_SETS][L2_CACHE_WAYS];

typedef struct cache_return_data
{
    uint32_t value;
    uint32_t cycle_time;
} cache_return_data_t;

typedef struct cache_return_data_l2
{
    fill_request_t fill_l1_req;
    memory_request_t access_to_dram_req;
} cache_return_data_l2_t;

enum req_type
{
    READ,
    WRITE
};

cache_return_data_t i_cache(const u_int32_t addr, const u_int32_t cycle_time);                                 // Returns a value and updates the i cache
cache_return_data_t d_cache(const bool read, const uint32_t data, const u_int32_t addr, const u_int32_t time); // Returns a value and updates the d cache

typedef struct cache_info
{
    uint32_t tags;
    uint32_t index;
    uint32_t offset;
    uint32_t tag_bit_shift;
    uint32_t index_bit_shift;
} cache_info_t;

// Initialize the instruction memory
extern uint32_t inst_mem[MEM_WORD_SIZE];

extern uint32_t data_mem[MEM_WORD_SIZE];

extern uint32_t pseudo_data_mem[MEM_WORD_SIZE];

void init_cache(); // Initializes the cache

cache_info_t calculate_cache_info(uint32_t addr, uint32_t word_size, uint32_t cache_size, uint32_t cache_ways, uint32_t cache_block_size);

req_type get_req_type(enum cache_status status);

#endif
