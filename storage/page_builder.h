// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef STORAGE_PAGE_BUILDER_H_
#define STORAGE_PAGE_BUILDER_H_

#include <cstdint>
#include <cstddef>
#include <string>

#include "filesystem/file.h"
#include "common/chunk.h"
#include "common/coding.h"
#include "util/slice.h"
#include "util/crc32c.h"

namespace litelsm {

enum class PageType : uint8_t
{
    kDataPage = 0,
    kIndexPage = 1,
    kFilterPage = 2
};

#pragma pack(push, 1)
struct PageFooter
{
    PageFooter() : type(PageType::kDataPage), checksum(0) {}
    PageType type;
    uint32_t checksum;
};
#pragma pack(pop)

static const size_t PAGESIZE = 4 * 1024;

struct PageBuilderOptions
{
    size_t pageSize = PAGESIZE;
};

class PageBuilder {
public:
    using RecordLengthType = uint32_t;

    virtual ~PageBuilder() = default;

    size_t pageSize() const {
        return pageSize_;
    }

    PageType type() const {
        return footer_.type;
    }

    Slice finish() {
        PageType pageType;
        encode_fixed8(reinterpret_cast<uint8_t*>(&pageType), static_cast<uint8_t>(footer_.type));
        buffer_.append(reinterpret_cast<const char*>(&pageType), sizeof(pageType));
        footer_.checksum = crc32c::Value(buffer_.data(), buffer_.size());
        buffer_.append(reinterpret_cast<const char*>(&footer_.checksum), sizeof(footer_.checksum));
        return Slice(buffer_);
    }

protected:
    PageFooter footer_;
    size_t pageSize_;
    std::string buffer_;
};

};  // namespace litelsm

#endif  // STORAGE_PAGE_BUILDER_H_ 