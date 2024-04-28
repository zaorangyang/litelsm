// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef STORAGE_PAGE_READER_H_
#define STORAGE_PAGE_READER_H_

#include <cstdint>
#include <cstddef>
#include <string>

#include "filesystem/file.h"
#include "common/chunk.h"
#include "common/coding.h"
#include "common/iterator.h"
#include "common/comparator.h"
#include "util/slice.h"
#include "util/crc32c.h"
#include "page_builder.h"

namespace litelsm {

class PageReader {
public:
    PageReader(const Slice& data) : data_(data) {}
    virtual ~PageReader() = default;
    virtual Iterator* newIterator(const Comparator* comparator) = 0;
    bool checkCRC32C() {
        if (data_.getSize() < sizeof(PageFooter)) {
            return false;
        }
        uint32_t crc = crc32c::Value(data_.data(), data_.getSize() - 4);
        uint32_t storedCrc = decode_fixed32_le(reinterpret_cast<const uint8_t*>(data_.data() + data_.getSize() - 4));
        return crc == storedCrc;
    }

    PageType getPageType() {
        return static_cast<PageType>(data_.data()[data_.getSize() - sizeof(PageFooter)]);
    }

protected:
    Slice data_;
};

};  // namespace litelsm

#endif  // STORAGE_PAGE_READER_H_ 