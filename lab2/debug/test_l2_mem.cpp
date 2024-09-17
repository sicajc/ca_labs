// googletest
#include <gtest/gtest.h>
#include "l2_mem.hpp"
#include <queue>

// inst mem
std::array<uint32_t, MEM_WORD_SIZE> test_inst_mem;
// data mem
std::array<uint32_t, MEM_WORD_SIZE> test_data_mem;
// pseudo data mem
std::array<uint32_t, MEM_WORD_SIZE> test_pseudo_data_mem;

struct cache_test : testing::Test
{
    int NUM_OF_TESTS = 100000;

    void SetUp()
    {
        // Initialize the cache
        init_l2_cache();

        // Seed the random number generator
        srand(471);

        // randomy generate the init_mem
        for (int i = 0; i < MEM_WORD_SIZE; i++)
        {
            test_inst_mem[i] = rand() % 1024;
            test_data_mem[i] = rand() % 1024;
        }

        // copy the whole array of test_data_mem to test_pseudo_data_mem, use stl algorithm function to do so
        std::copy(test_data_mem.begin(), test_data_mem.end(), test_pseudo_data_mem.begin());
    }

    void TearDown()
    {
        //
    }
};

// Test initialization of l2 cache
TEST_F(cache_test, L2_cache_init_test)
{
    // Testing strategy:
    // Initialize the l2 cache
    // Traverse the l2 cache and check if all the blocks are initialized correctly
    // Check if all the blocks are invalid, not dirty, and lru_cnt is 0
    // Check if all the blocks have tag 0
    // Check if all the blocks have value 0
    for (auto &set : l2_cache_mem)
    {
        for (auto &block : set)
        {
            ASSERT_EQ(block.key.dirty, 0);
            ASSERT_EQ(block.key.valid, 0);
            ASSERT_EQ(block.key.lru_cnt, 0);
            ASSERT_EQ(block.key.tag, 0);
            for (int i = 0; i < L2_CACHE_WORDS_IN_BLOCK; i++)
            {
                ASSERT_EQ(block.value.value[i], 0);
            }
        }
    }
}
// Test read a block from mem
TEST_F(cache_test, L2_cache_read_block_from_mem_test)
{
    // Testing strategy:
    // Generate a random address
    // Generate a random tag
    // Generate a random index
    // Read the block from the memory
    // Check if the block is read correctly
    for (int i = 0; i < NUM_OF_TESTS; i++)
    {
        // Generates a random address
        uint32_t addr = rand() % uint32_t(MEM_WORD_SIZE);

        // Gets the golden data from memory
        cache_block block, golden_block;
        golden_block.key.dirty = 0;
        golden_block.key.valid = 0;
        golden_block.key.lru_cnt = 0;
        golden_block.key.tag = 0;

        // get a whole block from the specified memory address
        // Note you must make sure you are using word address
        uint32_t block_addr = ((addr / L2_CACHE_BLOCK_SIZE) << uint32_t(log2(L2_CACHE_BLOCK_SIZE))) / uint32_t(WORD);

        // reads from the memory
        for (int i = 0; i < L2_CACHE_WORDS_IN_BLOCK; i++)
        {
            golden_block.value.value[i] = test_data_mem[block_addr + i];
        }

        decoded_addr_info_t cache_info = decode_addr(addr, WORD_SIZE, L2_CACHE_SIZE, L2_CACHE_WAYS, L2_CACHE_BLOCK_SIZE);
        block.key.tag = cache_info.tags;
        block.key.valid = 1;
        block.key.dirty = 0;
        block.key.lru_cnt = 0;

        block = read_block_from_mem(D_CACHE, block, cache_info.tags, cache_info.index);

        for (int i = 0; i < L2_CACHE_WORDS_IN_BLOCK; i++)
        {
            ASSERT_EQ(block.value.value[i], golden_block.value.value[i]);
        }
    }
}

// Test read functionality of l2 cache
TEST_F(cache_test, L2_cache_read_test)
{
    // Testing strategy:
    // randomly set values of l2 cache
    // read the values from the l2 cache to see if the same

    for (int i = 0; i < NUM_OF_TESTS; i++)
    {
        // Generates a random address
        uint32_t addr = rand() % uint32_t(MEM_WORD_SIZE);

        decoded_addr_info_t cache_info = decode_addr(addr, WORD_SIZE, L2_CACHE_SIZE, L2_CACHE_WAYS, L2_CACHE_BLOCK_SIZE);

        // random block
        cache_block block;

        // Writes the block to the cache
        block.key.tag = cache_info.tags;
        block.key.valid = 1;
        block.key.dirty = 0;
        block.key.lru_cnt = 0;

        // Replace the block of l2 cache with the block
        l2_cache_mem[cache_info.index][0] = block;

        // Reads the data from the l2 cache
        uint32_t read_data = read_l2_cache_mem(cache_info.tags, cache_info.index, cache_info.offset);

        // Check if the data read is the same as the data written
        ASSERT_EQ(read_data, block.value.value[cache_info.offset]);
    }
}

// Test writes of l2 cache
TEST_F(cache_test, L2_cache_write_test)
{
    // addr queue and data queue
    std::queue<uint32_t> addr_queue;
    std::queue<uint32_t> data_queue;
    // Testing strategy:
    // Generate random word and random address
    // write the values to random word address of the l2 cache and golden memory
    // Read the values from the l2 cache and compare to the golden pseudo memory
    for (int i = 0; i < NUM_OF_TESTS; i++)
    {
        // Generates a random address
        uint32_t addr = rand() % uint32_t(MEM_WORD_SIZE);

        uint32_t data = rand() % 1024;

        // put addr and data to the queue
        addr_queue.push(addr);
        data_queue.push(data);

        // writes to data memory as golden
        test_data_mem[addr / WORD] = data;

        // writes the data to l2 cache
        decoded_addr_info_t cache_info = decode_addr(addr, WORD_SIZE, L2_CACHE_SIZE, L2_CACHE_WAYS, L2_CACHE_BLOCK_SIZE);

        // set all tags in l2cache to valid
        for (auto &block : l2_cache_mem[cache_info.index])
        {
            block.key.valid = 1;
        }

        write_l2_cache_mem(cache_info.tags, cache_info.index, cache_info.offset, data);
    }

    // Testing
    for (int i = 0; i < NUM_OF_TESTS; i++)
    {
        // Get the address and data from the queue
        uint32_t addr = addr_queue.front();
        addr_queue.pop();

        decoded_addr_info_t cache_info = decode_addr(addr, WORD_SIZE, L2_CACHE_SIZE, L2_CACHE_WAYS, L2_CACHE_BLOCK_SIZE);

        uint32_t golden_data = test_data_mem[addr / WORD];

        uint32_t out_data =  read_l2_cache_mem(cache_info.tags, cache_info.index, cache_info.offset);

        // Check if the data read is the same as the data written
        ASSERT_EQ(out_data, golden_data) << "Number of test: " << i << " Address: " << addr << " Data: " << golden_data;
    }
}

// Integration test

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    // filter
    testing::GTEST_FLAG(filter) = "cache_test.L2_cache_write_test";

    return RUN_ALL_TESTS();
}
