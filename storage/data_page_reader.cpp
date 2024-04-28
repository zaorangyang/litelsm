// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <cstdint>
#include <cstddef>
#include <string>
#include <assert.h>
#include <iostream>

#include "filesystem/file.h"
#include "common/chunk.h"
#include "common/coding.h"
#include "common/iterator.h"
#include "util/slice.h"
#include "util/crc32c.h"
#include "page_reader.h"
#include "page_builder.h"
#include "data_page_reader.h"

namespace litelsm {

class DataPageIterator : public Iterator {
public:
    using RestartPointType = uint32_t;
    using InnerLengthType = uint32_t;
    using InnterCountType = uint32_t;
    static const int RestartPointTypeSize = sizeof(RestartPointType);
    static const int InnerLengthTypeSize = sizeof(InnerLengthType);
    static const int InnterCountTypeSize = sizeof(InnterCountType);

    DataPageIterator(const Slice& data, const Comparator* comparator) : data_(data.data(), data.getSize() - sizeof(PageFooter)), 
                                                                        comparator_(comparator), 
                                                                        cur_(0) {
        restartPointNum_ = decode_fixed32_le(reinterpret_cast<const uint8_t*>(data_.data() + data_.getSize() - InnterCountTypeSize));
        restartPointStart_ = data_.getSize() - InnterCountTypeSize - restartPointNum_ * RestartPointTypeSize;
    }
    virtual ~DataPageIterator() = default;
    virtual bool valid() const override;
    virtual void seekToFirst() override;
    virtual void seekToLast() override;
    virtual void seek(const Slice& target) override;
    virtual void next() override;
    virtual void prev() override;
    virtual Slice key() override;
    virtual Slice value() override;

private:
    size_t getEntrySize(size_t entryOffset) {
        const uint8_t* cur = reinterpret_cast<const uint8_t*>(data_.data() + entryOffset);
        size_t entrySize = 0;
        InnerLengthType length;

        // key prefix length
        get_varint32(cur, InnerLengthTypeSize, &length);
        entrySize += varint_length(length);
        cur = cur + varint_length(length);

        // key suffix length
        get_varint32(cur, InnerLengthTypeSize, &length);
        entrySize += varint_length(length);
        entrySize += length;
        cur = cur + varint_length(length);

        // value length
        get_varint32(cur, InnerLengthTypeSize, &length);
        entrySize += varint_length(length);
        entrySize += length;

        return entrySize;
    }

    inline size_t getRestartPointOffset(int restartPointIndex) const {
        return restartPointStart_ + restartPointIndex * RestartPointTypeSize;
    }

    inline size_t getRestartPointEntry(int restartPointIndex) const {
        return decode_fixed32_le(reinterpret_cast<const uint8_t*>(data_.data() + getRestartPointOffset(restartPointIndex)));
    }

    Slice getRestartPointKey(int restartPoint) const {
        size_t entry = getRestartPointEntry(restartPoint);
        const uint8_t* cur = reinterpret_cast<const uint8_t*>(data_.data() + entry);
        InnerLengthType length;
        get_varint32(cur, InnerLengthTypeSize, &length);
        cur = cur + varint_length(length);

        InnerLengthType keyLength;
        get_varint32(cur, InnerLengthTypeSize, &keyLength);
        cur = cur + varint_length(keyLength); 

        get_varint32(cur, InnerLengthTypeSize, &length);
        cur = cur + varint_length(length);

        return Slice(reinterpret_cast<const char*>(cur), keyLength);
    }

