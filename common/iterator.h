// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef COMMON_ITERATOR_H_
#define COMMON_ITERATOR_H_

#include <cstdint>
#include <cstddef>

#include "util/slice.h"
#include "util/status.h"

namespace litelsm {

class Iterator {
public:
    Iterator() = default;
    virtual ~Iterator() = default;
    virtual bool valid() const = 0;
    virtual void seekToFirst() = 0;
    virtual void seekToLast() = 0;
    virtual void seek(const Slice& target) = 0;
    virtual void next() = 0;
    virtual void prev() = 0;
    virtual Slice key() = 0;
    virtual Slice value() = 0;
};

};  // namespace litelsm

#endif  // COMMON_ITERATOR_H_