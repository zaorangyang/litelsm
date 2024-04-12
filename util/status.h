// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef UTIL_STATUS_H_
#define UTIL_STATUS_H_

#include <string>

#include "util/slice.h"

namespace litelsm {

enum class StatusCode {
    kOK = 0,
    kNotFound = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kIOError = 5,
    kAlreadyExist = 6,
};

class Status {
public:
    Status() : code_(StatusCode::kOK), msg_(nullptr) {}
    
    Status(StatusCode code) : code_(code), msg_(nullptr) { }

    Status(StatusCode code, const Slice& slice) : code_(code) {
        char* msg = (char*) malloc(sizeof(char) * slice.getSize() + 1);
        memcpy(msg, slice.data(), slice.getSize());
        msg[slice.getSize()] = '\0';
        msg_.reset(msg);
    }

    Status(StatusCode code, const std::string& msg) : code_(code) {
        char* msg_ptr = (char*) malloc(sizeof(char) * msg.size() + 1);
        strcpy(msg_ptr, msg.c_str());
        msg_.reset(msg_ptr);
    }

    Status(const Status& status) {
        code_ = status.code_;
        if (status.msg_.get() != nullptr) {
            char* msg = (char*) malloc(sizeof(char) * strlen(status.msg_.get()) + 1);
            strcpy(msg, status.msg_.get());
            msg_.reset(msg);
        }
    }

    Status& operator=(const Status& status) {
        code_ = status.code_;
        if (status.msg_.get() != nullptr) {
            char* msg = (char*) malloc(sizeof(char) * strlen(status.msg_.get()) + 1);
            strcpy(msg, status.msg_.get());
            msg_.reset(msg);
        }
        return *this;
    }

    Status(Status&& status) {
        code_ = status.code_;
        msg_ = std::move(status.msg_);
    }

    Status& operator=(Status&& status) {
        code_ = status.code_;
        msg_ = std::move(status.msg_);
        return *this;
    }

    ~Status() {}

    static Status OK() {
        return Status();
    }

    static Status NotFound(const std::string& msg) {
        return Status(StatusCode::kNotFound, msg);
    }

    static Status AlreadyExist(const std::string& msg) {
        return Status(StatusCode::kAlreadyExist, msg);
    }

    static Status IOError(const std::string& msg) {
        return Status(StatusCode::kIOError, msg);
    }

    bool ok() const {
        return code() == StatusCode::kOK;
    }

    bool isNotFound() const {
        return code() == StatusCode::kNotFound;
    }

    StatusCode code() const {
        return code_;
    }

    std::string message() const {
        if (msg_.get() == nullptr) {
            return "";
        }
        return msg_.get();
    }

private:
    StatusCode code_;
    std::unique_ptr<char[]> msg_;
};

};  // namespace litelsm

#endif // UTIL_STATUS_H_