#include <gtest/gtest.h>
#include <memory>

#include "status.h"

namespace litelsm {

TEST(StatusTest, testStatus) {
    Status status;
    EXPECT_EQ(status.code(), StatusCode::kOK);
    EXPECT_EQ(status.message(), "");

    Status status1(StatusCode::kNotFound);
    EXPECT_EQ(status1.code(), StatusCode::kNotFound);
    EXPECT_EQ(status1.message(), "");

    std::string str = "hello";
    Slice slice(str);
    Status status2(StatusCode::kCorruption, slice);
    EXPECT_EQ(status2.code(), StatusCode::kCorruption);
    EXPECT_EQ(status2.message(), "hello");

    Status status3(status2);
    EXPECT_EQ(status3.code(), StatusCode::kCorruption);
    EXPECT_EQ(status3.message(), "hello");

    Status status4 = status3;
    EXPECT_EQ(status4.code(), StatusCode::kCorruption);
    EXPECT_EQ(status4.message(), "hello");

    Status status5(std::move(status4));
    EXPECT_EQ(status5.code(), StatusCode::kCorruption);
    EXPECT_EQ(status5.message(), "hello");

    Status status6 = std::move(status5);
    EXPECT_EQ(status6.code(), StatusCode::kCorruption);
    EXPECT_EQ(status6.message(), "hello");

    Status status7(StatusCode::kNotSupported, "world");
    EXPECT_EQ(status7.code(), StatusCode::kNotSupported);
    EXPECT_EQ(status7.message(), "world");
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

};  // namespace litelsm