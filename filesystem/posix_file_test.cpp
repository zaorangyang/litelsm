#include <gtest/gtest.h>
#include <memory>

#include "util/uuid_gen.h"
#include "util/defer_op.h"
#include "filesystem.h"
#include "posix_file.h"

namespace litelsm {

class PosixFileTest : public ::testing::Test {
protected:
    PosixFileTest() {
        std::string uuid = generateUUID();
        baseDir += uuid;
        fs->makeDirRecursively(baseDir);
    }

    ~PosixFileTest() {
        fs->removeDirRecursively(baseDir);
    }

    void SetUp() override {}

    void TearDown() override {}

    std::string baseDir = "./tmp/posix_file_test_";
    std::shared_ptr<FileSystem> fs = FileSystem::defaultFileSystem();
};

TEST_F(PosixFileTest, rwFile) {
    std::string fname = baseDir + "/test.txt";
    {
        // Step1: create file and write data
        std::unique_ptr<File> file;
        auto status = fs->newRWFile(fname, &file);
        EXPECT_EQ(true, status.ok());
        size_t size = 1024;
        std::unique_ptr<uint8_t[]> src(new uint8_t[size]);
        for (int i = 0; i < size; i ++) {
            src[i] = i;
        }
        Slice slice(src.get(), size);
        status = file->append(slice);
        EXPECT_EQ(true, status.ok());
        status = file->sync();
        EXPECT_EQ(true, status.ok());

        std::unique_ptr<uint8_t[]> buf(new uint8_t[size]);
        Slice read;
        status = file->read(0, size, &read, (char*) buf.get());
        EXPECT_EQ(true, status.ok());
        uint8_t* p = (uint8_t*) read.data();
        for (int i = 0; i < size; i ++) {
            EXPECT_EQ(src[i], p[i]);
        }
    }

    {
        // Step2: reopen file and write data
        std::unique_ptr<File> file;
        auto status = fs->repenRWFile(fname, &file);
        EXPECT_EQ(true, status.ok());
        size_t size = 1024;
        std::unique_ptr<uint8_t[]> src(new uint8_t[size]);
        for (int i = 0; i < size; i ++) {
            src[i] = size + i;
        }
        Slice slice(src.get(), size);
        status = file->append(slice);
        EXPECT_EQ(true, status.ok());
        status = file->sync();
        EXPECT_EQ(true, status.ok());

        std::unique_ptr<uint8_t[]> buf(new uint8_t[size]);
        Slice read;
        status = file->read(size, size, &read, (char*) buf.get());
        EXPECT_EQ(true, status.ok());
        uint8_t* p = (uint8_t*) read.data();
        for (int i = 0; i < size; i ++) {
            EXPECT_EQ(src[i], p[i]);
        }
    }

    {
        // Step3: read file
        std::unique_ptr<File> file;
        auto status = fs->openReadableFile(fname, &file);
        EXPECT_EQ(true, status.ok());
        size_t size = 2048;
        std::unique_ptr<uint8_t[]> buf(new uint8_t[size]);
        Slice read;
        status = file->read(0, size, &read, (char*) buf.get());
        EXPECT_EQ(true, status.ok());
        uint8_t* p = (uint8_t*) read.data();
        for (int i = 0; i < size; i ++) {
            EXPECT_EQ((uint8_t)i, p[i]);
        }
        status = file->read(512, size, &read, (char*) buf.get());
        EXPECT_EQ(true, status.ok());
        EXPECT_EQ(2048 - 512, read.getSize());
        p = (uint8_t*) read.data();
        for (int i = 512; i < 2048 - 512; i ++) {
            EXPECT_EQ((uint8_t)i, p[i]);
        }
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

};  // namespace litelsm