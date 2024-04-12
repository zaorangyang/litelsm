// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef UTIL_DEFER_OP_H_
#define UTIL_DEFER_OP_H_

namespace litelsm {

template <typename DeferFunction>
class DeferOp {
public:
    explicit DeferOp(DeferFunction func) : _func(std::move(func)) {}

    ~DeferOp() noexcept { _func(); }

private:
    DeferFunction _func;
};

}   // namespace litelsm

#endif  // UTIL_DEFER_OP_H_