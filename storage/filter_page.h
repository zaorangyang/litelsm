// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A filter block is stored near the end of a Table file.  It contains
// filters (e.g., bloom filters) for all data blocks in the table combined
// into a single filter block.

#ifndef STORAGE_FILTER_PAGE_H_
#define STORAGE_FILTER_PAGE_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "page_builder.h"
#include "page_reader.h"
#include "util/slice.h"
#include "util/hash.h"

namespace litelsm {

class FilterPolicy;

// A FilterPageBuilder is used to construct all of the filters for a
// particular Table.  It generates a single string which is stored as
// a special block in the Table.
//
// The sequence of calls to FilterPageBuilder must match the regexp:
//      (StartBlock AddKey*)* Finish
class FilterPageBuilder : public PageBuilder {
 public:
  explicit FilterPageBuilder(const FilterPolicy*);

  FilterPageBuilder(const FilterPageBuilder&) = delete;
  FilterPageBuilder& operator=(const FilterPageBuilder&) = delete;

  void StartBlock(uint64_t block_offset);
  void AddKey(const Slice& key);
  Slice Finish();

 private:
  void GenerateFilter();

  const FilterPolicy* policy_;
  std::string keys_;             // Flattened key contents
  std::vector<size_t> start_;    // Starting index in keys_ of each key
  std::vector<Slice> tmp_keys_;  // policy_->CreateFilter() argument
  std::vector<uint32_t> filter_offsets_;
};

class FilterPageReader : public PageReader {
 public:
  // REQUIRES: "contents" and *policy must stay live while *this is live.
  FilterPageReader(const FilterPolicy* policy, const Slice& contents);
  bool KeyMayMatch(uint64_t block_offset, const Slice& key);

 private:
  const FilterPolicy* policy_;
  const char* data_;    // Pointer to filter data (at block-start)
  const char* offset_;  // Pointer to beginning of offset array (at block-end)
  size_t num_;          // Number of entries in offset array
  size_t base_lg_;      // Encoding parameter (see kFilterBaseLg in .cc file)
  Slice contents_;
};

}  // namespace litelsm

#endif  // STORAGE_FILTER_PAGE_H_
