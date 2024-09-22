#include "channel.hpp"
#include "dram_typedef.hpp"
#include "l2_cache.hpp"
#include <cstdint>
#include <vector>
#include <iostream>

std::vector<l2_mem_request_t> req_queue;
std::vector<dram_cmds_t> dram_cmds_issue_queue;

void init_controller()
{
    // clear the queue
    req_queue.clear();
}

bool is_row_buffer_hit(uint32_t addr)
{
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);
    return banks[decoded_addr.bank_id].row_buffer.row_id == decoded_addr.row_id;
}

bool is_cmd_issueable(uint32_t addr, req_op_type read)
{
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);
    return !bank_is_busy(addr) && !channel_is_busy();
}

l2_mem_request_t issue_request()
{
    // First check if queue is empty, if empty sends nothing
    if (req_queue.empty())
    {
        return l2_mem_request_t();
    }

    // If not empty, if only 1 request , take it out,then scan the queue and perform fr-fcfs algorithm
    if (req_queue.size() == 1)
    {
        l2_mem_request_t req = req_queue.front();
        req_queue.pop_back();
        return req;
    }
    else
    {
        // scans the queue and access each request
        // check if the request is row-buffer hits
        // if it is, return the request and removes the req from queue
        // if not, return the first request
        // reoder the queue
        l2_mem_request_t req = req_queue.front();

        // search for row buffer hit commands first
        for (auto it = req_queue.begin(); it != req_queue.end(); it++)
        {
            if (is_row_buffer_hit(it->addr) == true && is_cmd_issueable(it->addr, it->read) == true)
            {
                req = *it;
                req_queue.erase(it);
                return req;
            }
        }

        // Then search for the req from memory stage(D_CACHE)
        for (auto it = req_queue.begin(); it != req_queue.end(); it++)
        {
            if (it->req_cache_type == D_CACHE && is_cmd_issueable(it->addr, it->read) == true)
            {
                req = *it;
                req_queue.erase(it);
                return req;
            }
        }

        return req;
    }
}

void send_command_to_channel(req_op_type _req_type, data_block _block, uint32_t addr)
{
    bank_decoded_addr_t decoded_addr = bank_decode_addr(addr);

    //  first check if channel is busy
    if (channel_is_busy())
    {
        return;
    }

    if (~dram_cmds_issue_queue.empty())
    {
        // take cmds from the front of queue vector
        dram_cmds_t cmd = dram_cmds_issue_queue.front();
        dram_cmds_issue_queue.erase(dram_cmds_issue_queue.begin());

        _channel.status = CHANNEL_ISSUE_CMD;
        _channel.addr_bus = addr;
        _channel.command_bus = cmd;
        _channel.data_bus = _block;
        _channel.channel_stall_cycles = MEM_REQ_CHANNEL_ULTILIZE_LATENCY;
        return;
    }

    // depends on the target bank states, send the command to the bank
    switch (bank_get_row_buffer_status(addr))
    {
    case ROW_BUFFER_HIT:
    {
        dram_cmds_issue_queue.push_back(_req_type == WR ? WR : RD);
        break;
    }
    case ROW_BUFFER_MISS:
    {
        dram_cmds_issue_queue.push_back(ACT);
        dram_cmds_issue_queue.push_back(_req_type == WR ? WR : RD);
        break;
    }
    case ROW_BUFFER_CONFLICT:
    {
        dram_cmds_issue_queue.push_back(PRE);
        dram_cmds_issue_queue.push_back(ACT);
        dram_cmds_issue_queue.push_back(_req_type == WR ? WR : RD);
        break;
    }
    default:
    {
        break;
    }
    }

    return;
}

bool send_fill_req_to_l2()
{
    // check if channel is in issue rd wr states
    if (_channel.status != CHANNEL_RD_WR)
    {
        return false;
    }

    // check channel stall time is not zero
    if (_channel.channel_stall_cycles > 0)
    {
        return false;
    }

    // check if command is rd
    if (_channel.command_bus == RD)
    {
        // send the data to the l2 cache
        return true;
    }
    else
    {

        return false;
    }
}

void update_controller()
{
    // see if there is req to send into l2
    send_fill_req_to_l2();

    // check if the request queue is empty
    if (req_queue.empty())
    {
        return;
    }

    // check if the channel is busy
    if (channel_is_busy())
    {
        return;
    }

    // issue the request
    l2_mem_request_t req = issue_request();

    // send the command to the channel
    send_command_to_channel(req.read, req.data_block, req.addr);

    // send fill rq to l2
    if (send_fill_req_to_l2())
    {
        fill_request_t dram_req;
        cache_block block;

        for (int i = 0; i < CACHE_LINE_SIZE / WORD; i++)
        {
            block.value.value[i] = _channel.data_bus.words[i];
        }

        dram_req.addr = req.addr;
        dram_req.req_cache_type = req.req_cache_type;
        dram_req.req_op_type = req.read;
        dram_req.cycle_time = req.cycle_time;
        dram_req.block = block;

        // sends the request to l2
        dram_req_to_l2 = dram_req;
    }

    return;
}

void display_controller()
{
    // display the controller status
    std::cout << "======================================" << std::endl;
    std::cout << "Controller Status: " << _channel.status << std::endl;
    std::cout << "Controller Command: " << _channel.command_bus << std::endl;
    std::cout << "Controller Address: " << _channel.addr_bus << std::endl;
    std::cout << "Controller Stall Cycles: " << _channel.channel_stall_cycles << std::endl;
    std::cout << "======================================" << std::endl;
}