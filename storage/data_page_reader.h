// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef STORAGE_DATA_PAGE_READER_H_
#define STORAGE_DATA_PAGE_READER_H_

#include <cstdint>
#include <cstddef>
#include <string>

#include "filesystem/file.h"
#include "common/chunk.h"
#include "common/coding.h"
#include "util/slice.h"
#include "util/crc32c.h"
#include "page_reader.h"

namespace litelsm {

class DataPageReader : public PageReader {
public:
    DataPageReader(const Slice& data) : PageReader(data) {}
    virtual ~DataPageReader() = default;
    virtual Iterator* newIterator(const Comparator* comparator);
};

};  // namespace litelsm

#endif  // STORAGE_DATA_PAGE_READER_H_ 