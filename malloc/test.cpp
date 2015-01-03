#include <gtest/gtest.h>

#include <random>
#include <functional>
#include <chrono>
#include <stdlib.h>
#include <string.h>

TEST(malloc, random)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    auto sizes = std::bind(std::uniform_int_distribution<size_t>(0, 1000000), generator);

    size_t actions = sizes();
    std::set<void *> blocks;

    for (size_t i = 0; i < actions; ++i)
    {
        std::cerr << i << "/" << actions << ": ";
        bool alloc = sizes() % 2;
        if (alloc)
        {
            std::cerr << "allocation" << std::endl;
            size_t size = sizes();
            void * block = malloc(size);
            if (block == nullptr)
            {
                std::cerr << "Failed to allocate " << size << " bytes" << std::endl;
                continue;
            }
            memset(block, size, size);
            blocks.insert(block);
        }
        else
        {
            std::cerr << "deallocation" << std::endl;
            if (!blocks.size())
            {
                std::cerr << "No allocated blocks, skipping" << std::endl;
                continue;
            }
            void * block = *blocks.begin();
            blocks.erase(blocks.begin());
            free(block);
        }
    }
    for (void * block : blocks)
        free(block);
    EXPECT_EQ(0, 0);
}

TEST(calloc, random)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    auto sizes = std::bind(std::uniform_int_distribution<size_t>(0, 1000), generator);

    size_t allocations = sizes();
    for (size_t i = 0; i < allocations; ++i)
    {
        size_t size  = sizes();
        size_t nmemb = sizes();
        unsigned char * block = reinterpret_cast<unsigned char *>(calloc(nmemb, size));
        for (size_t j = 0; j < nmemb; ++j)
            for (size_t k = 0; k < size; ++k)
                EXPECT_EQ(*(block + j * size + k), '\0');
        memset(block, size, nmemb * size);
        free(block);
    }
}

TEST(realloc, random)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    auto sizes = std::bind(std::uniform_int_distribution<size_t>(0, 1000), generator);

    size_t allocations = sizes();
    for (size_t i = 0; i < allocations; ++i)
    {
        size_t size  = sizes();
        void * block = malloc(size);
        void * block_copy = malloc(size);
        memset(block, size, size);
        memcpy(block_copy, block, size);
        size_t new_size = sizes();
        block = realloc(block, new_size);
        size_t copied_size = std::min(size, new_size);
        for (size_t j = 0; j < copied_size; ++j)
            EXPECT_EQ(*(reinterpret_cast<unsigned char *>(block) + j),
                      *(reinterpret_cast<unsigned char *>(block_copy) + j));
        free(block);
        free(block_copy);
    }
}

TEST(posix_memalign, random)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    auto sizes = std::bind(std::uniform_int_distribution<size_t>(0, 10000), generator);
    auto align_pows = std::bind(std::uniform_int_distribution<size_t>(0, 20), generator);

    size_t actions = sizes();
    std::set<void *> blocks;

    for (size_t i = 0; i < actions; ++i)
    {
        std::cerr << i << "/" << actions << ": ";
        bool alloc = sizes() % 2;
        if (alloc)
        {
            std::cerr << "allocation" << std::endl;
            size_t size = sizes();
            size_t alignment = 2 << align_pows();
            void * block;
            int result = posix_memalign(&block, alignment, size);
            if (result == -1)
            {
                std::cerr << "Failed to allocate " << size << " bytes aligned to " << alignment << std::endl;
                continue;
            }
            EXPECT_EQ(reinterpret_cast<size_t>(block) % alignment, 0);
            EXPECT_EQ(reinterpret_cast<size_t>(block) % sizeof(void *), 0);
            memset(block, size, size);
            blocks.insert(block);
        }
        else
        {
            std::cerr << "deallocation" << std::endl;
            if (!blocks.size())
            {
                std::cerr << "No allocated blocks, skipping" << std::endl;
                continue;
            }
            void * block = *blocks.begin();
            blocks.erase(blocks.begin());
            free(block);
        }
    }
    for (void * block : blocks)
        free(block);
    EXPECT_EQ(0, 0);
}

int main(int argc, char ** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
