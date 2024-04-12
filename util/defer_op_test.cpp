#include <gtest/gtest.h>
#include <memory>

#include "defer_op.h"

namespace litelsm {

TEST(SliceTest, testDeferOp) {
    int x = 0;
    {
        DeferOp defer([&x]() {
            x = 1;
        });
        EXPECT_EQ(x, 0);
    }
    EXPECT_EQ(x, 1);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

};  // namespace litelsm