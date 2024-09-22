#include "channel.hpp"
#include <cstdint>
#include <iostream>

void channel_init()
{
    data_block_t _block;
    _channel.status = CHANNEL_IDLE;
    _channel.command_bus = 0;
    _channel.addr_bus = 0;
    _channel.channel_stall_cycles = 0;
    _channel.data_bus = _block;
}

bool channel_is_busy()
{
    return _channel.status == CHANNEL_BUSY;
}

void clear_channel()
{
    _channel.status = CHANNEL_IDLE;
    _channel.command_bus = 0;
    _channel.addr_bus = 0;
}

void update_channel_status()
{
    // decrements the channel stall cycles then returns
    if (_channel.channel_stall_cycles > 0)
    {
        _channel.channel_stall_cycles--;
        return;
    }
    else if (_channel.channel_stall_cycles == 0)
    {
        clear_channel();
    }

    return;
}

void display_channel()
{
    // display the channel status, data and command
    std::cout << "======================================" << std::endl;
    std::cout << "Channel Status: " << _channel.status << std::endl;
    std::cout << "Channel Command: " << _channel.command_bus << std::endl;
    std::cout << "Channel Address: " << _channel.addr_bus << std::endl;
    std::cout << "Channel Stall Cycles: " << _channel.channel_stall_cycles << std::endl;
    std::cout << "======================================" << std::endl;
}