// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef UTIL_STRING_UTIL_H_
#define UTIL_STRING_UTIL_H_

#include <string>

#include "util/slice.h"

namespace litelsm {

std::string EscapeString(const Slice& value);

};  // namespace litelsm

#endif // UTIL_STRING_UTIL_H_