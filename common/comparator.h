// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef COMMON_COMPARATOR_H_
#define COMMON_COMPARATOR_H_

#include <string>
#include "util/slice.h"

namespace litelsm {

class Slice;

class Comparator {
 public:
  virtual ~Comparator() = default;

  virtual int compare(const Slice& a, const Slice& b) const = 0;

  virtual const char* Name() const = 0;
};

Comparator* createLiteLsmDefaultComparator();

}   // litelsm

#endif  // COMMON_COMPARATOR_H_
