/*#include <Windows.h>
#include <evntrace.h>*/
#include <iostream>
#include <iomanip>
#include "wppng/Trace.h"
#include "wppng/DefaultTracing.h"

void printf_guid(GUID guid) {
    printf("Guid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
           guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2],
           guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

constexpr const GUID CONTROL_GUID{
    0x11223344, 0xaaaa, 0xbbbb, {0xcc, 0xcc, 0xdd, 0xff, 0xff, 0x11, 0x22, 0x33}};

// #define TRACE_INFO(fmt, ...) WPP_NG_DO_TRACE(provider, 1, ::wpp::TraceLevel::Information, fmt, __VA_ARGS__)
#define TRACE_INFO(fmt, ...)    WPP_NG_TRACE_INFO(fmt, __VA_ARGS__)
// #define TRACE_INFO(fmt, ...) WPP_NG_TRACE_FLAG_LEVEL(1, ::wpp::TraceLevel::Information, fmt, __VA_ARGS__)

template<typename T>
constexpr T maxVal() {
    return (std::numeric_limits<T>::max)();
}

int main() {
    wpp::WppTraceGuard guard{CONTROL_GUID};
    // wpp::wppInitTraces(CONTROL_GUID);
    // auto provider = wpp::TraceProvider(CONTROL_GUID);
    // printf_guid(myGuid);
    // provider.traceMessage(1, wpp::TraceLevel::Information, myGuid, 5);
    const void* ptr = &main;
    const auto* str = "str";
    const auto* wstr = L"wstr";
    int8_t int8 = static_cast<int8_t>(maxVal<uint8_t>() - 1);
    int16_t int16 = static_cast<int16_t>(maxVal<uint16_t>() - 1);
    int32_t int32 = static_cast<int32_t>(maxVal<uint32_t>() - 1);
    int64_t int64 = static_cast<int64_t>(maxVal<uint64_t>() - 1);
    uint8_t uint8 = maxVal<uint8_t>() - 1;
    uint16_t uint16 = maxVal<uint16_t>() - 1;
    uint32_t uint32 = maxVal<uint32_t>() - 1;
    uint64_t uint64 = maxVal<uint64_t>() - 1;
    size_t size = maxVal<size_t>() - 1;
    ptrdiff_t ptrdiff = static_cast<ptrdiff_t>(maxVal<size_t>() - 1);
    TRACE_INFO("Pointer: {} {:p}", ptr, ptr);
    TRACE_INFO("String: {}, {:s}, {:x}, {:xd}", "str", str, str, str);
    TRACE_INFO("WString: {:}, {:s}, {:x}, {:xd}", L"wstr", wstr, wstr, wstr);
    TRACE_INFO("Int8: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", int8, int8, int8, int8, int8, int8, int8);
    TRACE_INFO("Int16: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", int16, int16, int16, int16, int16, int16, int16);
    TRACE_INFO("Int32: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", int32, int32, int32, int32, int32, int32, int32);
    TRACE_INFO("Int64: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", int64, int64, int64, int64, int64, int64, int64);
    TRACE_INFO("PtrDiffT: {:z}, {:zd}, {:zx}, {:zX}, {:zb}, {:zB}, {:zo}", ptrdiff, ptrdiff,
               ptrdiff, ptrdiff, ptrdiff, ptrdiff, ptrdiff);
    TRACE_INFO("UInt8: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", uint8, uint8, uint8, uint8, uint8, uint8,
               uint8);
    TRACE_INFO("UInt16: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", uint16, uint16, uint16, uint16, uint16,
               uint16, uint16);
    TRACE_INFO("UInt32: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", uint32, uint32, uint32, uint32, uint32,
               uint32, uint32);
    TRACE_INFO("UInt64: {}, {:d}, {:x}, {:X}, {:b}, {:B}, {:o}", uint64, uint64, uint64, uint64, uint64,
               uint64, uint64);
    TRACE_INFO("SizeT: {:z}, {:zd}, {:zx}, {:zX}, {:zb}, {:zB}, {:zo}", size, size, size, size,
               size, size, size);
    TRACE_INFO("Char: {}, {:c}", 'c', 'c');
    TRACE_INFO("WChar: {}, {:c}", L'c', L'c');
    TRACE_INFO("GUID: {}", CONTROL_GUID);

    /*const void* ptr = &main;
    const auto* str = "str";
    const auto* wstr = L"wstr";
    int8_t int8 = static_cast<int8_t>(maxVal<uint8_t>() - 1);
    int16_t int16 = static_cast<int16_t>(maxVal<uint16_t>() - 1);
    int32_t int32 = static_cast<int32_t>(maxVal<uint32_t>() - 1);
    int64_t int64 = static_cast<int64_t>(maxVal<uint64_t>() - 1);
    uint8_t uint8 = maxVal<uint8_t>() - 1;
    uint16_t uint16 = maxVal<uint16_t>() - 1;
    uint32_t uint32 = maxVal<uint32_t>() - 1;
    uint64_t uint64 = maxVal<uint64_t>() - 1;
    size_t size = maxVal<size_t>() - 1;
    ptrdiff_t ptrdiff = static_cast<ptrdiff_t>(maxVal<size_t>() - 1);
    
    /*do {
        if (::wpp::g_wppNgDefaultProvider) {
            // TRACE_INFO("Pointer: {} {:p}", ptr, ptr);
            WPP_NG_DO_TRACE((*::wpp::g_wppNgDefaultProvider), 1,
                            ::wpp::TraceLevel::Information, "Pointer: {} {:p}", ptr, ptr);
        }
    } while (0);*/
    /*TRACE_INFO("Pointer: {} {:p}", ptr, ptr);
    TRACE_INFO("String: {}, {:s}", "str", str);
    TRACE_INFO("WString: {:}, {:s}", L"wstr", wstr);
    TRACE_INFO("Int8: {}, {:d}", int8, int8);
    TRACE_INFO("Int16: {}, {:d}", int16, int16);
    TRACE_INFO("Int32: {}, {:d}", int32, int32);
    TRACE_INFO("Int64: {}, {:d}", int64, int64);
    TRACE_INFO("PtrDiff: {:z}, {:zd}", ptrdiff, ptrdiff);
    TRACE_INFO("UInt8: {}, {:d}", uint8, uint8);
    TRACE_INFO("UInt16: {}, {:d}", uint16, uint16);
    TRACE_INFO("UInt32: {}, {:d}", uint32, uint32);
    TRACE_INFO("UInt64: {}, {:d}", uint64, uint64);
    TRACE_INFO("SizeT: {:z}, {:zd}", size, size);
    TRACE_INFO("Char: {}, {:c}", 'c', 'c');
    TRACE_INFO("WChar: {}, {:c}", L'c', L'c');
    TRACE_INFO("GUID: {}", CONTROL_GUID);*/

    // pasten(provider, CONTROL_GUID);
    // constexpr const char* str = "ptr";
    // TRACE_INFO("String: {:s}", "str");
    // TRACE_INFO("Ptr: {} {}", str, str);
    /*TRACE_INFO("Pastenino: {:}, {:}", "str", "str");
    TRACE_INFO("Pastenino: {:}, {:}", "str", "str");
    TRACE_INFO("Hello: {:}, {:}", 5, "str");
    TRACE_INFO("Hello: {:}, {:}", "str", "str");*/
    // TRACE_INFO("String: {:}, {:}", "str", "str");
    // TRACE_INFO_OLD("String: {:}, {:s}", "str", "str");

    //constexpr const auto* const ptr = "Moshe";
    //for (auto i = 10; i < 60; ++i) {
    //    // WPP_NG_DO_TRACE(provider, 1, ::wpp::TraceLevel::Information, "hello world {} {:s}", i,
    //    // ptr); WPP_NG_DO_TRACE(provider, 1, ::wpp::TraceLevel::Information, "hello world {} {:d}",
    //    // i, i+1);
    //    WPP_NG_DO_TRACE(provider, 1, ::wpp::TraceLevel::Information,
    //                    "hello world number {:x}! My name is {:s}", i, ptr);
    //    Sleep(5000);
    //}

    // wpp::wppStopTraces();

    return 0;
}
