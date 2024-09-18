#pragma once
#include "cache.hpp"
#include <iostream>
#include <array>
#include <algorithm>

// since we access the L2 cache in a block level manner, the block would be loaded back to L1 caches
// And it would sends the fill notification to the l1 $

#define NO_OF_MSHR_ENTRIES 16

extern std::array<mshr_entry_t, NO_OF_MSHR_ENTRIES> mshr;

void init_mshr();
// If the block address does not exist also the mshr is not full, then we can access the mshr
// L1 cache request accessing mshr and modify the mshr entry
void add_request_to_mshr(uint32_t block_addr, enum req_cache _req_cache);

bool is_mshr_full();
bool is_request_in_mshr(uint32_t block_addr,enum req_cache _req_cache);
// dram invalidates through a fill request
void invalidate_mshr_entry(uint32_t block_addr);