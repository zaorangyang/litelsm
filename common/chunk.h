// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef COMMON_CHUNCK_H_
#define COMMON_CHUNCK_H_

#include <cstdint>
#include <cstddef>

#include "util/slice.h"
#include "util/status.h"

namespace litelsm {

class Chunk {
public:
    Chunk() = default;
    virtual ~Chunk() = default;
    virtual Status add(const Slice& key, const Slice& value) = 0;
    // Return kv nums of the chunk.
    virtual size_t size() const = 0;
    // Return memory usage of the chunk.
    virtual size_t memoryUsage() const = 0;
    virtual bool hasNext() const = 0;
    virtual Status getNext(Slice* key, Slice* value) = 0;
};

};  // namespace litelsm

#endif  // COMMON_CHUNCK_H_