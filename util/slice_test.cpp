#include <gtest/gtest.h>
#include <memory>

#include "slice.h"

namespace litelsm {

TEST(SliceTest, testString) {
    std::string str = "hello";
    Slice slice(str);
    EXPECT_EQ(slice.getSize(), 5);
    EXPECT_EQ(slice.data()[0], 'h');
    EXPECT_EQ(slice.data()[1], 'e');
    EXPECT_EQ(slice.data()[2], 'l');
    EXPECT_EQ(slice.data()[3], 'l');
    EXPECT_EQ(slice.data()[4], 'o');
}

TEST(SliceTest, testRawData) {
    size_t dataSize = 500;
    std::unique_ptr<uint8_t[]> data(new uint8_t[dataSize]);
    for (size_t i = 0; i < dataSize; i++) {
        data[i] = i;
    }
    Slice slice(data.get(), dataSize);
    EXPECT_EQ(slice.getSize(), dataSize);
    for (size_t i = 0; i < dataSize; i++) {
        EXPECT_EQ((uint8_t) slice.data()[i], data[i]);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

};  // namespace litelsm