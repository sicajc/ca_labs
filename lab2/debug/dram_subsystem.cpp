#include "dram_subsystem.hpp"
void dram_subsystem_init()
{
    init_controller();
    channel_init();
    bank_init();
}

void dram_subsystem_update()
{
    update_bank_status();
    update_channel_status();
    update_controller_status();
    dram_cycles++;
}

void dram_subsystem_display()
{
    // display_controller();
    display_channel();
    // display_bank();
}