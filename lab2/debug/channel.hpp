#include "dram_typedef.hpp"
#include "bank.hpp"
#include <cstdint>

// Type definition channel status
typedef enum channel_status
{
    CHANNEL_IDLE,
    CHANNEL_ISSUE_CMD,
    CHANNEL_RD_WR
} channel_status_t;

// Type definition for channel status
typedef struct channel
{
    channel_status_t status;
    uint32_t command_bus;
    uint32_t addr_bus;
    uint32_t channel_stall_cycles = 0;
    data_block data_bus;

    // initialize channel
    channel()
    {
        status = CHANNEL_IDLE;
        command_bus = 0;
        addr_bus = 0;
    }
} channel_t;

extern channel_t _channel;

// Channel functions
// Initialize the channel
void channel_init();

// check channel status
bool channel_is_busy();

// send a command to the channel
void send_command_to_channel(uint32_t command, data_block_t _block,uint32_t addr);

// clear the channel
void clear_channel();

// display commands on channel
void display_channel();

//updates cycle channel status
void update_channel_status();