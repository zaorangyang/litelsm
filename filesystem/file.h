// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef FILESYSTEM_FILE_H_
#define FILESYSTEM_FILE_H_

#include <cstdint>
#include <cstddef>

#include "util/slice.h"
#include "util/status.h"

namespace litelsm {

class File {
public:
    File() = default;
    virtual ~File() = default;
    virtual Status append(const Slice& slice) = 0;
    virtual Status flush() = 0;
    virtual Status sync() = 0;
    virtual Status close() = 0;
    // The caller should ensure that the buffer is large enough to hold the data.
    virtual Status read(uint64_t offset, size_t size, Slice* data, char* buf) = 0;
};

};  // namespace litelsm

#endif  // FILESYSTEM_FILE_H_