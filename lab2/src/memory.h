#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "common.h"
#include "interconnect.h"

#define MEM_NUM_BANKS 8
#define MEM_ROW_ADDRESS_MASK ~((1 << 16) - 1)

#define MEM_NUM_CMD_INTERVALS 3
#define MEM_PRE_IDX 0
#define MEM_ACT_IDX 1
#define MEM_RW_IDX 2

typedef enum Memory_Row_Buffer_Status {
  // the possible status for row buffer
  MEM_ROW_BUFFER_HIT,
  MEM_ROW_BUFFER_MISS,
  MEM_ROW_BUFFER_CONFLICT
} Memory_Row_Buffer_Status;

typedef struct Memory_Interval {
  // This struct is used to store the start and end cycle of the memory request
  // on the command bus data bus and bank
  int start;
  int end;
  bool valid;
} Memory_Interval;

typedef struct Memory_Bank {
  // The memory bank is used to store the memory bank information
  // it consists of the row buffer, the row buffer status and the row buffer
  // open status
  uint32_t row_buffer;
  bool row_buffer_open;
} Memory_Bank;

/*
 * A memory request can be pending or in progress. Each request stores when it
 * accesses which resource (e.g. command bus). For a pending request these
 * values get calculated in every cycle and once the request get scheduled they
 * won't change.
 */
typedef struct Memory_Request {
  /* corresponding cache block */
  Cache_Block *cache_block;
  /* row index */
  uint32_t row;
  /* row buffer status */
  Memory_Row_Buffer_Status status;
  /* command/address bus usage */
  struct Memory_Interval cmd_ints[MEM_NUM_CMD_INTERVALS];
  /* data bus usage */
  struct Memory_Interval data_int;
  /* bank usage */
  Memory_Bank *bank;
  int bank_idx;
  struct Memory_Interval bank_int;
} Memory_Request;

struct Memory_State {
  /* memory banks */
  Memory_Bank banks[MEM_NUM_BANKS];
  /* current cycle */
  int curr_cycle;
  /* pending request queue */
  // This serves as the queue for the memory requests, since needs to perform fr-fcfs on the requests
  // This is being added to the pending request queue, and then being processed in the memory_cycle
  list_t *pending_requests;
  /* ongoing request queue */
  // This serves as the queue for the ongoing requests
  list_t *ongoing_requests;
  /* ptr to interconnect */
  Interconnect_State *interconnect;
};

/* init memory */
void memory_init(Memory_State *m, Interconnect_State *i);

/* free storage allocated by memory */
void memory_free(Memory_State *m);

/* add a memory request */
void memory_add_request(Memory_State *m, Cache_Block *b);

/* retire ongoing requests and schedule a pending request */
void memory_cycle(Memory_State *m);

#endif
