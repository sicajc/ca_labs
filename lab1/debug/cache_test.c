#include <stdio.h>
#include <gtest/gtest.h>
#include <assert.h>
#include <queue>
#include "cache.h"

// instantiate memory
uint32_t inst_mem[MEM_WORD_SIZE];
uint32_t data_mem[MEM_WORD_SIZE];
uint32_t pseudo_data_mem[MEM_WORD_SIZE];

struct cache_test : testing::Test
{
    int NUM_OF_TESTS = 10000;

    void SetUp()
    {
        // Initialize the cache
        init_cache();

        // Seed the random number generator
        srand(1234);

        // randomy generate the init_mem
        for (int i = 0; i < MEM_WORD_SIZE; i++)
        {
            inst_mem[i] = rand() % 1024;
            data_mem[i] = rand() % 1024;
        }

        // copy the whole array of data_mem to pseudo_data_mem, use stl algorithm function to do so
        for (int i = 0; i < MEM_WORD_SIZE; i++)
        {
            pseudo_data_mem[i] = data_mem[i];
        }
    }

    void TearDown()
    {
        //
    }
};

TEST_F(cache_test, I_Cache_TestHitOf2DataOfSameBlockI_cache)
{
    // Testing strategy:
    // Insert pc address 0,4,8,12,16,20,24,28,0,4,8,12,16,20,24,28,32.... repeatly into the cache
    // The cache is 4-way associative, 64 sets, 32B block size
    // Expect the hit rate to be 100%, as the same block is being accessed

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    for (int i = 0; i < NUM_OF_TESTS; i++)
    {

        addr = addr % I_CACHE_BLOCK_SIZE;

        uint32_t word_addr = addr / WORD;

        uint32_t golden = inst_mem[word_addr]; // Note the golden is byte address
        // printf("Addr: %d\n", addr);
        uint32_t value = i_cache_get(addr, &cycle_time);

        ASSERT_EQ(value, golden) << "Num of tests: " << i << "  Addrs:" << addr;

        addr += 4;

        assert(addr % 4 == 0);
    }

    // printf("Cycle time: %d\n", cycle_time);

    // Expect the time to be 50, since only 1 cache miss would occurs
    // (cycle_time, 50);
    ASSERT_EQ(cycle_time, 50) << "Cycle time: " << cycle_time;

    // Expects reading the same value from the cache
}

TEST_F(cache_test, I_Cache_TestConsecutiveExecutionOfSameSet)
{
    // Testing strategy:
    // Insert PC address of (tag,index) pair to test execution of same set but with different data
    // Pair (0,0),(1,0),(2,0),(3,0) -> (0,0),(1,0) .....
    // The cache is 4-way associative, 64 sets, 32B block size
    // Expect 4 misses, later all hits,200 cycle time

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    uint32_t tag = 0;
    uint32_t index = 0;

    for (int i = 0; i < 100; i++)
    {
        // Address generations
        cache_info_t cache_info = calculate_cache_info(addr,
                                                       uint32_t(WORD_SIZE),
                                                       uint32_t(I_CACHE_SIZE),
                                                       uint32_t(I_CACHE_WAYS),
                                                       uint32_t(I_CACHE_BLOCK_SIZE));

        addr = (tag << cache_info.tag_bit_shift) | (index << cache_info.index_bit_shift);

        uint32_t word_addr = addr / WORD;

        uint32_t golden = inst_mem[word_addr]; // Note the golden is byte address
        uint32_t value = i_cache_get(addr, &cycle_time);

        ASSERT_EQ(value, golden) << "Num of tests: " << i << "  Addrs:" << addr
                                 << "  Golden: " << golden << "  Value: " << value
                                 << " Tag: " << tag << "  Index: " << index;

        // Increment the tag and index
        tag = (tag + 1) % 4;
    }

    // Expect the time to be 200
    EXPECT_EQ(cycle_time, 200);
}

