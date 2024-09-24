#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "l1_cache.h"

static int bit_length(uint32_t n) {
  uint32_t l = 0;

  while (n >>= 1)
    ++l;

  return l;
}

// Initialization of l1 cache, label indicates it is i$ or d$
// total_size is the total size of the cache
// num_ways is the number of ways
// i is the interconnection state
void l1_cache_init(L1_Cache_State *c, char *label, int total_size, int num_ways,
                   Interconnect_State *i) {

  c->label = label;
  c->total_size = total_size;
  c->num_ways = num_ways;

  c->num_sets = (c->total_size / c->num_ways) / CACHE_BLOCK_SIZE;
  c->set_idx_from = bit_length(CACHE_BLOCK_SIZE);

  if (c->num_sets == 1) {
    c->set_idx_to = c->set_idx_from;
  } else {
    c->set_idx_to = c->set_idx_from + bit_length(c->num_sets) - 1;
  }

  /* init sets*ways cache blocks */
  c->blocks = (L1_Cache_Block *)calloc(c->num_sets * c->num_ways,
                                       sizeof(L1_Cache_Block));

  // timestamp is used to determine the recency of the cache block
  c->timestamp = 0;
  // set the pointer of interconnection it used to the interconnection for
  // accessing the memory hierarchy
  c->interconnect = i;
}

void l1_cache_free(L1_Cache_State *c) { free(c->blocks); }

/*
  Notice that static usually models the needed helper functions for the main
  current functions, which only the current file needs to access. This is a
  common pattern in C programming. To restrict the scope of the functions to the
  current file. Which prevent naming error and conflicts.
*/
static uint32_t get_set_idx(L1_Cache_State *c, uint32_t addr) {
  // This gets the index of the set for memory access, from accessing the
  // information from the current l1$ state
  uint32_t mask = ~0;
  mask >>= 32 - (c->set_idx_to + 1);

  uint32_t set_idx = (addr & mask);
  set_idx >>= c->set_idx_from;

  return set_idx;
}

static void write_block(L1_Cache_Block *block, uint32_t tag, int timestamp) {
  // This updates the block with the tag and timestamp to ensure the recency of
  // the block
  block->valid = true;
  block->tag = tag;
  // usage of timestamp to determine the recency of the block, this is updated
  // whenever the $ is being accessed
  block->last_access = timestamp;
}

// Only returns miss or hit, the actual data is not stored in the cache block
// This is a simulation of the cache access, the actual data is being handled by
// other functions
Cache_Result l1_cache_access(L1_Cache_State *c, uint32_t addr) {
  /* increase timestamp for recency updates everytime l1 $ gets accessed*/
  c->timestamp++;

  /* calculate set idx */
  uint32_t tag = CACHE_BLOCK_ALIGNED_ADDR(addr);
  uint32_t set_idx = get_set_idx(c, addr);
  // set pointer is calculated by adding the base address of the blocks + the
  // offset which is from the set index* number of ways
  L1_Cache_Block *set =
      c->blocks +
      set_idx * c->num_ways; // address translation for the set pointer address
  L1_Cache_Block *block;

  /* check if addr is in cache ,if not it is a cache miss*/
  for (int way = 0; way < c->num_ways; ++way) {
    block = set + way;
    if (block->valid && (block->tag == tag)) {
      // the use of debug is to print the information of the cache access, this
      // is used for debugging This helps printing out vital information about
      // cache hit for tracing.
      debug_l1("%s: [0x%X] HIT in set %d way %d\n", c->label, tag, set_idx,
               way);
      // the block is updated with the tag and timestamp to ensure the recency
      // of the block
      write_block(block, tag, c->timestamp);
      return CACHE_HIT;
    }
  }

  // important event to print out the cache miss
  debug_l1("%s: [0x%X] MISS in set %d\n", c->label, tag, set_idx);

  /* addr not in cache -> probe L2 cache */
  Cache_Block *b = (Cache_Block *)malloc(sizeof(Cache_Block));
  b->tag = tag;
  b->l1 = c;

  // Probing l2 cache through interconnections
  interconnect_l1_to_l2(c->interconnect, b);

  return CACHE_MISS;
}

// Inserts a cache block into l1$, the $block pointer is being passed in
// later being transformed into a L1_Cache_Block, b is then being freed
void l1_insert_block(struct Cache_Block *b) {
  // b is a pointer to the $ block from either d$ or i$
  L1_Cache_State *c = b->l1;

  // $ is accessed
  c->timestamp++;

  uint32_t tag = b->tag;
  uint32_t set_idx = get_set_idx(c, tag);
  L1_Cache_Block *set = c->blocks + set_idx * c->num_ways;
  L1_Cache_Block *block; // A pointer to the $ block

  /* check that addr is not already in cache */
  for (int way = 0; way < c->num_ways; ++way) {
    block = set + way;
    if (block->valid && (block->tag == tag)) {
      /* if block is already in cache don't insert it again */
      goto out;
    }
  }

  /* try to insert into invalid block */
  for (int way = 0; way < c->num_ways; ++way) {
    block = set + way;
    if (!block->valid) {
      debug_l1("%s: [0x%X] inserted (invalid) into set %d way %d\n", c->label,
               tag, set_idx, way);
      write_block(block, tag, c->timestamp);
      goto out;
    }
  }

  /* no invalid block -> evict LRU block */
  block = set + 0; // initiated the block pointer to the start of the set.
  for (int way = 1; way < c->num_ways; ++way) {
    // Traverse throught the set using pointer arithemtic to find the least
    // recently used block
    L1_Cache_Block *temp_block = set + way;
    // last access is used to determine the recency of the block, the block with
    // the least recency is evicted
    if (block->last_access > temp_block->last_access) {
      block = temp_block;
    }
  }

  debug_l1("%s: [0x%X] inserted (lru) in set %d way %d\n", c->label, tag,
           set_idx, (int)(block - set));

  // the block is updated with the tag and timestamp to ensure the recency of
  // the block tag and recency must be kept
  write_block(block, tag, c->timestamp);

out:
  /* always free cache block */
  free(b);
}

// This is used when the $ access @ fetch state gets flushed, this is used to
// cancel the access
void l1_cancel_cache_access(L1_Cache_State *c, uint32_t addr) {
  // Cancel the cache access, find the block with the tag and cancel the access
  Cache_Block b;
  b.tag = CACHE_BLOCK_ALIGNED_ADDR(addr);
  b.l1 = c;

  // this cancel statement must also be called in the interconnection
  interconnect_l1_to_l2_cancel(c->interconnect, &b);
}
