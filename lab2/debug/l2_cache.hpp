#pragma once
#include <iostream>
#include <array>
#include <algorithm>
#include <assert.h>
#include "cache.hpp"

void init_L2_cache();
memory_request_t L1_cache_access_L2_cache(memory_request_t L1_req);
void L2_cache_access_dram();
void L2_fill_notification();
void DRAM_fill_notification(fill_request_t dram_fill_req);