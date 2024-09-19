// googletest
#include <gtest/gtest.h>
#include "l2_mem.hpp"
#include "l2_cache.hpp"
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
        int SEED = 347;

        // Initialize the cache
        init_L2_cache();

        // Seed the random number generator
        srand(SEED);

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
TEST_F(cache_test, L2_full_cache_init)
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

// Test if reading from l2 cache works
// Since there is compulsory miss, we will test if the data is written to the l2 cache
TEST_F(cache_test, L2_cache_read_test)
{
    // Testing strategy:
    // Generate a random address, read a block of data from memory
    // Read from the l2 cache
    // Check if the data is written to the l2 cache
    // Check if the data is the same as the data in the memory
    for (int i = 0; i < NUM_OF_TESTS; i++)
    {
        // Generate a random address
        uint32_t addr = uint32_t(rand()) % uint32_t(MEM_WORD_SIZE);

        // decode addr
        decoded_addr_info_t decoded_addr = decode_addr(addr, WORD_SIZE, L2_CACHE_SIZE, L2_CACHE_WAYS, L2_CACHE_BLOCK_SIZE);

        // req_type
        req_cache req_type = addr % 2 == 0 ? D_CACHE : I_CACHE;

        // Read block from the memory
        cache_block block = read_block_from_mem(req_type, decoded_addr.tags, decoded_addr.index);

        memory_request_t req;
        req.valid = true;
        req.cycle_time = 0;
        req.addr = addr;
        req.req_cache_type = req_type;
        req.req_op_type = READ;

        // Read the block from l2_cache
        cache_block read_block = L1_cache_access_L2_cache(req);

        bool equal = block == read_block;
        // TERMINATES THE TEST
        ASSERT_TRUE(equal) << "Number of tests: " << i << " Address: " << addr;
    }
}

// Test if writing works
// 1. Write to the l2 cache with the same addr
TEST_F(cache_test, L2_cache_write_test)
{
    // Testing strategy: Writing to the same addr location
    uint32_t addr = uint32_t(rand()) % uint32_t(MEM_WORD_SIZE);
    for (int i = 0; i < NUM_OF_TESTS; i++)
    {
        // randomly generate a block
        cache_block block;
        for (int i = 0; i < L2_CACHE_WORDS_IN_BLOCK; i++)
        {
            block.value.value[i] = rand() % 1024;
        }

        // decode addr
        decoded_addr_info_t decoded_addr = decode_addr(addr, WORD_SIZE, L2_CACHE_SIZE, L2_CACHE_WAYS, L2_CACHE_BLOCK_SIZE);

        // req_type
        req_cache req_type = rand()%2 ? D_CACHE : I_CACHE;

        // Read block from the memory
        write_data_block_to_mem(req_type, decoded_addr.tags, decoded_addr.index, block);

        memory_request_t req;
        req.valid = true;
        req.cycle_time = 0;
        req.addr = addr;
        req.req_cache_type = req_type;
        req.req_op_type = WRITE;

        // Read the block from l2_cache
        cache_block write_block = L1_cache_access_L2_cache(req);
    }

    // Traverse all set of the cache, get the index
    // then traverse all the ways of the cache get the block
    // check if dirty, if dirty writes the block back to the memory
    for (int i = 0; i < L2_CACHE_SETS; i++)
    {
        for (int j = 0; j < L2_CACHE_WAYS; j++)
        {
            if (l2_cache_mem[i][j].key.dirty == 1)
            {
                cache_block _block;
                _block.value = l2_cache_mem[i][j].value;

                // Need to write block back to dram
                write_data_block_to_mem(l2_cache_mem[i][j].key.req_cache_type, l2_cache_mem[i][j].key.tag, i, _block);
            }
        }
    }

    // Compare the test_data_mem and test_pseudo_data_mem
    for (int i = 0; i < MEM_WORD_SIZE; i++)
    {
        ASSERT_EQ(test_data_mem[i], test_pseudo_data_mem[i]);
    }
}
//2 Keep writing to the same block
TEST_F(cache_test, L2_cache_write_same_block_test)
{
    // Testing strategy: Writing to the same addr location
    uint32_t addr = 0;
    for (int i = 0; i < NUM_OF_TESTS; i++)
    {
        // randomly generate a block
        cache_block block;
        for (int i = 0; i < L2_CACHE_WORDS_IN_BLOCK; i++)
        {
            block.value.value[i] = rand() % 1024;
        }

        // decode addr
        decoded_addr_info_t decoded_addr = decode_addr(addr, WORD_SIZE, L2_CACHE_SIZE, L2_CACHE_WAYS, L2_CACHE_BLOCK_SIZE);

        // req_type
        req_cache req_type = rand()%2 ? D_CACHE : I_CACHE;

        decoded_addr.index = 0;

        // Read block from the memory
        write_data_block_to_mem(req_type, decoded_addr.tags, decoded_addr.index, block);

        memory_request_t req;
        req.valid = true;
        req.cycle_time = 0;
        req.addr = addr;
        req.req_cache_type = req_type;
        req.req_op_type = WRITE;

        // Read the block from l2_cache
        cache_block write_block = L1_cache_access_L2_cache(req);
    }

    // Traverse all set of the cache, get the index
    // then traverse all the ways of the cache get the block
    // check if dirty, if dirty writes the block back to the memory
    for (int i = 0; i < L2_CACHE_SETS; i++)
    {
        for (int j = 0; j < L2_CACHE_WAYS; j++)
        {
            if (l2_cache_mem[i][j].key.dirty == 1)
            {
                cache_block _block;
                _block.value = l2_cache_mem[i][j].value;

                // Need to write block back to dram
                write_data_block_to_mem(l2_cache_mem[i][j].key.req_cache_type, l2_cache_mem[i][j].key.tag, i, _block);
            }
        }
    }

    // Compare the test_data_mem and test_pseudo_data_mem
    for (int i = 0; i < MEM_WORD_SIZE; i++)
    {
        ASSERT_EQ(test_data_mem[i], test_pseudo_data_mem[i]);
    }
}

// Test the mshr entry and its behaviour

// Test the DRAM fill notification

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    // filter
    testing::GTEST_FLAG(filter) = "cache_test.L2_cache_write_test";

    return RUN_ALL_TESTS();
}