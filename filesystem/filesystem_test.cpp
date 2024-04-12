#include <gtest/gtest.h>
#include <memory>

#include "util/uuid_gen.h"
#include "util/defer_op.h"
#include "filesystem.h"

namespace litelsm {

TEST(FileSystem, defaultTest) {
    std::shared_ptr<FileSystem> fs = FileSystem::defaultFileSystem();
    EXPECT_EQ(fs->getFileSystemType(), FileSystemType::kPosix);
}

class FileSystemTest : public ::testing::Test {
public:
    FileSystemTest() {}

    void SetUp() override {
        std::string uuid = generateUUID();
        base_dir_ += uuid;
    }

    void TearDown() override {}

    std::string getBaseDir() {
        return base_dir_;
    }

private:
    std::string base_dir_ = "./tmp/filesystem_test_";
};

TEST_F(FileSystemTest, posixTest) {
    std::shared_ptr<FileSystem> fs = FileSystem::defaultFileSystem();
    EXPECT_EQ(fs->getFileSystemType(), FileSystemType::kPosix);
    std::string baseDir = getBaseDir();
    DeferOp defer([&]() {
        Status status = fs->removeDirRecursively(baseDir);
        EXPECT_EQ(true, status.ok());
    });
    auto status = fs->makeDirRecursively(baseDir);
    EXPECT_EQ(true, status.ok());
    EXPECT_EQ(true, fs->fileExists(baseDir).ok());
    status = fs->makeDir(baseDir + "/test");
    EXPECT_EQ(true, status.ok());
    EXPECT_EQ(true, fs->fileExists(baseDir + "/test").ok());
    status = fs->removeDir(baseDir + "/test");
    
    EXPECT_EQ(true, status.ok());

    // Delete no-exist directory
    std::string noExistDir = "./asfsdfxxxxx";
    status = fs->fileExists(noExistDir);
    EXPECT_EQ(true, status.isNotFound());
    status = fs->removeDir(noExistDir);
    EXPECT_EQ(true, status.isNotFound());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

};  // namespace litelsm