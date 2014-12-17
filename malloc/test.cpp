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

int main(int argc, char ** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
