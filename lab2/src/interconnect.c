#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "interconnect.h"
#include "l1_cache.h"
#include "l2_cache.h"
#include "memory.h"

// The message direction for the passing of cache block requests direction
typedef enum Message_Direction
{
  MSG_L2_TO_L1,
  MSG_L2_TO_MEM,
  MSG_MEM_TO_L2
} Message_Direction;

typedef struct Message
{
  Cache_Block *b;
  // the time needed for this certain message to be processed
  int cycles;
  Message_Direction dir;
} Message;

void interconnect_init(Interconnect_State *i, L2_Cache_State *l2,
                       Memory_State *m)
{
  // Uses a list to store the messages
  i->messages = list_new();
  i->l2 = l2;
  i->m = m;
}

void interconnect_free(Interconnect_State *i)
{
  list_node_t *node;
  list_iterator_t *it = list_iterator_new(i->messages, LIST_HEAD);
  while ((node = list_iterator_next(it)))
  {
    free(node->val);
    list_remove(i->messages, node);
  }
  list_iterator_destroy(it);
  list_destroy(i->messages);
}

// enqueue the message to the message queue
static void interconnect_send(Interconnect_State *i, Cache_Block *b, int cycles,
                              Message_Direction dir)
{
  Message *msg = (Message *)malloc(sizeof(Message));
  msg->b = b;
  /* subtract current cycle */
  msg->cycles = cycles - 1;
  msg->dir = dir;
  list_rpush(i->messages, list_node_new(msg));
}

void interconnect_cycle(Interconnect_State *i)
{
  // Notice this queue is used all over the place to ensure the timing is
  // processed correctly. This is a common pattern in hardware simulation.

  // Interconnection processes 3 types of messages:
  // 1. L2 to L1
  // 2. L2 to MEM
  // 3. MEM to L2
  // instantiates the message queue
  list_node_t *node;
  list_iterator_t *it = list_iterator_new(i->messages, LIST_HEAD);
  // Traverse the message queue, if there is a message, decrement the cycle of
  // message each message is an event, for example. inserting the block or add a
  // memory request
  while ((node = list_iterator_next(it)))
  {
    Message *msg = (Message *)node->val;
    if (msg->cycles > 0)
    {
      msg->cycles--;
    }
    else
    {
      // determine what kind of message this is
      switch (msg->dir)
      {
      case MSG_L2_TO_L1:
        l1_insert_block(msg->b);
        break;
      case MSG_L2_TO_MEM:
        memory_add_request(i->m, msg->b);
        break;
      case MSG_MEM_TO_L2:
        l2_insert_block(i->l2, msg->b);
        break;
      }
      free(msg);
      list_remove(i->messages, node);
    }
  }
  list_iterator_destroy(it);
}

// Notice that is groups the interconnect functions by the direction of the
// message Also adds the cycles to each functions, for modularity Modeling each
// needed event as a function of the interconnections for extensibility
void interconnect_l1_to_l2(Interconnect_State *i, Cache_Block *b)
{
  // each access to interconnection triggers another event
  /* no latency for L2 probe */
  l2_cache_probe(i->l2, b);
}

void interconnect_l1_to_l2_cancel(Interconnect_State *i, Cache_Block *b)
{
  /* no latency for cancellation */
  l2_cancel_cache_access(i->l2, b);
}

void interconnect_l2_to_l1(Interconnect_State *i, Cache_Block *b)
{
  int cycles = 15;
  interconnect_send(i, b, cycles, MSG_L2_TO_L1);
  debug_int("L2 to L1 notification for 0x%x inserted (%d cylces left)\n",
            b->tag, cycles);
}

void interconnect_l2_to_l1_no_latency(Interconnect_State *i
                                      __attribute__((unused)),
                                      Cache_Block *b)
{
  l1_insert_block(b);
}

void interconnect_l2_to_mem(Interconnect_State *i, Cache_Block *b)
{
  int cycles = 5;
  interconnect_send(i, b, cycles, MSG_L2_TO_MEM);
  debug_int("L2 to MEM notification for 0x%x inserted (%d cylces left)\n",
            b->tag, cycles);
}

void interconnect_mem_to_l2(Interconnect_State *i, Cache_Block *b)
{
  int cycles = 5;
  interconnect_send(i, b, cycles, MSG_MEM_TO_L2);
  debug_int("MEM to L2 notification for 0x%x inserted (%d cylces left)\n",
            b->tag, cycles);
}
