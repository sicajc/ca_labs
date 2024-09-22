#pragma once
#include <array>
#include "dram_typedef.hpp"
#include "controller.hpp"
#include "channel.hpp"
#include "bank.hpp"

//statistics
extern uint32_t dram_cycles;


// DRAM Subsystem functions
void dram_subsystem_init();
void dram_subsystem_update();
void dram_subsystem_display();
