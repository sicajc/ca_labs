#include "mshr.hpp"

std::array<mshr_entry_t, NO_OF_MSHR_ENTRIES> mshr;

void init_mshr()
{
    for (auto &entry : mshr)
    {
        entry.valid = false;
        entry.done = false;
        entry.block_addr = 0;
        entry.status = L1D_RD;
    }
}

void add_request_to_mshr(uint32_t block_addr, enum cache_status status)
{
    auto entry = std::find_if(mshr.begin(), mshr.end(),
                              [block_addr](const mshr_entry_t &entry)
                              {
                                  return entry.block_addr == block_addr;
                              });

    entry->block_addr = block_addr;
    entry->status = status;
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

bool is_request_in_mshr(uint32_t block_addr)
{
    return std::any_of(mshr.begin(), mshr.end(),
                       [block_addr](const mshr_entry_t &entry)
                       {
                           return entry.block_addr == block_addr;
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
