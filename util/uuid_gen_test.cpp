#include <gtest/gtest.h>
#include <regex>

#include "uuid_gen.h"

namespace litelsm {

TEST(UUIDGenTest, GeneratesValidUUID) {
    std::string uuid = generateUUID();

    // Regular expression pattern to match UUID format
    std::regex pattern(R"([0-9a-fA-F]{8}_[0-9a-fA-F]{4}_[0-9a-fA-F]{4}_[0-9a-fA-F]{4}_[0-9a-fA-F]{12})");

    // Check if the generated UUID matches the expected format
    EXPECT_TRUE(std::regex_match(uuid, pattern));
}

TEST(UUIDGenTest, GeneratesUniqueUUIDs) {
    std::string uuid1 = generateUUID();
    std::string uuid2 = generateUUID();

    // Check if the generated UUIDs are unique
    EXPECT_NE(uuid1, uuid2);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
    
};  // namespace litelsm