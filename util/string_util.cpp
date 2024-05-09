// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include "string_util.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <limits>

#include "slice.h"

namespace litelsm {

static void AppendEscapedStringTo(std::string* str, const Slice& value) {
  for (size_t i = 0; i < value.getSize(); i++) {
    char c = value[i];
    if (c >= ' ' && c <= '~') {
      str->push_back(c);
    } else {
      char buf[10];
      std::snprintf(buf, sizeof(buf), "\\x%02x",
                    static_cast<unsigned int>(c) & 0xff);
      str->append(buf);
    }
  }
}

std::string EscapeString(const Slice& value) {
  std::string r;
  AppendEscapedStringTo(&r, value);
  return r;
}

}  // namespace litelsm
