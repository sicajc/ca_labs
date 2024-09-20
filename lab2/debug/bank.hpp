#pragma once

#include "cache.hpp"
#include <array>

#define NUM_BANKS 8
#define NUM_OF_ROWS 64 * KILO
#define ROW_SIZE 8 * KILO

#define CACHE_LINE_SIZE 32
#define NO_OF_LINE_IN_ROW ROW_SIZE / CACHE_LINE_SIZE

#define BANK_BUSY_LATENCY 100

// Type definition for bank status
typedef enum bank_status
{
    BANK_IDLE,
    BANK_BUSY
} bank_status_t;

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

typedef struct bank
{
    bank_status_t status = BANK_IDLE;

    bank_row_buffer_t row_buffer;

    std::array<bank_row, NUM_OF_ROWS> rows;

    // Initialize the bank
    bank()
    {
        status = BANK_IDLE;
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

typedef struct bank_decoded_addr
{
    uint32_t bank_id;
    uint32_t row_id;
    uint32_t col_id;
} bank_decoded_addr_t;

// Bank functions
extern bank_t banks[NUM_BANKS];

// bank initialization
void bank_init();

// read from a bank row
bank_row_t bank_read(uint32_t addr);

// write to a bank row
void bank_write(uint32_t addr, bank_row_t row);

// activates a bank
void bank_activate(uint32_t addr);

//precharge a bank
void bank_precharge(uint32_t addr);

//decode addr
bank_decoded_addr_t bank_decode_addr(uint32_t addr);
