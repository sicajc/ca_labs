#pragma once
#include <iostream>
#include <array>
#include <algorithm>
#include <assert.h>
#include "cache.hpp"

#define TEST_L2_CACHE

void init_L2_cache();
cache_block L1_cache_access_L2_cache(memory_request_t L1_req);
// Invalidates MSHR entry & updates L2 cache, returns a if it is time to fill
bool DRAM_fill_notification(fill_request_t dram_fill_req);

void update_L2_cache();