TEST_F(cache_test, I_Cache_Test_LRU_Policy)
{
    // Testing strategy:
    // Insert PC address of (tag,index) pair to test execution of same set but with different data
    // Pair (0,0),(1,0),(2,0),(3,0),(4,0) -> (0,0),(1,0) .....
    // The cache is 4-way associative, 64 sets, 32B block size
    // Expect all misses, 5000 cycles

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    uint32_t tag = 0;
    uint32_t index = 0;

    for (int i = 0; i < 100; i++)
    {
        // Address generations
        cache_info_t cache_info = calculate_cache_info(addr,
                                                       uint32_t(WORD_SIZE),
                                                       uint32_t(I_CACHE_SIZE),
                                                       uint32_t(I_CACHE_WAYS),
                                                       uint32_t(I_CACHE_BLOCK_SIZE));
        addr = (tag << cache_info.tag_bit_shift) | (index << cache_info.index_bit_shift);

        uint32_t word_addr = addr / WORD;

        uint32_t golden = inst_mem[word_addr]; // Note the golden is byte address
        uint32_t value = i_cache_get(addr, &cycle_time);

        ASSERT_EQ(value, golden) << "Num of tests: " << i << "  Addrs:" << addr
                                 << "  Golden: " << golden << "  Value: " << value
                                 << " Tag: " << tag << "  Index: " << index;

        // Increment the tag and index
        tag = (tag + 1) % 5;
    }

    // Expect the time to be 5000
    EXPECT_EQ(cycle_time, 5000);
}

TEST_F(cache_test, I_Cache_TestRandomAccess)
{
    // Testing strategy:
    // Insert PC address of (tag,index) pair to test execution of same set but with different data
    // Pair (0,0),(1,0),(2,0),(3,0),(4,0) -> (0,0),(1,0) .....
    // The cache is 4-way associative, 64 sets, 32B block size
    // Expect all misses, 5000 cycles

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    uint32_t tag = 0;
    uint32_t index = 0;

    for (int i = 0; i < 100000; i++)
    {
        // Address generations
        addr = rand() % MEM_WORD_SIZE;

        uint32_t word_addr = addr / WORD;

        uint32_t golden = inst_mem[word_addr]; // Note the golden is word address
        uint32_t value = i_cache_get(addr, &cycle_time);

        EXPECT_EQ(value, golden) << "Num of tests: " << i << "  Addrs:" << addr
                                 << "  Golden: " << golden << "  Value: " << value
                                 << " Tag: " << tag << "  Index: " << index;
    }
}

TEST_F(cache_test, D_Cache_RD_TestHitOf2DataOfSameBlock)
{
    // Testing strategy:
    // Insert mem address 0,4,8,12,16,20,24,28,0,4,8,12,16,20,24,28,32.... repeatly into the cache
    // The cache is 8-way associative, 256 sets, 32B block size
    // Expects only 1 miss, 50 cycle time

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    bool read_cmd = true;
    uint32_t data;

    for (int i = 0; i < NUM_OF_TESTS; i++)
    {

        addr = addr % D_CACHE_BLOCK_SIZE;

        uint32_t word_addr = addr / WORD;

        uint32_t golden = data_mem[word_addr]; // Note the golden is byte address
        // printf("Addr: %d\n", addr);
        uint32_t value = d_cache_get(read_cmd, data, addr, &cycle_time);

        ASSERT_EQ(value, golden) << "Num of tests: " << i << "  Addrs:" << addr;

        addr += 4;

        assert(addr % 4 == 0);
    }

    // printf("Cycle time: %d\n", cycle_time);

    // Expect the time to be 50, since only 1 cache miss would occurs
    // (cycle_time, 50);
    ASSERT_EQ(cycle_time, 50) << "Cycle time: " << cycle_time;

    // Expects reading the same value from the cache
}

