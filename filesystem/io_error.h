// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef FILESYSTEM_IO_ERROR_H_
#define FILESYSTEM_IO_ERROR_H_

#include "util/status.h"

namespace litelsm {

Status ioError(const std::string& context, int err_number);

};  // namespace litelsm

#endif  // FILESYSTEM_IO_ERROR_H_