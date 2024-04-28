// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef STORAGE_DATA_PAGE_BUILDER_H_
#define STORAGE_DATA_PAGE_BUILDER_H_

#include <cstdint>
#include <cstddef>
#include <string>
#include <iostream>

#include "filesystem/file.h"
#include "common/chunk.h"
#include "common/coding.h"
#include "util/slice.h"
#include "util/crc32c.h"
#include "page_builder.h"

namespace litelsm {

class DataPageBuilder : public PageBuilder {
public:
    static const uint8_t RestartPointInterval = 16;

    DataPageBuilder() {
        footer_.type = PageType::kDataPage;
    }

    DataPageBuilder(const PageBuilderOptions& options) : PageBuilder(options) {
        footer_.type = PageType::kDataPage;
    }

    virtual ~DataPageBuilder() = default;

    static inline size_t estimateEntrySize(const Slice& key, const Slice& value) {
        return varint_length(key.getSize()) + key.getSize() + varint_length(value.getSize()) + value.getSize() + sizeof(uint32_t);
    }

    inline size_t estimateSize() {
        return buffer_.size() + sizeof(PageFooter) + sizeof(uint32_t) + (1 + restartPointOffsets_.size()) * sizeof(uint32_t);
    }

    Slice finish() {
        // Add restart points.
        for (uint32_t offset : restartPointOffsets_) {
            buffer_.append(reinterpret_cast<const char*>(&offset), sizeof(offset));
        }
        uint32_t restartPointOffsetsSize = restartPointOffsets_.size();
        buffer_.append(reinterpret_cast<const char*>(&restartPointOffsetsSize), sizeof(restartPointOffsetsSize));
        return PageBuilder::finish();
    }

    virtual void add(const Slice& key, const Slice& value);

    inline size_t getRecordNum() const {
        return recordNum_;
    }

private:
    size_t commonPrefix(const Slice& key, const Slice& lastKey);

    int recordNum_ = 0;
    Slice lastRestartPointKey_;
    std::vector<uint32_t> restartPointOffsets_;
};

};  // namespace litelsm

#endif  // STORAGE_DATA_PAGE_BUILDER_H_ 