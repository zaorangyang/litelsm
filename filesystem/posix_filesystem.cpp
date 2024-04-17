// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>
#include <filesystem>
#include <functional>
#include <fcntl.h>

#include "io_error.h"
#include "util/slice.h"
#include "util/status.h"
#include "filesystem.h"
#include "file.h"
#include "posix_file.h"

namespace litelsm {

class PosixFileSystem : public FileSystem {
public:
    PosixFileSystem() = default;

    virtual Status makeDir(const std::string& dir);

    virtual Status makeDirRecursively(const std::string& dir);

    virtual Status removeDir(const std::string& dir);

    virtual Status removeDirRecursively(const std::string& dir);

    virtual Status fileExists(const std::string& fname);

    virtual Status listDir(const std::string& dir, std::vector<std::string>* result);

    virtual Status removeFile(const std::string& fname);

    virtual Status newRWFile(const std::string& fname, std::unique_ptr<File>* file);

    virtual Status repenRWFile(const std::string& fname, std::unique_ptr<File>* file);

    virtual Status openReadableFile(const std::string& fname, std::unique_ptr<File>* file);

    virtual ~PosixFileSystem() = default;

    virtual FileSystemType getFileSystemType() {
        return FileSystemType::kPosix;
    }

private:
    Status openRWFile(const std::string& fname, bool isTrunc, std::unique_ptr<File>* file);

    Status iterateDir(const std::string& dir, const std::function<bool(std::string_view)>& cb);
};

Status PosixFileSystem::makeDir(const std::string& dir) {
    if (mkdir(dir.c_str(), 0755) != 0) {
        return ioError(dir, errno);
    }
    return Status::OK();
}

Status PosixFileSystem::fileExists(const std::string& fname) {
    int result = access(fname.c_str(), F_OK);
    if (result == 0) {
      return Status::OK();
    }
    return ioError(fname, errno);
}

Status PosixFileSystem::makeDirRecursively(const std::string& dir) {
    std::error_code ec;
    // If dir already exist and is a directory, the return value would be false and ec.value() would be 0
    (void)std::filesystem::create_directories(dir, ec);
    if (ec.value() != 0) {
        std::string msg = "create " + dir + " recursively error: " + ec.message();
        return ioError(msg, ec.value());
    }
    return Status::OK();
}

Status PosixFileSystem::removeDirRecursively(const std::string& dir) {
    std::error_code ec;    
    (void)std::filesystem::remove_all(dir, ec);
    if (ec.value() != 0) {
        std::string msg = "remove " + dir + " recursively error: " + ec.message();
        return ioError(msg, ec.value());
    }
    return Status::OK();
}

Status PosixFileSystem::removeDir(const std::string& dir) {
    if (rmdir(dir.c_str()) != 0) {
        return ioError(dir, errno);
    }
    return Status::OK();
}

Status PosixFileSystem::iterateDir(const std::string& dir, const std::function<bool(std::string_view)>& cb) {
    DIR* d = opendir(dir.c_str());
    if (d == nullptr) {
        return ioError(dir, errno);
    }
    errno = 0;
    Status ret;
    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        std::string_view name(entry->d_name);
        if (name == "." || name == "..") {
            continue;
        }
        auto saved_errno = errno;
        auto r = cb(name);
        errno = saved_errno;
        if (!r) {
            break;
        }
    }
    if (entry == nullptr && errno != 0) {
        return ioError(dir, errno);
    }
    if (closedir(d) != 0) {
        return ioError(dir, errno);
    }
    return ret;
}

Status PosixFileSystem::listDir(const std::string& dir, std::vector<std::string>* result) {
    result->clear();
    return iterateDir(dir, [&](std::string_view name) -> bool {
        result->emplace_back(name);
        return true;
    });
}

Status PosixFileSystem::removeFile(const std::string& fname) {
    if (unlink(fname.c_str()) != 0) {
        return ioError(fname, errno);
    }
    return Status::OK();
}

// Currently, we don't support verbose flags when creating a new file.
Status PosixFileSystem::newRWFile(const std::string& fname, std::unique_ptr<File>* file) {
    return openRWFile(fname, true, file);
}

Status PosixFileSystem::repenRWFile(const std::string& fname,  std::unique_ptr<File>* file) {
    return openRWFile(fname, false, file);
}

Status PosixFileSystem::openRWFile(const std::string& fname, bool isNew, std::unique_ptr<File>* file) {
    int flags = isNew ? (O_CREAT | O_TRUNC) : (O_CREAT | O_APPEND);
    flags |= O_RDWR;
    int fd = open(fname.c_str(), flags, 0644);
    if (fd == -1) {
        std::string errMsg = "While openning file " + fname;
        return ioError(errMsg, errno);
    }
    file->reset(new PosixFile(fd, fname));
    return Status::OK();
}

Status PosixFileSystem::openReadableFile(const std::string& fname, std::unique_ptr<File>* file) {
    int fd = open(fname.c_str(), O_RDONLY);
    if (fd == -1) {
        std::string errMsg = "While openning file " + fname;
        return ioError(errMsg, errno);
    }
    file->reset(new PosixFile(fd, fname));
    return Status::OK();
}

std::shared_ptr<FileSystem> FileSystem::defaultFileSystem() {
    static std::shared_ptr<FileSystem> fs = std::make_shared<PosixFileSystem>();
    return fs;
};

};  // namespace litelsm