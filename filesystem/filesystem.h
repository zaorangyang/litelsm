// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef FILESYSTEM_FILESYSTEM_H_
#define FILESYSTEM_FILESYSTEM_H_

#include "util/slice.h"
#include "util/status.h"
#include "file.h"

namespace litelsm {

enum class FileSystemType {
    kPosix,
    kOther
};

class FileSystem {
public:
    FileSystem() = default;
    virtual Status makeDir(const std::string& path) = 0;
    virtual Status makeDirRecursively(const std::string& dir) = 0;
    virtual Status removeDir(const std::string& path) = 0;
    virtual Status removeDirRecursively(const std::string& dir) = 0;
    virtual Status fileExists(const std::string& fname) = 0;
    virtual Status listDir(const std::string& dir, std::vector<std::string>* result) = 0;
    virtual Status removeFile(const std::string& path) = 0;
    // Create a new read-write file. If the file already exists, it will be truncated.
    virtual Status newRWFile(const std::string& fname, std::unique_ptr<File>* file) = 0;
    // Reopen a new read-write file. If the file already exists, it will be appened.
    virtual Status repenRWFile(const std::string& fname, std::unique_ptr<File>* file) = 0;
    virtual Status openReadableFile(const std::string& fname, std::unique_ptr<File>* file) = 0;
    virtual ~FileSystem() = default;
    static std::shared_ptr<FileSystem> defaultFileSystem();
    virtual FileSystemType getFileSystemType() = 0;
};

};  // namespace litelsm

#endif  // FILESYSTEM_FILESYSTEM_H_