    Slice data_;
    const Comparator* comparator_;
    size_t cur_;
    std::string key_;
    std::string value_;
    uint32_t restartPointNum_;
    size_t restartPointStart_;
    int curRestartPoint_ = 0;
};

bool DataPageIterator::valid() const {
    return cur_ < restartPointStart_;
};

void DataPageIterator::seekToFirst() {
    cur_ = 0;
    curRestartPoint_ = 0;
};

void DataPageIterator::seekToLast() {
    const char* data = data_.data();
    curRestartPoint_ = restartPointNum_ - 1;
    const char* curRestartPointPtr = data + getRestartPointOffset(curRestartPoint_);
    size_t curEntry = decode_fixed32_le(reinterpret_cast<const uint8_t*>(curRestartPointPtr));
    while (curEntry + getEntrySize(curEntry) < restartPointStart_ ) {
        curEntry += getEntrySize(curEntry);
    }
    cur_ = curEntry;
};

void DataPageIterator::next() {
    if (!valid()) return;
    cur_ += getEntrySize(cur_);
    if (cur_ == getRestartPointOffset(curRestartPoint_ + 1)) {
        curRestartPoint_++;
    }
};

void DataPageIterator::prev() {
    if (!valid()) return;
    if (cur_ == 0) {
        cur_ = restartPointStart_;
        curRestartPoint_ = restartPointNum_ - 1;
        return;
    }
    if (cur_ == getRestartPointEntry(curRestartPoint_)) {
        curRestartPoint_--;
    }
    size_t curEntry = getRestartPointEntry(curRestartPoint_);
    while (curEntry + getEntrySize(curEntry) < cur_) {
        curEntry += getEntrySize(curEntry);
    }
    cur_ = curEntry;
};

void DataPageIterator::seek(const Slice& target) {
    // Empty page
    if (restartPointNum_ == 0) {
        return;
    }
    int left = 0;
    int right = restartPointNum_ - 1;
    while (left < right) {
        int mid = (left + right) / 2;
        const Slice& key = getRestartPointKey(mid);
        if (comparator_->compare(key, target) < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    assert(left == right);
    if (left == 0) {
        seekToFirst();
        return;
    }
    size_t start = getRestartPointOffset(left - 1);
    if (left == restartPointNum_ - 1) {
        const Slice& key = getRestartPointKey(left);
        if (comparator_->compare(key, target) < 0) {
            // All restart point entry lower than target, we need to seek to the last restart point
            start = getRestartPointOffset(left);
        }
    }
    cur_ =  decode_fixed32_le(reinterpret_cast<const uint8_t*>(data_.data() + start));
    Slice key = this->key();
    while (valid() && comparator_->compare(key, target) < 0) {
        next();
        key = this->key();
    }
};

Slice DataPageIterator::key() {
    const uint8_t* cur = reinterpret_cast<const uint8_t*>(data_.data() + cur_);
    InnerLengthType prefixLength;
    get_varint32(cur, InnerLengthTypeSize, &prefixLength);
    cur = cur + varint_length(prefixLength);
    InnerLengthType suffixLength;
    get_varint32(cur, InnerLengthTypeSize, &suffixLength);
    cur = cur + varint_length(suffixLength); 
    InnerLengthType valueLength;
    get_varint32(cur, InnerLengthTypeSize, &valueLength);
    cur = cur + varint_length(valueLength);
    const Slice& restartKey = getRestartPointKey(curRestartPoint_);
    key_.clear();
    key_.append(restartKey.data(), prefixLength);
    key_.append(reinterpret_cast<const char*>(cur), suffixLength);
    return key_;
};

Slice DataPageIterator::value() {
    const uint8_t* cur = reinterpret_cast<const uint8_t*>(data_.data() + cur_);
    InnerLengthType prefixLength;
    get_varint32(cur, InnerLengthTypeSize, &prefixLength);
    cur = cur + varint_length(prefixLength);

    InnerLengthType suffixLength;
    get_varint32(cur, InnerLengthTypeSize, &suffixLength);
    cur = cur + varint_length(suffixLength); 

    InnerLengthType valueLength;
    get_varint32(cur, InnerLengthTypeSize, &valueLength);
    cur = cur + varint_length(valueLength) + suffixLength;

    value_.clear();
    value_.append(reinterpret_cast<const char*>(cur), valueLength);
    return value_;
};

Iterator* DataPageReader::newIterator(const Comparator* comparator) {
    return new DataPageIterator(data_, comparator);
};
    
};  // namespace litelsm