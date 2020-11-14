#pragma once
#include <cstdint>
#include "String.h"

namespace wpp::internal::md5 {

/**
 * This structure represents the result of an MD5 sum.
 * In order to convert it to the "standard" md5 digest form, the following python line may be
 * used: `binascii.hexlify(struct.pack('<IIII', a, b, c, d))`.
 */
struct MD5Sum {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;

    constexpr bool operator==(const MD5Sum& other) const {
        return a == other.a && b == other.b && c == other.c && d == other.d;
    }

    constexpr bool operator!=(const MD5Sum& other) const {
        return a != other.a || b != other.b || c != other.c || d != other.d;
    }
};

namespace detail {

/**
 * Tables used by the md5 algorithm.
 */
constexpr const inline std::array<uint32_t, 64> ROUND_SHIFTS = {
    7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 5,  9,  14, 20, 5,  9,
    14, 20, 5,  9,  14, 20, 5,  9,  14, 20, 4,  11, 16, 23, 4,  11, 16, 23, 4,  11, 16, 23,
    4,  11, 16, 23, 6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21};

constexpr const inline std::array<uint32_t, 64> SINE_CONSTANTS{
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

constexpr uint32_t bytesToInt(const char* data) {
    return static_cast<uint8_t>(data[0]) | (static_cast<uint8_t>(data[1]) << 8) |
           (static_cast<uint8_t>(data[2]) << 16) | (static_cast<uint8_t>(data[3]) << 24);
}

static_assert(bytesToInt("\x01\x02\x03\x04") == 0x04030201U);

constexpr std::array<uint32_t, 16> unpackBytes(const char* bytes) {
    std::array<uint32_t, 16> result{};
    for (size_t i = 0; i < 16; ++i) {
        result[i] = bytesToInt(bytes + (4 * i));
    }
    return result;
}

constexpr uint32_t leftRotate(uint32_t n, uint32_t c) {
    return (n << c) | (n >> (32 - c));
}

constexpr auto md5Transform(std::array<uint32_t, 16> chunks, MD5Sum& result) {
    auto A = result.a;
    auto B = result.b;
    auto C = result.c;
    auto D = result.d;
    for (uint32_t i = 0; i < 64; ++i) {
        uint32_t F = 0;
        uint32_t g = 0;

        if (i < 16) {
            F = (B & C) | ((~B) & D);
            g = i;
        } else if (i < 32) {
            F = (D & B) | ((~D) & C);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            F = B ^ C ^ D;
            g = (3 * i + 5) % 16;
        } else {
            F = C ^ (B | (~D));
            g = (7 * i) % 16;
        }

        F = F + A + SINE_CONSTANTS[i] + chunks[g];
        A = D;
        D = C;
        C = B;
        B = B + leftRotate(F, ROUND_SHIFTS[i]);
    }

    result.a += A;
    result.b += B;
    result.c += C;
    result.d += D;
}

constexpr std::array<uint32_t, 16> buildPadding(const char* message, size_t size,
                                                size_t originalSize, bool add1Bit,
                                                bool addOriginalSize) {
    std::array<uint32_t, 16> result{};
    size_t i = 0;
    for (; i < size / 4; ++i) {
        result[i] = bytesToInt(message + i * 4);
    }

    char bytesLeft[4]{};
    for (size_t j = 0; j < 4; ++j) {
        if (j < size % 4) {
            bytesLeft[j] = message[(i * 4) + j];
        } else if (add1Bit) {
            bytesLeft[j] = 0x80U;
            add1Bit = false;
        } else {
            bytesLeft[j] = 0;
        }
    }

    result[i] = bytesToInt(bytesLeft);
    ++i;

    while (i < 14) {
        result[i] = 0;
        ++i;
    }

    if (i == 15 || !addOriginalSize) {
        while (i < 16) {
            result[i] = 0;
            ++i;
        }
        return result;
    }

    result[14] = static_cast<uint32_t>((originalSize * 8) & 0xffffffff);
    result[15] = static_cast<uint32_t>((originalSize >> 29) & 0xffffffff);

    return result;
}

constexpr MD5Sum md5Sum(const char* message, size_t size) {
    MD5Sum result{0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};

    const auto originalSize = size;
    while (size >= 64) {
        md5Transform(unpackBytes(message), result);
        size -= 64;
        message += 64;
    }

    bool shouldAdd1Bit = true;
    if (size >= 56) {
        md5Transform(buildPadding(message, size, originalSize, shouldAdd1Bit, false), result);
        shouldAdd1Bit = false;
        message += size;
        size = 0;
    }

    md5Transform(buildPadding(message, size, originalSize, shouldAdd1Bit, true), result);

    return result;
}

}  // namespace detail

using detail::md5Sum;

template<size_t N>
constexpr MD5Sum md5Sum(const char (&data)[N]) {
    return md5Sum(data, N - 1);
}

template<size_t N>
constexpr MD5Sum md5Sum(const ConstexprString<N>& data) {
    return md5Sum(data.data(), N);
}

}  // namespace wpp::internal::md5