#pragma once
#include <iostream>
#include <array>
#include <algorithm>
#include <assert.h>
#include "cache.hpp"

#define TEST_L2_CACHE

extern fill_request_t dram_req_to_l2;
extern memory_request_t l2_req_to_dram;
extern uint32_t l2_cache_stall_cycles;

void init_L2_cache();
cache_block L1_cache_access_L2_cache(memory_request_t L1_req);
// Invalidates MSHR entry & updates L2 cache, returns a if it is time to fill
bool DRAM_fill_notification(fill_request_t dram_fill_req);
void send_req_to_dram(memory_request_t req);

void update_L2_cache_states();