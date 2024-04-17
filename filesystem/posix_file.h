// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef FILESYSTEM_POSIX_FILE_H_
#define FILESYSTEM_POSIX_FILE_H_

#include <cstdint>
#include <cstddef>

#include "util/slice.h"
#include "file.h"

namespace litelsm {

class PosixFile : public File {
public:
    PosixFile(int fd, std::string fname) : fd_(fd), fname_(fname) {}
    virtual ~PosixFile() {
        close();
    }
    virtual Status append(const Slice& slice);
    // Note: Posix API does not provide a flush method, just return OK.
    virtual Status flush() {
        return Status::OK();
    }
    virtual Status sync();
    virtual Status close();
    virtual Status read(uint64_t offset, size_t size, Slice* data, char* buf);
private:
    int fd_;    
    std::string fname_;
};

};  // namespace litelsm

#endif  // FILESYSTEM_POSIX_FILE_H_