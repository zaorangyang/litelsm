// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef FILESYSTEM_POSIX_FILESYSTEM_H_
#define FILESYSTEM_POSIX_FILESYSTEM_H_

#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>
#include <filesystem>

#include "util/slice.h"
#include "util/status.h"
#include "filesystem.h"

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

    virtual Status createFile(const std::string& fname);

    virtual ~PosixFileSystem() = default;

    virtual FileSystemType getFileSystemType() {
        return FileSystemType::kPosix;
    }

    Status iterateDir(const std::string& dir, const std::function<bool(std::string_view)>& cb);
};

static Status ioError(const std::string& context, int err_number) {
    std::string msg;
    switch (err_number) {
    case 0:
        return Status::OK();
    case ENOENT:
        msg = context + " " + std::strerror(err_number);
        return Status::NotFound(msg);
    case EEXIST:
        msg = context + " " + std::strerror(err_number);
        return Status::AlreadyExist(msg);
    default:
        msg = context + " " + std::strerror(err_number);
        return Status::IOError(msg);
    }
}

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

Status PosixFileSystem::createFile(const std::string& path) {
    return Status::OK();
}

std::shared_ptr<FileSystem> FileSystem::defaultFileSystem() {
    static std::shared_ptr<FileSystem> fs = std::make_shared<PosixFileSystem>();
    return fs;
};

};  // namespace litelsm

#endif  // FILESYSTEM_POSIX_FILESYSTEM_H_