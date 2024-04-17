// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <cstdint>
#include <cstddef>
#include <unistd.h>
#include <iostream>

#include "util/slice.h"
#include "posix_file.h"
#include "io_error.h"

namespace litelsm {

Status PosixFile::append(const Slice& slice) {
    const char* src = slice.data();
    size_t left = slice.getSize();

    while (left != 0) {
        size_t bytes_to_write = left;

        ssize_t done = write(fd_, src, bytes_to_write);
        if (done < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::string errMsg = "While writting file " + fname_;
            return ioError(errMsg, errno);
        }
        left -= done;
        src += done;
    }
    return Status::OK();
}

Status PosixFile::sync() {
    int ret = fsync(fd_);
    if (ret == -1) {
        std::string errMsg = "While syncing file " + fname_;
        return ioError(errMsg, errno);
    }
    return Status::OK();
}

Status PosixFile::close() {
    if (::close(fd_) < 0) {
        std::string errMsg = "While closing file " + fname_;
        return ioError(errMsg, errno);
    }
    fd_ = -1;
    return Status::OK();
}

Status PosixFile::read(uint64_t offset, size_t size, Slice* data, char* buf) {
    size_t left = size;
    while (left != 0) {
        ssize_t done = pread(fd_, buf, size, offset);
        if (done < 0) {
            if (done == -1 && errno == EINTR) {
                continue;
            }
            std::string errMsg = "While reading file " + fname_;
            return ioError(errMsg, errno);
        }
        if (done == 0) {
            break;
        }
        left -= done;
        offset += done;
    }
    *data = Slice(buf, size - left);
    return Status::OK();
}

};  // namespace litelsm