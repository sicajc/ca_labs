#pragma once
#include "cache.hpp"

#define NUM_BANKS 8
#define NUM_OF_ROWS 64 * KILO
#define ROW_SIZE 8 * KILO

#define CACHE_LINE_SIZE 32
#define NO_OF_LINE_IN_ROW ROW_SIZE / CACHE_LINE_SIZE

#define BANK_BUSY_LATENCY 100
#define MEM_REQ_CHANNEL_ULTILIZE_LATENCY 4
#define RD_WR_CHANNEL_ULTILIZE_LATENCY 50

typedef struct data_block
{
    std::array<u_int32_t, CACHE_LINE_SIZE / WORD> words;

    // Initialize the data block
    data_block()
    {
        for (int i = 0; i < CACHE_LINE_SIZE / WORD; i++)
        {
            words[i] = 0;
        }
    }
} data_block_t;

typedef struct l2_mem_request
{
    uint32_t addr;
    data_block_t data_block;
    req_op_type read;
    uint32_t cycle_time;
    bool valid;
    enum req_cache req_cache_type;
} l2_mem_request_t;


typedef enum dram_cmds
{
    ACT,
    PRE,
    RD,
    WR
} dram_cmds_t;

// Type definition for bank status
typedef enum bank_status
{
    BANK_IDLE,
    BANK_BUSY
} bank_status_t;

typedef enum row_buffer_status
{
    ROW_BUFFER_HIT,
    ROW_BUFFER_MISS,
    ROW_BUFFER_CONFLICT
} row_buffer_status_t;


typedef struct bank_row
{
    std::array<data_block_t, NO_OF_LINE_IN_ROW> lines;

    // Initialize the bank row
    bank_row()
    {
        for (int i = 0; i < NO_OF_LINE_IN_ROW; i++)
        {
            lines[i] = data_block_t();
        }
    }
} bank_row_t;

typedef struct bank_row_buffer
{
    bank_row_t row_buffer;
    uint32_t row_id;
    bool valid;

    // Initialize the bank row buffer
    bank_row_buffer()
    {
        row_id = 0;
        valid = false;
    }
} bank_row_buffer_t;

typedef enum bank_op_type
{
    BANK_NONE,
    BANK_PRE,
    BANK_ACT,
    BANK_RD,
    BANK_WR
} bank_op_type_t;

typedef struct bank
{
    uint32_t bank_stall_cycles = 0;

    bank_status_t status = BANK_IDLE;

    bank_op_type_t bank_op;

    bank_row_buffer_t row_buffer;

    std::array<bank_row, NUM_OF_ROWS> rows;

    // Initialize the bank
    bank()
    {
        status = BANK_IDLE;
        bank_op = BANK_NONE;
        row_buffer = bank_row_buffer_t();
        for (int i = 0; i < NUM_OF_ROWS; i++)
        {
            rows[i] = bank_row();
        }
    }

    // reset the bank
    void reset()
    {
        status = BANK_IDLE;
        row_buffer = bank_row_buffer_t();
        for (int i = 0; i < NUM_OF_ROWS; i++)
        {
            rows[i] = bank_row();
        }
    }
} bank_t;

typedef struct controller_to_channel_req
{
    bool valid;
    uint32_t addr;
    uint32_t data;
    req_op_type read;
} ctr_to_channel_req_t;


typedef struct bank_decoded_addr
{
    uint32_t bank_id;
    uint32_t row_id;
    uint32_t col_id;
} bank_decoded_addr_t;

// Banks
extern bank_t banks[NUM_BANKS];
extern ctr_to_channel_req_t ctr_to_channel_req;
