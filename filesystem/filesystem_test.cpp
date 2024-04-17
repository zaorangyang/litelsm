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
protected:
    FileSystemTest() {
        std::string uuid = generateUUID();
        baseDir += uuid;
        fs->makeDirRecursively(baseDir);
    }

    ~FileSystemTest() {
        fs->removeDirRecursively(baseDir);
    }

    void SetUp() override {}

    void TearDown() override {}

    std::string baseDir = "./tmp/filesystem_test_";
    std::shared_ptr<FileSystem> fs = FileSystem::defaultFileSystem();
};

TEST_F(FileSystemTest, posixTest) {
    EXPECT_EQ(true, fs->fileExists(baseDir).ok());
    Status status = fs->makeDir(baseDir + "/test");
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

TEST_F(FileSystemTest, newFile) {
    {
        std::string fname = baseDir + "/test.txt";
        std::unique_ptr<File> file;
        auto status = fs->newRWFile(fname, &file);
        EXPECT_EQ(true, status.ok());
        EXPECT_EQ(true, fs->fileExists(fname).ok());
        status = fs->removeFile(fname);
        EXPECT_EQ(true, status.ok());
        EXPECT_EQ(true, fs->fileExists(fname).isNotFound());
    }

    {
        std::string fname = baseDir + "/test.txt";
        std::unique_ptr<File> file;
        auto status = fs->repenRWFile(fname, &file);
        EXPECT_EQ(true, status.ok());
        EXPECT_EQ(true, fs->fileExists(fname).ok());
        status = fs->removeFile(fname);
        EXPECT_EQ(true, status.ok());
        EXPECT_EQ(true, fs->fileExists(fname).isNotFound());
    }

    {
        std::string fname = baseDir + "/test.txt";
        std::unique_ptr<File> file;
        auto status = fs->openReadableFile(fname, &file);
        EXPECT_EQ(false, status.ok());
        EXPECT_EQ(true, fs->fileExists(fname).isNotFound());
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

};  // namespace litelsm