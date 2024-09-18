#include "mshr.hpp"

std::array<mshr_entry_t, NO_OF_MSHR_ENTRIES> mshr;

void init_mshr()
{
    for (auto &entry : mshr)
    {
        entry.valid = false;
        entry.done = false;
        entry.block_addr = 0;
        entry.req_cache_type = I_CACHE;
    }
}

void add_request_to_mshr(uint32_t block_addr, req_cache _req_cache)
{
    auto entry = std::find_if(mshr.begin(), mshr.end(),
                              [](const mshr_entry_t &entry)
                              {
                                  return entry.valid == false;
                              });

    entry->block_addr = block_addr;
    entry->req_cache_type = _req_cache;
    entry->valid = true;
}

bool is_mshr_full()
{
    return std::all_of(mshr.begin(), mshr.end(),
                       [](const mshr_entry_t &entry)
                       {
                           return entry.valid;
                       });
}

bool is_request_in_mshr(uint32_t block_addr,req_cache _req_cache)
{
    return std::any_of(mshr.begin(), mshr.end(),
                       [block_addr,_req_cache](const mshr_entry_t &entry)
                       {
                           return entry.block_addr == block_addr && entry.valid == true && entry.req_cache_type == _req_cache;
                       });
}

void invalidate_mshr_entry(uint32_t block_addr)
{
    auto entry = std::find_if(mshr.begin(), mshr.end(),
                              [block_addr](const mshr_entry_t &entry)
                              {
                                  return entry.block_addr == block_addr;
                              });

    entry->valid = false;
    entry->done = true;
}
