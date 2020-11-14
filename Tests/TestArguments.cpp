#include "catch.hpp"

#include "wppng/String.h"
#include "wppng/ParseUtils.h"
#include "wppng/TraceItems.h"

#define CHECK_ARGUMENT_COUNT(str, value)                                \
    do {                                                                \
        constexpr const auto result = countArgs(std::string_view(str)); \
        STATIC_REQUIRE(result.status == ArgumentParseStatus::Success);  \
        STATIC_REQUIRE(result.count == value);                          \
    } while (0)

#define CHECK_COUNT_FAILS(str, statusValue)                             \
    do {                                                                \
        constexpr const auto result = countArgs(std::string_view(str)); \
        STATIC_REQUIRE(result.status == statusValue);                   \
    } while (0)

TEST_CASE("Argument count", "[Args]") {
    using namespace ::wpp::internal;
    using namespace ::wpp;

    CHECK_ARGUMENT_COUNT("", 0);
    CHECK_ARGUMENT_COUNT("hello", 0);
    CHECK_ARGUMENT_COUNT("{}", 1);
    CHECK_ARGUMENT_COUNT("hello {}", 1);
    CHECK_ARGUMENT_COUNT("hello {} {} {}", 3);
    CHECK_ARGUMENT_COUNT("hello {} {} {}", 3);

    CHECK_ARGUMENT_COUNT("hello {{}}", 0);
    CHECK_ARGUMENT_COUNT("hello {{{}}}", 1);
    CHECK_ARGUMENT_COUNT("hello {{ asd }} {}", 1);

    CHECK_COUNT_FAILS("{", ArgumentParseStatus::ExcessOpens);
    CHECK_COUNT_FAILS("}", ArgumentParseStatus::ExcessCloses);
    CHECK_COUNT_FAILS("{}}", ArgumentParseStatus::ExcessCloses);
    CHECK_COUNT_FAILS("{{}", ArgumentParseStatus::ExcessCloses);
    CHECK_COUNT_FAILS("{asd}", ArgumentParseStatus::MissingColonInFormat);
}

#define CHECK_STATUS(str, statusValue)                            \
    do {                                                          \
        __WPP_NG_STRING_MAKER(FormatType, str);                   \
        using FormatInfo = decltype(getFormatInfo<FormatType>()); \
        STATIC_REQUIRE(FormatInfo::status() == statusValue);      \
    } while (0)

#define CHECK_TYPE(str, ArgType, TraceType)                                                      \
    do {                                                                                         \
        __WPP_NG_STRING_MAKER(FormatType, str);                                                  \
        using FormatInfo = decltype(getFormatInfo<FormatType>());                                \
        STATIC_REQUIRE(FormatInfo::status() == ArgumentParseStatus::Success);                    \
        STATIC_REQUIRE(                                                                          \
            std::is_same_v<                                                                      \
                decltype(buildTraceItem<std::tuple_element_t<0, decltype(FormatInfo::value())>>( \
                    std::declval<ArgType>())),                                                   \
                TraceType>);                                                                     \
    } while (0)

#define CHECK_BAD_FORMAT(str, ArgType) CHECK_TYPE(str, ArgType, wpp::internal::InvalidFormatItem)

