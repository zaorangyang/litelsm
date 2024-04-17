// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).


#include "io_error.h"

namespace litelsm {

Status ioError(const std::string& context, int err_number) {
    std::string msg;
    switch (err_number) {
    case 0:
        return Status::OK();
    case ENOENT:
        msg = context + " " + std::strerror(err_number);
        return Status::NotFound(msg);
    case EEXIST:
        msg = context + " " + std::strerror(err_number);
        return Status::AlreadyExist(msg);
    default:
        msg = context + " " + std::strerror(err_number);
        return Status::IOError(msg);
    }
}

};  // namespace litelsm