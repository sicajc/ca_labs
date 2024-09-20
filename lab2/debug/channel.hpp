#include "cache.h"
#include "bank.hpp"

// Type definition channel status
typedef enum channel_status
{
    CHANNEL_IDLE,
    CHANNEL_BUSY
} channel_status_t;

// Type definition for channel status
typedef struct channel
{
    channel_status_t status;
    uint32_t command_bus;
    uint32_t addr_bus;
    data_block data_bus;

    // initialize channel
    channel()
    {
        status = CHANNEL_IDLE;
        command_bus = 0;
        addr_bus = 0;
    }
} channel_t;


// Channel functions
// Initialize the channel
void channel_init();

// check channel status
bool channel_is_busy();

// send a command to the channel
void send_command_to_channel(uint32_t command, uint32_t addr);