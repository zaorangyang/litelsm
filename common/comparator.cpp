// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include "comparator.h"

namespace litelsm {

class LiteLsmDefaultComparator : public Comparator {
 public:
  LiteLsmDefaultComparator() = default;

  virtual ~LiteLsmDefaultComparator() = default;

  // Three-way comparison function:
  //   if a < b: negative result
  //   if a > b: positive result
  //   else: zero result
  int compare(const Slice& a, const Slice& b) const override {
    return a.compare(b);
  }

  const char* Name() const override {
    return "litelsm.LiteLsmDefaultComparator";
  }
};

Comparator* createLiteLsmDefaultComparator() {
  static LiteLsmDefaultComparator* comparator = new LiteLsmDefaultComparator();
  return comparator; 
}

};  // namespace litelsm