TEST_CASE("Argument types", "[Args]") {
    using namespace ::wpp::internal;
    using namespace ::wpp;

    CHECK_TYPE("{}", int, Int32Item);
    CHECK_TYPE("{:}", int, Int32Item);
    CHECK_TYPE("{:d}", int, Int32Item);
    CHECK_TYPE("{:b}", int, Int32Item);
    CHECK_TYPE("{:B}", int, Int32Item);
    CHECK_TYPE("{:o}", int, Int32Item);
    CHECK_TYPE("{:x}", int, Int32Item);
    CHECK_TYPE("{:X}", int, Int32Item);

    CHECK_BAD_FORMAT("{:D}", int);
    CHECK_BAD_FORMAT("{:O}", int);
    CHECK_BAD_FORMAT("{:dd}", int);
    CHECK_BAD_FORMAT("{:e}", int);
    CHECK_BAD_FORMAT("{:p}", int);
    CHECK_BAD_FORMAT("{:c}", int);
    CHECK_BAD_FORMAT("{:s}", int);

    CHECK_TYPE("{:c}", char, CharItem);
    CHECK_TYPE("{:c}", wchar_t, WCharItem);

    CHECK_TYPE("{}", char*, StringItem);
    CHECK_TYPE("{:s}", char*, StringItem);
    CHECK_TYPE("{:p}", char*, PointerItem);
    CHECK_TYPE("{:x}", char*, HexBufferItem);
    CHECK_TYPE("{:xd}", char*, HexDumpItem);

    CHECK_TYPE("{}", char[], StringItem);
    CHECK_TYPE("{:s}", char[], StringItem);
    CHECK_TYPE("{:p}", char[], PointerItem);
    CHECK_TYPE("{:x}", char[], HexBufferItem);
    CHECK_TYPE("{:xd}", char[], HexDumpItem);

    CHECK_TYPE("{}", char[10], StringItem);
    CHECK_TYPE("{:s}", char[10], StringItem);
    CHECK_TYPE("{:p}", char[10], PointerItem);
    CHECK_TYPE("{:x}", char[10], HexBufferItem);
    CHECK_TYPE("{:xd}", char[10], HexDumpItem);

    CHECK_TYPE("{}", wchar_t*, WStringItem);
    CHECK_TYPE("{:s}", wchar_t*, WStringItem);
    CHECK_TYPE("{:p}", wchar_t*, PointerItem);
    CHECK_TYPE("{:x}", wchar_t*, HexBufferItem);
    CHECK_TYPE("{:xd}", wchar_t*, HexDumpItem);

    CHECK_TYPE("{}", wchar_t[], WStringItem);
    CHECK_TYPE("{:s}", wchar_t[], WStringItem);
    CHECK_TYPE("{:p}", wchar_t[], PointerItem);
    CHECK_TYPE("{:x}", wchar_t[], HexBufferItem);
    CHECK_TYPE("{:xd}", wchar_t[], HexDumpItem);
    
    CHECK_TYPE("{}", wchar_t[10], WStringItem);
    CHECK_TYPE("{:s}", wchar_t[10], WStringItem);
    CHECK_TYPE("{:p}", wchar_t[10], PointerItem);
    CHECK_TYPE("{:x}", wchar_t[10], HexBufferItem);
    CHECK_TYPE("{:xd}", wchar_t[10], HexDumpItem);

    CHECK_TYPE("{}", GUID, GuidItem);
    
    CHECK_TYPE("{}", float, FloatItem);
    CHECK_TYPE("{:a}", float, FloatItem);
    CHECK_TYPE("{:A}", float, FloatItem);
    CHECK_TYPE("{:e}", float, FloatItem);
    CHECK_TYPE("{:E}", float, FloatItem);
    CHECK_TYPE("{:f}", float, FloatItem);
    CHECK_TYPE("{:F}", float, FloatItem);
    CHECK_TYPE("{:g}", float, FloatItem);
    CHECK_TYPE("{:G}", float, FloatItem);

    CHECK_TYPE("{}", double, DoubleItem);
    CHECK_TYPE("{:a}", double, DoubleItem);
    CHECK_TYPE("{:A}", double, DoubleItem);
    CHECK_TYPE("{:e}", double, DoubleItem);
    CHECK_TYPE("{:E}", double, DoubleItem);
    CHECK_TYPE("{:f}", double, DoubleItem);
    CHECK_TYPE("{:F}", double, DoubleItem);
    CHECK_TYPE("{:g}", double, DoubleItem);
    CHECK_TYPE("{:G}", double, DoubleItem);

    CHECK_TYPE("{}", long double, LongDoubleItem);
    CHECK_TYPE("{:a}", long double, LongDoubleItem);
    CHECK_TYPE("{:A}", long double, LongDoubleItem);
    CHECK_TYPE("{:e}", long double, LongDoubleItem);
    CHECK_TYPE("{:E}", long double, LongDoubleItem);
    CHECK_TYPE("{:f}", long double, LongDoubleItem);
    CHECK_TYPE("{:F}", long double, LongDoubleItem);
    CHECK_TYPE("{:g}", long double, LongDoubleItem);
    CHECK_TYPE("{:G}", long double, LongDoubleItem);
}
