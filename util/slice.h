// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef UTIL_SLICE_H_
#define UTIL_SLICE_H_

#include <cstdint>
#include <cstddef>
#include <string>
#include <assert.h>

namespace litelsm {

class Slice {
public:
    Slice() : data_(""), size_(0) { }

    Slice(const std::string& data) : data_(data.data()), size_(data.size()) {}

    Slice(const char* data, size_t size) : data_(data), size_(size) {}

    Slice(const char* s) : data_(s), size_(strlen(s)) {}

    Slice(const uint8_t* data, size_t size) : data_(reinterpret_cast<const char*>(data)), size_(size) {}

    const char* data() const {
        return data_;
    }

    size_t getSize() const { return size_; }

    int inline compare(const Slice& b) const {
        int ret = memcmp(data_, b.data_, std::min(size_, b.size_));
        if (ret == 0) {
            if (size_ == b.size_) {
                return 0;
            }
            if (size_ < b.size_) {
                return -1;
            }
            return 1;
        }
        return ret;
    }

    std::string ToString() const { return std::string(data_, size_); }

    char operator [] (size_t index) const {
        assert(index < size_);
        return data_[index];
    }

private:
    const char* data_;
    size_t size_;
};

bool inline operator == (const Slice& a, const Slice& b) {
    int ret = a.compare(b);
    if (ret == 0) {
        return true;
    }
    return false;
}

};  // namespace litelsm

#endif