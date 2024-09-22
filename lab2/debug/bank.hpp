#pragma once
#include <array>
#include "dram_typedef.hpp"

// bank initialization
void bank_init();

// read a block from a bank row buffer
data_block_t bank_read(const uint32_t addr);

// We writes a cache line into a row, not a whole row!!
void bank_write(const uint32_t addr, const data_block_t _block);

// activates a bank, load the row from bank into row buffer
void bank_activate(const uint32_t addr);

// precharge a bank's row,writes the row buffer back to the bank's row
void bank_precharge(const uint32_t addr);

// receive a command from the memory controller, and locks the bank for 100 cycles
void bank_receive_cmd_delay(const uint32_t addr);

void bank_send_cmd_to_channel(const uint32_t addr, const uint32_t _block, req_op_type read);

row_buffer_status_t bank_get_row_buffer_status(const uint32_t addr);

// decode addr
bank_decoded_addr_t bank_decode_addr(const uint32_t addr);

//check if bank is busy
bool bank_is_busy(uint32_t addr);

void display_bank();

// Updates bank status
void update_bank_status();