TEST_F(cache_test, D_Cache_TestConsecutiveExecutionOfSameSet)
{
    // Testing strategy:
    // Insert PC address of (tag,index) pair to test execution of same set but with different data
    // Pair (0,0),(1,0),(2,0),(3,0),(4,0),(5,0),(6,0),(7,0) -> (0,0),(1,0) ..... repeats
    // The cache is 4-way associative, 64 sets, 32B block size
    // Expect 8 misses, later all hits,400 cycle time

    uint32_t cycle_time = 0;
    bool read_cmd = true;
    uint32_t addr = 0;
    uint32_t tag = 0;
    uint32_t index = 0;
    uint32_t data = 0;

    // Beware that the MEM_WORD_SIZE should be adjusted large enough for these test to work
    for (int i = 0; i < 100; i++)
    {
        // Address generations
        // Address generations
        cache_info_t cache_info = calculate_cache_info(addr,
                                                       uint32_t(WORD_SIZE),
                                                       uint32_t(D_CACHE_SIZE),
                                                       uint32_t(D_CACHE_WAYS),
                                                       uint32_t(D_CACHE_BLOCK_SIZE));
        addr = (tag << cache_info.tag_bit_shift) | (index << cache_info.index_bit_shift);

        uint32_t word_addr = addr / WORD;

        uint32_t golden = data_mem[word_addr]; // Note the golden is byte address
        uint32_t value = d_cache_get(read_cmd, data, addr, &cycle_time);

        ASSERT_EQ(value, golden) << "Num of tests: " << i << "  Addrs:" << addr
                                 << "  Golden: " << golden << "  Value: " << value
                                 << " Tag: " << tag << "  Index: " << index;

        // Increment the tag and index
        tag = (tag + 1) % 8;
    }

    // Expect the time to be 400
    EXPECT_EQ(cycle_time, 400);
}

TEST_F(cache_test, D_Cache_Test_LRU_Policy)
{
    // Testing strategy:
    // D $cache is 8-way associative, 256 sets, 32B block size
    // Insert PC address of (tag,index) pair to test execution of same set but with differen
    // Pair (0,0),(1,0),(2,0),(3,0),(4,0),(5,0),(6,0),(7,0),(8,0) -> (0,0),(1,0) ..... REPEATS
    // Expect all misses, 5000 cycles

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    uint32_t tag = 0;
    uint32_t index = 0;
    uint32_t data = 0;
    bool read_cmd = true;

    for (int i = 0; i < 100; i++)
    {

        // Address generations
        cache_info_t cache_info = calculate_cache_info(addr,
                                                       uint32_t(WORD_SIZE),
                                                       uint32_t(D_CACHE_SIZE),
                                                       uint32_t(D_CACHE_WAYS),
                                                       uint32_t(D_CACHE_BLOCK_SIZE));
        addr = (tag << cache_info.tag_bit_shift) | (index << cache_info.index_bit_shift);

        uint32_t word_addr = addr / WORD;

        uint32_t golden = data_mem[word_addr]; // Note the golden is byte address
        uint32_t value = d_cache_get(read_cmd, data, addr, &cycle_time);

        ASSERT_EQ(value, golden) << "Num of tests: " << i << "  Addrs:" << addr
                                 << "  Golden: " << golden << "  Value: " << value
                                 << " Tag: " << tag << "  Index: " << index;

        // Increment the tag and index
        tag = (tag + 1) % 9;
    }

    // Expect the time to be 5000
    EXPECT_EQ(cycle_time, 5000);
}

// Random test read_cmd of d_cache
TEST_F(cache_test, D_Cache_TestRandomAccess)
{
    // Testing strategy:
    // Insert PC address of (tag,index) pair to test execution of same set but with different data
    // Pair (0,0),(1,0),(2,0),(3,0),(4,0),(5,0),(6,0),(7,0),(8,0) -> (0,0),(1,0) ..... REPEATS
    // Expect all misses, 5000 cycles

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    uint32_t tag = 0;
    uint32_t index = 0;
    uint32_t data = 0;
    bool read_cmd = true;

    for (int i = 0; i < 100000; i++)
    {
        // Address generations
        addr = rand() % MEM_WORD_SIZE;

        uint32_t word_addr = addr / WORD;

        uint32_t golden = data_mem[word_addr]; // Note the golden is byte address
        uint32_t value = d_cache_get(read_cmd, data, addr, &cycle_time);

        ASSERT_EQ(value, golden) << "Num of tests: " << i << "  Addrs:" << addr
                                 << "  Golden: " << golden << "  Value: " << value
                                 << " Tag: " << tag << "  Index: " << index;
    }
}

// Want to test, if dirty tag is working as expected
// Test if the data is written to the cache and the dirty bit is set

