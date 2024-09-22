#pragma once
#include "dram_typedef.hpp"
#include "bank.hpp"
#include <vector>

extern std::vector<l2_mem_request_t> req_queue;

void init_controller();

// send a commands on to the channel
void sends_cmd_to_channel(uint32_t addr,uint32_t _block, req_op_type read);

// issue request, scans the queue, perform fr-fcfs algorithm and returns the request, also reorder the queue
l2_mem_request_t issue_request();

// check if cmd is issuable, checks if channel is busy and the target bank is busy
bool is_cmd_issuable(uint32_t addr, req_op_type read);

// check if row buffer hit
bool is_row_buffer_hit(uint32_t addr);

void mark_channel_busy();

bool send_fill_req_to_l2();

void display_controller();
// update controller status
void update_controller_status();
