//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license.
// (https://developers.google.com/open-source/licenses/bsd)

#ifndef COMMON_CODING_H_
#define COMMON_CODING_H_

#include <string>

#include "util/slice.h"

namespace litelsm {

inline void encode_fixed8(uint8_t* buf, uint8_t val) {
    *buf = val;
}

inline void encode_fixed16_le(uint8_t* buf, uint16_t val) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    memcpy(buf, &val, sizeof(val));
#else
    uint16_t res = bswap_16(val);
    memcpy(buf, &res, sizeof(res));
#endif
}

inline void encode_fixed32_le(uint8_t* buf, uint32_t val) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    memcpy(buf, &val, sizeof(val));
#else
    uint32_t res = bswap_32(val);
    memcpy(buf, &res, sizeof(res));
#endif
}

inline void encode_fixed64_le(uint8_t* buf, uint64_t val) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    memcpy(buf, &val, sizeof(val));
#else
    uint64_t res = gbswap_64(val);
    memcpy(buf, &res, sizeof(res));
#endif
}

inline uint8_t decode_fixed8(const uint8_t* buf) {
    return *buf;
}

inline uint16_t decode_fixed16_le(const uint8_t* buf) {
    uint16_t res;
    memcpy(&res, buf, sizeof(res));
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return res;
#else
    return bswap_16(res);
#endif
}

inline uint32_t decode_fixed32_le(const uint8_t* buf) {
    uint32_t res;
    memcpy(&res, buf, sizeof(res));
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return res;
#else
    return bswap_32(res);
#endif
}

inline uint64_t decode_fixed64_le(const uint8_t* buf) {
    uint64_t res;
    memcpy(&res, buf, sizeof(res));
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return res;
#else
    return gbswap_64(res);
#endif
}

template <typename T>
inline void put_fixed32_le(T* dst, uint32_t val) {
    uint8_t buf[sizeof(val)];
    encode_fixed32_le(buf, val);
    dst->append((char*)buf, sizeof(buf));
}

template <typename T>
inline void put_fixed64_le(T* dst, uint64_t val) {
    uint8_t buf[sizeof(val)];
    encode_fixed64_le(buf, val);
    dst->append((char*)buf, sizeof(buf));
}

// Returns the length of the varint32 or varint64 encoding of "v"
inline int varint_length(uint64_t v) {
    int len = 1;
    while (v >= 128) {
        v >>= 7;
        len++;
    }
    return len;
}

extern uint8_t* encode_varint32(uint8_t* dst, uint32_t value);
extern uint8_t* encode_varint64(uint8_t* dst, uint64_t value);

inline uint8_t* encode_varint64(uint8_t* dst, uint64_t v) {
    static const unsigned int B = 128;
    while (v >= B) {
        *(dst++) = (v & (B - 1)) | B;
        v >>= 7;
    }
    *(dst++) = static_cast<unsigned char>(v);
    return dst;
}

extern const uint8_t* decode_varint32_ptr_fallback(const uint8_t* p, const uint8_t* limit, uint32_t* value);

inline const uint8_t* decode_varint32_ptr(const uint8_t* ptr, const uint8_t* limit, uint32_t* value) {
    if (ptr < limit) {
        uint32_t result = *ptr;
        if ((result & 128) == 0) {
            *value = result;
            return ptr + 1;
        }
    }
    return decode_varint32_ptr_fallback(ptr, limit, value);
}

extern const uint8_t* decode_varint64_ptr(const uint8_t* p, const uint8_t* limit, uint64_t* value);

template <typename T>
inline void put_varint32(T* dst, uint32_t v) {
    uint8_t buf[64];
    uint8_t* ptr = encode_varint32(buf, v);
    dst->append((char*)buf, static_cast<size_t>(ptr - buf));
}

template <typename T>
inline void put_varint64(T* dst, uint64_t v) {
    uint8_t buf[64];
    uint8_t* ptr = encode_varint64(buf, v);
    dst->append((char*)buf, static_cast<size_t>(ptr - buf));
}

template <typename T>
inline void put_length_prefixed_slice(T* dst, const Slice& value) {
    put_varint32(dst, value.getSize());
    dst->append(value.data(), value.getSize());
}

template <typename T>
inline void put_varint64_varint32(T* dst, uint64_t v1, uint32_t v2) {
    uint8_t buf[64];
    uint8_t* ptr = encode_varint64(buf, v1);
    ptr = encode_varint32(ptr, v2);
    dst->append((char*)buf, static_cast<size_t>(ptr - buf));
}

// parse a varint32 from the start of `input` into `val`.
// on success, return true and advance `input` past the parsed value.
// on failure, return false and `input` is not modified.
inline bool get_varint32(Slice* input, uint32_t* val) {
    const auto* p = (const uint8_t*)input->data();
    const uint8_t* limit = p + input->getSize();
    const uint8_t* q = decode_varint32_ptr(p, limit, val);
    if (q == nullptr) {
        return false;
    } else {
        *input = Slice(q, limit - q);
        return true;
    }
}

inline bool get_varint32(const uint8_t* p, size_t cell_size, uint32_t* val) {
    const uint8_t* limit = p + cell_size;
    const uint8_t* q = decode_varint32_ptr(p, limit, val);
    if (q == nullptr) {
        return false;
    } else {
        return true;
    }
}

// parse a varint64 from the start of `input` into `val`.
// on success, return true and advance `input` past the parsed value.
// on failure, return false and `input` is not modified.
inline bool get_varint64(Slice* input, uint64_t* val) {
    const auto* p = (const uint8_t*)input->data();
    const uint8_t* limit = p + input->getSize();
    const uint8_t* q = decode_varint64_ptr(p, limit, val);
    if (q == nullptr) {
        return false;
    } else {
        *input = Slice(q, limit - q);
        return true;
    }
}

} // namespace litelsm

#endif  // COMMON_CODING_H_