// Copyright (c) 2024-present, Zaorang Yang.  All rights reserved.
// This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <random>
#include <sstream>
#include <iomanip>

namespace litelsm {

std::string generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            ss << "_";
        } else if (i == 14) {
            ss << "4";  // UUID version 4
        } else if (i == 19) {
            ss << dis(gen) % 4 + 8; // UUID variant
        } else {
            ss << dis(gen);
        }
    }

    return ss.str();
}

}  // namespace litelsm
