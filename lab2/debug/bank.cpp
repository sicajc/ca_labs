#include "bank.hpp"
#include "dram_typedef.hpp"
#include "channel.hpp"
#include <cstdint>
#include <iostream>

// Bank functions, inputs are the data, address and commands from the channels
// Bank receiving commands locks the bank for another 100 cycles
// Returns the data, address and commands to the channels, ultilize for 50 cycles

channel_t _channel;
void bank_init()
{
    for (int i = 0; i < NUM_BANKS; i++)
    {
        banks[i] = bank_t();
    }
}

bank_decoded_addr_t bank_decode_addr(const uint32_t addr)
{
    bank_decoded_addr_t decoded_addr;
    // 3 bits bank id, 16 bits row id,8 bits col id
    decoded_addr.bank_id = (addr >> 5) & 0x7;
    decoded_addr.row_id = (addr >> 16) & 0xFFFF;
    decoded_addr.col_id = (addr >> 8) & 0xFF;
    return decoded_addr;
}

data_block_t bank_read(const uint32_t addr)
{
    // decode addr
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    // read the bank's row buffer
    return banks[decoded_addr.bank_id].row_buffer.row_buffer.lines[decoded_addr.col_id];
}

void bank_write(const uint32_t addr, const data_block_t _block)
{
    // decode addr
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    // write to the bank's row buffer
    banks[decoded_addr.bank_id].row_buffer.row_buffer.lines[decoded_addr.col_id] = _block;
}

void bank_activate(const uint32_t addr)
{
    // decode addr
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    // load the row from bank into row buffer
    banks[decoded_addr.bank_id].row_buffer.row_buffer = banks[decoded_addr.bank_id].rows[decoded_addr.row_id];
    banks[decoded_addr.bank_id].row_buffer.row_id = decoded_addr.row_id;
    banks[decoded_addr.bank_id].row_buffer.valid = true;
}

void bank_receive_cmd_delay(const uint32_t addr)
{
    // decode addr
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    // set the bank status to busy
    banks[decoded_addr.bank_id].status = BANK_BUSY;
}

row_buffer_status_t bank_get_row_buffer_status(const uint32_t addr)
{
    // decode addr
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    // check if the row buffer is valid
    if (banks[decoded_addr.bank_id].row_buffer.valid)
    {
        // check if the row buffer is hit
        if (banks[decoded_addr.bank_id].row_buffer.row_id == decoded_addr.row_id)
        {
            return ROW_BUFFER_HIT;
        }
        else
        {
            return ROW_BUFFER_CONFLICT;
        }
    }
    else
    {
        return ROW_BUFFER_MISS;
    }
}

void bank_precharge(const uint32_t addr)
{
    // decode addr
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    // write the row buffer back to the row
    banks[decoded_addr.bank_id].rows[banks[decoded_addr.bank_id].row_buffer.row_id] = banks[decoded_addr.bank_id].row_buffer.row_buffer;

    // reset the row buffer
    banks[decoded_addr.bank_id].row_buffer.valid = false;
    banks[decoded_addr.bank_id].row_buffer.row_id = 0;
}

bool bank_is_busy(uint32_t addr)
{
    // decode addr
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    // check if the bank is busy
    if (banks[decoded_addr.bank_id].status == BANK_BUSY)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Updates the status of the inner bank, also there stall conditions
void update_bank_status()
{
    // decrement the stall cycles of every bank if it is greater than 0
    for (int i = 0; i < NUM_BANKS; i++)
    {
        if (banks[i].bank_stall_cycles > 0)
        {
            banks[i].bank_stall_cycles--;
        }
        else
        {
            banks[i].status = BANK_IDLE;
        }
    }

    // read from the channel
    uint32_t command = _channel.command_bus;

    // decode the command
    uint32_t addr = _channel.addr_bus;

    // decode addr
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    // check if the bank is busy simply returns
    if (~bank_is_busy(addr))
    {
        // issue the cmds to the bank
        // Issue operations according to the command from the channel
        switch (command)
        {
        case ACT:
            bank_activate(addr);
            banks[decoded_addr.bank_id].bank_stall_cycles = BANK_BUSY_LATENCY;
            // updates bank op type
            banks[decoded_addr.bank_id].bank_op = BANK_ACT;
            break;
        case PRE:
            bank_precharge(addr);
            banks[decoded_addr.bank_id].bank_stall_cycles = BANK_BUSY_LATENCY;
            // updates bank op type
            banks[decoded_addr.bank_id].bank_op = BANK_PRE;
            break;
        case RD:
            bank_read(addr);
            banks[decoded_addr.bank_id].bank_stall_cycles = BANK_BUSY_LATENCY;
            // updates bank op type
            banks[decoded_addr.bank_id].bank_op = BANK_RD;
            break;
        case WR:
            bank_write(addr, _channel.data_bus);
            banks[decoded_addr.bank_id].bank_stall_cycles = BANK_BUSY_LATENCY;
            // updates bank op type
            banks[decoded_addr.bank_id].bank_op = BANK_WR;
            break;
        default:
            std::cerr << "Invalid command" << std::endl;
            break;
        }
    }

    // check if the channel is busy
    if (_channel.status == CHANNEL_ISSUE_CMD || _channel.status == CHANNEL_RD_WR)
    {
        return;
    }

    // if channel is not busy, checks which bank wants to issue rd wr commands
    for (int i = 0; i < NUM_BANKS; i++)
    {
        if ((banks[i].bank_op == BANK_RD || banks[i].bank_op == BANK_WR) && banks[i].bank_stall_cycles == 0)
        {
            // read from bank's row buffer
            data_block_t data_block = bank_read(addr);

            // send the command to the channel
            _channel.status = CHANNEL_RD_WR;
            _channel.addr_bus = addr;
            _channel.command_bus = banks[i].bank_op == BANK_RD ? RD : WR;
            _channel.data_bus = data_block;
            _channel.channel_stall_cycles = RD_WR_CHANNEL_ULTILIZE_LATENCY;

            // reset the bank op type
            banks[i].bank_op = BANK_NONE;
            banks[i].status = BANK_IDLE;

            return;
        }
    }

    return;
}

void display_bank()
{
    // display the bank status
    std::cout << "======================================" << std::endl;
    for (int i = 0; i < NUM_BANKS; i++)
    {
        std::cout << "Bank " << i << " status: " << banks[i].status << std::endl;
        std::cout << "Bank " << i << " stall cycles: " << banks[i].bank_stall_cycles << std::endl;
        // row buffer status
        std::cout << "Bank " << i << " row buffer status: " << banks[i].row_buffer.valid << std::endl;
        std::cout << "Bank " << i << " row buffer row id: " << banks[i].row_buffer.row_id << std::endl;
    }
    std::cout << "======================================" << std::endl;
}
