#define NOMINMAX
#include <iostream>
#include <iomanip>
#include "wpp/Trace.h"
#include "wpp/DefaultTracing.h"

constexpr const GUID CONTROL_GUID{
    0x11223344, 0xaaaa, 0xbbbb, {0xcc, 0xcc, 0xdd, 0xff, 0xff, 0x11, 0x22, 0x33}};

#define TRACE_INFO(fmt, ...) WPP_TRACE_INFO(fmt, __VA_ARGS__)

template<typename T>
constexpr T maxVal() {
    return (std::numeric_limits<T>::max)();
}

int main() {
    wpp::WppTraceGuard guard{CONTROL_GUID};
    const void* ptr = &main;
    const auto* str = "str";
    const auto* wstr = L"wstr";
    constexpr const int8_t int8 = static_cast<int8_t>(maxVal<uint8_t>() - 1);
    constexpr const int16_t int16 = static_cast<int16_t>(maxVal<uint16_t>() - 1);
    constexpr const int32_t int32 = static_cast<int32_t>(maxVal<uint32_t>() - 1);
    constexpr const int64_t int64 = static_cast<int64_t>(maxVal<uint64_t>() - 1);
    constexpr const uint8_t uint8 = maxVal<uint8_t>() - 1;
    constexpr const uint16_t uint16 = maxVal<uint16_t>() - 1;
    constexpr const uint32_t uint32 = maxVal<uint32_t>() - 1;
    constexpr const uint64_t uint64 = maxVal<uint64_t>() - 1;
    constexpr const size_t size = maxVal<size_t>() - 1;
    constexpr const ptrdiff_t ptrdiff = static_cast<ptrdiff_t>(maxVal<size_t>() - 1);
    constexpr const float flt = std::numeric_limits<float>::max();
    constexpr const double dbl = std::numeric_limits<double>::max();
    constexpr const long double long_dbl = std::numeric_limits<long double>::max();
    TRACE_INFO("Pointer: {} {:p}", ptr, ptr);
    TRACE_INFO("String: {}, {:s}, {:x}, {:xd}", "str", str, str, str);
    TRACE_INFO("WString: {:}, {:s}, {:x}, {:xd}", L"wstr", wstr, wstr, wstr);
    TRACE_INFO("Int8: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", int8, int8, int8, int8, int8, int8,
               int8);
    TRACE_INFO("Int16: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", int16, int16, int16, int16, int16,
               int16, int16);
    TRACE_INFO("Int32: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", int32, int32, int32, int32, int32,
               int32, int32);
    TRACE_INFO("Int64: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", int64, int64, int64, int64, int64,
               int64, int64);
    TRACE_INFO("PtrDiffT: {:z}, {:zd}, {:zx}, {:zX}, {:zb}, {:zB}, {:zo}", ptrdiff, ptrdiff,
               ptrdiff, ptrdiff, ptrdiff, ptrdiff, ptrdiff);
    TRACE_INFO("UInt8: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", uint8, uint8, uint8, uint8, uint8,
               uint8, uint8);
    TRACE_INFO("UInt16: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", uint16, uint16, uint16, uint16,
               uint16, uint16, uint16);
    TRACE_INFO("UInt32: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", uint32, uint32, uint32, uint32,
               uint32, uint32, uint32);
    TRACE_INFO("UInt64: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", uint64, uint64, uint64, uint64,
               uint64, uint64, uint64);
    TRACE_INFO("SizeT: {:z}, {:zd}, {:zx}, {:zX}, {:zb}, {:zB}, {:zo}", size, size, size, size,
               size, size, size);
    TRACE_INFO("Char: {}, {:c}", 'c', 'c');
    TRACE_INFO("WChar: {}, {:c}", L'c', L'c');
    TRACE_INFO("Float: {}, {:a}, {:A}, {:e}, {:E}, {:f}, {:F}, {:g}, {:G}", flt, flt, flt, flt, flt,
               flt, flt, flt, flt);
    TRACE_INFO("Double: {}, {:a}, {:A}, {:e}, {:E}, {:f}, {:F}, {:g}, {:G}", dbl, dbl, dbl, dbl,
               dbl, dbl, dbl, dbl, dbl);
    TRACE_INFO("Long Double: {}, {:a}, {:A}, {:e}, {:E}, {:f}, {:F}, {:g}, {:G}", long_dbl,
               long_dbl, long_dbl, long_dbl, long_dbl, long_dbl, long_dbl, long_dbl, long_dbl);
    TRACE_INFO("GUID: {}", CONTROL_GUID);

    return 0;
}
