// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include "data_page_builder.h"
#include "common/coding.h"

namespace litelsm {

size_t DataPageBuilder::commonPrefix(const Slice& key, const Slice& lastKey) {
    size_t minLen = std::min(key.getSize(), lastKey.getSize());
    size_t i = 0;
    for (; i < minLen; i++) {
        if (key.data()[i] != lastKey.data()[i]) {
            break;
        }
    }
    return i;
}

void DataPageBuilder::add(const Slice& key, const Slice& value) {
    size_t prefixLen = 0;
    size_t suffixLen = 0;
    if (recordNum_ % RestartPointInterval == 0) {
        lastRestartPointKey_ = key;
        suffixLen = key.getSize();
        restartPointOffsets_.push_back(buffer_.size());
    } else {
        prefixLen = commonPrefix(key, lastRestartPointKey_);
        suffixLen = key.getSize() - prefixLen;
    }
    size_t start = buffer_.size();
    put_varint32<std::string>(&buffer_, prefixLen);
    put_varint32<std::string>(&buffer_, suffixLen);
    put_varint32<std::string>(&buffer_, value.getSize());
    buffer_.append(key.data() + prefixLen, suffixLen);
    buffer_.append(value.data(), value.getSize());
    recordNum_++;
}

};  // namespace litelsm