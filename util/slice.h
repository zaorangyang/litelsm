// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef UTIL_SLICE_H_
#define UTIL_SLICE_H_

#include <cstdint>
#include <cstddef>
#include <string>

namespace litelsm {

class Slice {
public:
    Slice() : data_(""), size_(0) { }

    Slice(const std::string& data) : data_(data.data()), size_(data.size()) {}

    Slice(const char* data, size_t size) : data_(data), size_(size) {}

    Slice(const uint8_t* data, size_t size) : data_(reinterpret_cast<const char*>(data)), size_(size) {}

    const char* data() const {
        return data_;
    }

    size_t getSize() const { return size_; }

private:
    const char* data_;
    size_t size_;
};

};  // namespace litelsm

#endif