// Write of D_cache
TEST_F(cache_test, D_Cache_WR_TestHitOf2DataOfSameBlock)
{
    // Testing strategy:
    // Write to mem address 0,4,8,12,16,20,24,28,0,4,8,12,16,20,24,28,32.... repeatly into the cache
    // The cache is 8-way associative, 256 sets, 32B block size
    // Creates two memory, one for data and one for cache write
    // Expects only 1 write miss, 50 cycle time

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    bool read_cmd = false;
    uint32_t data = 1234;
    uint32_t expected_cycle_time = 50;

    uint32_t pseudo_data_mem[MEM_WORD_SIZE];

    // copy the whole array of data_mem to pseudo_data_mem
    for (int i = 0; i < MEM_WORD_SIZE; i++)
    {
        pseudo_data_mem[i] = data_mem[i];
    }

    for (int i = 0; i < NUM_OF_TESTS; i++)
    {
        addr = addr % 32;

        // Generate random data to write
        data = rand();

        uint32_t word_addr = addr / WORD;

        // writes to the data_mem
        data_mem[word_addr] = data;

        // printf("Addr: %d\n", addr);
        uint32_t value = d_cache_get(read_cmd, data, addr, &cycle_time);

        addr += 4;

        // CHECKS if the value writes to the cache is the same as the data_mem
        ASSERT_EQ(value, data) << "Num of tests: " << i << "  Addrs:" << addr;
    }

    // printf("Cycle time: %d\n", cycle_time);

    // Expect the time to be 50, since only 1 write cache miss would occurs
    // (cycle_time, 50);
    ASSERT_EQ(cycle_time, expected_cycle_time) << "Cycle time: " << cycle_time;

    // Expects reading the same value from the cache
}

// Write of D_cache, to test the LRU policy and dirty bits
TEST_F(cache_test, D_cache_rd_wr_test)
{
    // Testing strategy:
    // D $cache is 8-way associative, 256 sets, 32B block size
    // Insert PC address of (tag,index) pair to test execution of same set but with different data
    // First write to the cache, then read_cmd from the cache
    // Checks the data after reading out from the cache
    // Writes sequentially, 0,4,8,12,24,28,32,36 to the same block then reads it out

    uint32_t cycle_time = 0;
    uint32_t addr = 0;
    uint32_t tag = 0;
    uint32_t index = 0;
    uint32_t data = 0;
    bool read_cmd = false;

    uint32_t expected_cycle_time = 400;

    // initiliaze a queue
    std::queue<uint32_t> addr_queue;

    // Writes to the cache
    for (int pat_counts = 0; pat_counts < NUM_OF_TESTS; pat_counts++)
    {
        // Address generations
        // Address generations
        cache_info_t cache_info = calculate_cache_info(addr,
                                                       uint32_t(WORD_SIZE),
                                                       uint32_t(I_CACHE_SIZE),
                                                       uint32_t(I_CACHE_WAYS),
                                                       uint32_t(I_CACHE_BLOCK_SIZE));
        addr = (tag << cache_info.tag_bit_shift) | (index << cache_info.index_bit_shift);

        // Generate random data to write
        data = rand() % 1024;

        uint32_t word_addr = addr / WORD;

        // writes to the data_mem
        data_mem[word_addr] = data;

        // printf("Addr: %d\n", addr);
        uint32_t value = d_cache_get(read_cmd, data, addr, &cycle_time);

        // put the address into the queue
        addr_queue.push(addr);

        // Increment the tag and index
        tag = (tag + 1) % 8;
    }

    read_cmd = true;

    // Reads from the cache
    for (int pat_counts = 0; pat_counts < NUM_OF_TESTS; pat_counts++)
    {
        // Address generations
        addr = addr_queue.front();
        addr_queue.pop();

        uint32_t word_addr = addr / WORD;

        // reads from the data_mem
        uint32_t golden_data = data_mem[word_addr];

        uint32_t value = d_cache_get(read_cmd, data, addr, &cycle_time);

        // CHECKS if the value writes to the cache is the same as the data_mem
        ASSERT_EQ(value, golden_data) << "Num of tests: " << pat_counts << "  Addrs:" << addr;
    }

    // Expect the time to be 5000
    EXPECT_EQ(cycle_time, expected_cycle_time);
}

