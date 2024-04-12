// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef FILESYSTEM_FILE_H_
#define FILESYSTEM_FILE_H_

#include <cstdint>
#include <cstddef>

#include "util/slice.h"

namespace litelsm {

class File {
public:
    File() = default;
    virtual ~File() = 0;
    virtual void append(const Slice& slice) = 0;
    virtual void flush() = 0;
    virtual void close() = 0;
    virtual void sync() = 0;
    virtual void read(uint64_t offset, size_t size, Slice* data) = 0;
};

};  // namespace litelsm

#endif  // FILESYSTEM_FILE_H_