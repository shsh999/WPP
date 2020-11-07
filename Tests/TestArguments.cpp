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

#define CHECK_BAD_FORMAT(str, ArgType) CHECK_TYPE(str, ArgType, wpp::InvalidFormatItem)

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

    CHECK_BAD_FORMAT("{:d}", char*);

    struct NotSupported {};
    CHECK_TYPE("{}", NotSupported, TypeDoesNotSupportFormatting);
}
