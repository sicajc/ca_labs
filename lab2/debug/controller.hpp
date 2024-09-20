#include "cache.hpp"
#include "bank.hpp"
#include <vector>

typedef struct l2_mem_request
{
    uint32_t addr;
    uint32_t data;
    req_op_type read;
    uint32_t cycle_time;
} l2_mem_request_t;

typedef enum dram_cmds
{
    ACT,
    PRE,
    RD,
    WR
} dram_cmds_t;

extern std::vector<l2_mem_request_t> req_queue;

// check row buffer status
bool bank_is_row_buffer_hits(uint32_t addr);

// check bank status
bool bank_is_busy(uint32_t addr);

// send a commands to the bank
dram_cmds_t send_command_to_bank(uint32_t addr, req_op_type read, uint32_t data, uint32_t cycle_time);

// issue request, scans the queue, perform fr-fcfs algorithm and returns the request, also reorder the queue
l2_mem_request_t issue_request();
