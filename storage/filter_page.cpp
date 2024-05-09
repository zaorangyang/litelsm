// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <assert.h>

#include "filter_page.h"
#include "common/filter_policy.h"
#include "common/coding.h"

namespace litelsm {

// See doc/table_format.md for an explanation of the filter block format.

// Generate new filter every 2KB of data
static const size_t kFilterBaseLg = 11;
static const size_t kFilterBase = 1 << kFilterBaseLg;

FilterPageBuilder::FilterPageBuilder(const FilterPolicy* policy)
    : policy_(policy) {
  footer_.type = PageType::kFilterPage;
  // For filter page, we won't consider the page size for now.
  pageSize_ = -1;
}

void FilterPageBuilder::StartBlock(uint64_t block_offset) {
  uint64_t filter_index = (block_offset / kFilterBase);
  assert(filter_index >= filter_offsets_.size());
  while (filter_index > filter_offsets_.size()) {
    GenerateFilter();
  }
}

void FilterPageBuilder::AddKey(const Slice& key) {
  Slice k = key;
  start_.push_back(keys_.size());
  keys_.append(k.data(), k.getSize());
}

Slice FilterPageBuilder::Finish() {
  if (!start_.empty()) {
    GenerateFilter();
  }

  // Append array of per-filter offsets
  const uint32_t array_offset = buffer_.size();
  for (size_t i = 0; i < filter_offsets_.size(); i++) {
    put_fixed32_le(&buffer_, filter_offsets_[i]);
  }

  put_fixed32_le(&buffer_, array_offset);
  buffer_.push_back(kFilterBaseLg);  // Save encoding parameter in result
  return PageBuilder::finish();
}

void FilterPageBuilder::GenerateFilter() {
  const size_t num_keys = start_.size();
  if (num_keys == 0) {
    // Fast path if there are no keys for this filter
    filter_offsets_.push_back(buffer_.size());
    return;
  }

  // Make list of keys from flattened key structure
  start_.push_back(keys_.size());  // Simplify length computation
  tmp_keys_.resize(num_keys);
  for (size_t i = 0; i < num_keys; i++) {
    const char* base = keys_.data() + start_[i];
    size_t length = start_[i + 1] - start_[i];
    tmp_keys_[i] = Slice(base, length);
  }

  // Generate filter for current set of keys and append to buffer_.
  filter_offsets_.push_back(buffer_.size());
  policy_->CreateFilter(&tmp_keys_[0], static_cast<int>(num_keys), &buffer_);

  tmp_keys_.clear();
  keys_.clear();
  start_.clear();
}

FilterPageReader::FilterPageReader(const FilterPolicy* policy,
                                     const Slice& contents)
    : PageReader(contents), policy_(policy), data_(nullptr), offset_(nullptr), num_(0), base_lg_(0) {
  contents_ = Slice(contents.data(), contents.getSize() - sizeof(PageFooter));
  size_t n = contents_.getSize();
  if (n < 5) return;  // 1 byte for base_lg_ and 4 for start of offset array
  base_lg_ = contents_[n - 1];
  uint32_t last_word = decode_fixed32_le(reinterpret_cast<const uint8_t*>(contents_.data() + n - 5));
  if (last_word > n - 5) return;
  data_ = contents_.data();
  offset_ = data_ + last_word;
  num_ = (n - 5 - last_word) / 4;
}

bool FilterPageReader::KeyMayMatch(uint64_t block_offset, const Slice& key) {
  uint64_t index = block_offset >> base_lg_;
  if (index < num_) {
    uint32_t start = decode_fixed32_le(reinterpret_cast<const uint8_t*>(offset_ + index * 4));
    uint32_t limit = decode_fixed32_le(reinterpret_cast<const uint8_t*>(offset_ + index * 4 + 4));
    if (start <= limit && limit <= static_cast<size_t>(offset_ - data_)) {
      Slice filter = Slice(data_ + start, limit - start);
      return policy_->KeyMayMatch(key, filter);
    } else if (start == limit) {
      // Empty filters do not match any keys
      return false;
    }
  }
  return true;  // Errors are treated as potential matches
}

}  // namespace leveldb
