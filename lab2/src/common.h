#ifndef _CACHE_H_
#define _CACHE_H_

// Notice this is included in everyfile, thus for the standard library that you
// need to use Simply include this file and add it here
#include <stdbool.h>
#include <stdint.h>

#include "debug.h"
#include "list.h"

// common.h stores the information that is widely adopted in all memory hiearchy
// the architecutural states of each memory hierarchy are defined in this file

/* cache block size is 32 byte across the memory hierarchy */
#define CACHE_BLOCK_SIZE 32
// The mask is used to align the address to the block size
#define CACHE_BLOCK_MASK (CACHE_BLOCK_SIZE - 1)
// The aligned address is used to align the address to the block size
#define CACHE_BLOCK_ALIGNED_ADDR(ADDR) ((ADDR) & ~CACHE_BLOCK_MASK)

// Defines the functional declarations for the memory hierarchy, these are
// access everywhere and must be propogated throughout the memory system
typedef struct L1_Cache_State L1_Cache_State;
typedef struct L2_Cache_State L2_Cache_State;
typedef struct Memory_State Memory_State;

/* generic cache block to passed around the memory hierarchy */
typedef struct Cache_Block {
  /* addr saved in this cache block */
  uint32_t tag;
  /* ptr to data or instruction cache */
  L1_Cache_State *l1;
} Cache_Block;

#endif