// Checks Writeback cache policy
// Write of D_cache, to test the LRU policy and dirty bits
TEST_F(cache_test, D_cache_rd_wr_test_writeback)
{
    // Testing strategy:
    // D $cache is 8-way associative, 256 sets, 32B block size
    // Whenever the cache block is replaced, writes back to the psuedo memory
    // Requires a psuedo memory and a data memory
    // After the completion of accesses, write back those blocks with dirty bits back to the psuedo memory
    // Can achieve similiar effects with the use of queue, that is whenever a data bit is set to dirty,
    // write back to the psuedo memory
    // Compare the value of pseudo memory and data memory after the access

    uint32_t cycle_time = 0;
    uint32_t tag = 0;
    uint32_t index = 0;
    uint32_t data = 0;
    bool read_cmd = false;

    // Intialize the queue
    std::queue<uint32_t> addr_queue;
    std::queue<uint32_t> cmd_queue;

    cache_info_t cache_info = calculate_cache_info(0,
                                                   uint32_t(WORD_SIZE),
                                                   uint32_t(D_CACHE_SIZE),
                                                   uint32_t(D_CACHE_WAYS),
                                                   uint32_t(D_CACHE_BLOCK_SIZE));

    for (int pat_counts = 0; pat_counts < NUM_OF_TESTS; pat_counts++)
    {
        // Address generations
        uint32_t addr = rand() % MEM_WORD_SIZE;

        // Generate random data to write
        data = rand() % 1024;
        read_cmd = rand() % 2;

        // store the addr
        addr_queue.push(addr);
        cmd_queue.push(read_cmd);

        uint32_t word_addr = addr / WORD;

        if (read_cmd == false) // write value into data_memory
            data_mem[word_addr] = data;

        uint32_t value = d_cache_get(read_cmd, data, addr, &cycle_time);
    }

    // Writes back the cache to memory after the completion of the access
    for (int cache_index = 0; cache_index < 256; cache_index++)
        for (int set_iter = 0; set_iter < 8; set_iter++)
        {
            if (d_cache[cache_index][set_iter].key.dirty == 1)
            {
                // write the whole block back to memory
                for (int word_iter = 0; word_iter < 8; word_iter++)
                {
                    uint32_t word = d_cache[cache_index][set_iter].value.value[word_iter];
                    uint32_t wb_block_addr = (d_cache[cache_index][set_iter].key.tag << cache_info.tag_bit_shift) \
                                            | (cache_index << cache_info.index_bit_shift) \
                                            | (word_iter << 2);
                    uint32_t wb_addr = wb_block_addr / BYTE;

                    pseudo_data_mem[wb_addr] = word;
                }
                d_cache[cache_index][set_iter].key.dirty = 0;
            }
        }

    // Compare the data_mem and pseudo_data_mem
    for (int pat_counts = 0; pat_counts < NUM_OF_TESTS; pat_counts++)
    {
        // Address generations
        uint32_t addr = addr_queue.front();
        // std::cout << "Addr: " << addr << '\n';
        addr_queue.pop();

        bool read_cmd = cmd_queue.front();
        cmd_queue.pop();

        uint32_t word_addr = addr / WORD;

        // reads from the data_mem and pseudo_data_mem
        uint32_t test_data = pseudo_data_mem[word_addr];
        uint32_t golden_data = data_mem[word_addr];

// add compiler directive
#ifdef DEBUG
        // print out command and current address
        std::cerr << "Read command: " << read_cmd << "  Addr: " << addr << '\n';
#endif

        // Compare their value
        ASSERT_EQ(test_data, golden_data) << "Num of tests: " << pat_counts << "  Addrs:" << addr
                                          << " Byte addr:" << addr << "  Word addr:" << word_addr;
    }
}

int main()
{
    // Your code here
    testing::InitGoogleTest();

    // Set the filter to run only the specific test
    // ::testing::GTEST_FLAG(filter) = "cache_test.D_Cache_TestConsecutiveExecutionOfSameSet";
    ::testing::GTEST_FLAG(filter) = "cache_test.D_cache_rd_wr_test_writeback";

    RUN_ALL_TESTS();

    return 0;
}
