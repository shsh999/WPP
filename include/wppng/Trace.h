#pragma once
#include <guiddef.h>
#include "TypeTraits.h"
#include "String.h"
#include "PathUtils.h"
#include "TraceItems.h"
#include "TraceProvider.h"
#include "Md5.h"
#include "ParseUtils.h"

#define __WPP_NG_MAKE_STRING_IMPL(...) #__VA_ARGS__
#define __WPP_NG_MAKE_STRING(...) __WPP_NG_MAKE_STRING_IMPL(__VA_ARGS__)##""

#define _WPP_NG_MAKE_WIDE_IMPL(X) L##X
#define __WPP_NG_MAKE_WIDE(X) _WPP_NG_MAKE_WIDE_IMPL(X)
#define __WPP_NG_MAKE_WSTRING(...) __WPP_NG_MAKE_WIDE(__WPP_NG_MAKE_STRING(__VA_ARGS__))

namespace wpp::internal {

////////////////////////////////
// Trace argument annotations //
////////////////////////////////

/**
 * Annotates the function name, containing the trace hash and the trace item argument types.
 * This is used to write the argument types to the PDB file, and the hash is required in order
 * to match the types with the rest of the trace information.
 */
template<uint32_t hashA, uint32_t hashB, uint32_t hashC, uint32_t hashD, typename... Args>
void annotateArgTypes() {
    __annotation(L"TMF_NG_TYPES:", __WPP_NG_MAKE_WIDE(__FUNCSIG__));
}

template<uint32_t hashA, uint32_t hashB, uint32_t hashC, uint32_t hashD, typename Format,
         typename TupleType>
struct AnnotateArgsCaller;

template<uint32_t hashA, uint32_t hashB, uint32_t hashC, uint32_t hashD, typename Format,
         typename... Args>
struct AnnotateArgsCaller<hashA, hashB, hashC, hashD, Format, std::tuple<Args...>> {
    template<size_t... Ixs>
    constexpr void operator()(std::index_sequence<Ixs...>) {
        annotateArgTypes<hashA, hashB, hashC, hashD,
                         decltype(buildTraceItem<std::tuple_element_t<Ixs, Format>>(
                             std::declval<Args>()))...>();
    }
};

///////////////////////////////
// Trace argument validation //
///////////////////////////////

template<typename Format, typename TupleType>
struct ArgChecker;

enum class ArgCheckResult { Success, InvalidFormat };

/**
 * Checks that the format is valid for all the argument types.
 * This is done by checking that all the generated trace items are not InvalidFormatItem.
 */
template<typename... Args>
constexpr ArgCheckResult makeArgCheckStatus() {
    if constexpr ((std::is_same_v<Args, ::wpp::internal::InvalidFormatItem> || ...)) {
        return ArgCheckResult::InvalidFormat;
    } else {
        return ArgCheckResult::Success;
    }
}

template<typename Format, typename... Args>
struct ArgChecker<Format, std::tuple<Args...>> {
    template<size_t... Ixs>
    constexpr auto operator()(std::index_sequence<Ixs...>) {
        return makeArgCheckStatus<decltype(
            buildTraceItem<std::tuple_element_t<Ixs, Format>>(std::declval<Args>()))...>();
    }
};

/////////////////////
// Trace utilities //
/////////////////////

constexpr GUID md5ToUUID3(const wpp::internal::md5::MD5Sum& sum) {
    return {sum.a,
            static_cast<unsigned short>(sum.b & 0xffff),
            static_cast<unsigned short>(((sum.b >> 16) & 0x0fff) | 0x3000),
            {sum.c & 0xff, (sum.c >> 8) & 0xff, (sum.c >> 16) & 0xff, (sum.c >> 24) & 0xff,
             sum.d & 0xff, (sum.d >> 8) & 0xff, (sum.d >> 16) & 0xff, (sum.d >> 24) & 0xff}};
}

template<typename Format, size_t... indices, typename... Args>
constexpr __forceinline void wppNGTraceNewInternal(std::index_sequence<indices...>,
                                                   TraceProvider& provider, const GUID& traceGuid,
                                                   Args&&... args) {
    provider.traceMessageFromTraceItems(
        traceGuid,
        buildTraceItem<std::tuple_element_t<indices, Format>>(std::forward<Args>(args))...);
}

/**
 * Traces only if traces are currently enabled.
 */
template<typename Format, UCHAR flag, TraceLevel level, typename... Args>
constexpr void wppNGTraceNew(TraceProvider& provider, const GUID& traceGuid, Args&&... args) {
    if (provider.areTracesEnabled(flag, level)) {
        wppNGTraceNewInternal<Format>(std::make_index_sequence<sizeof...(Args)>{}, provider,
                                      traceGuid, std::forward<Args>(args)...);
    }
}

}  // namespace wpp::internal

//////////////////
// Trace Macros //
//////////////////

/**
 * Checks that all the constant-type paramters are correct.
 */
#define __WPP_NG_VALIDATE_BASIC_PARAMETERS(provider, flag, level, fmt, ...)                  \
    static_assert(::wpp::internal::IsStringLiteral<decltype(fmt)>,                           \
                  "WPP-NG: The format must be a string literal!");                           \
    static_assert(std::is_same_v<decltype(level), ::wpp::TraceLevel>,                        \
                  "WPP-NG: The trace level must be a TraceLevel!");                          \
    static_assert(std::is_convertible_v<decltype(flag), UCHAR>,                              \
                  "WPP-NG: The flag must be a UCHAR!");                                      \
    static_assert(flag > 0 && (flag & (flag - 1)) == 0, "The flag must be a power of two!"); \
    static_assert(std::is_same_v<std::decay_t<decltype(provider)>, ::wpp::TraceProvider>,    \
                  "WPP-NG: The provider must be a valid TraceProvider!");

/**
 * Checks that the format parsing was successful, and that it matches the number of arguments
 */
#define __WPP_NG_VALIDATE_FORMAT_AND_ARGS(FormatInfo, actualCount, ...)                         \
    if constexpr (FormatInfo::status() != ::wpp::internal::ArgumentParseStatus::Success) {      \
        constexpr const auto ___wpp_ng_status = FormatInfo::status();                           \
        static_assert(___wpp_ng_status != ::wpp::internal::ArgumentParseStatus::ExcessCloses,   \
                      "Too many closing brackets!");                                            \
        static_assert(___wpp_ng_status != ::wpp::internal::ArgumentParseStatus::ExcessOpens,    \
                      "Too many opening brackets!");                                            \
        static_assert(                                                                          \
            ___wpp_ng_status != ::wpp::internal::ArgumentParseStatus::MissingColonInFormat,     \
            "Format specificatiton is missing ':' in brackets!");                               \
    } else {                                                                                    \
        constexpr const auto formatCount = std::tuple_size_v<decltype(FormatInfo::value())>;    \
        static_assert(formatCount <= actualCount,                                               \
                      "The format string specifies more than the passed number of arguments!"); \
        static_assert(formatCount >= actualCount,                                               \
                      "The format string specifies less than the passed number of arguments!"); \
        constexpr const auto argCheckResult =                                                   \
            ::wpp::internal::ArgChecker<decltype(FormatInfo::value()),                          \
                                        decltype(std::forward_as_tuple(__VA_ARGS__))>{}(        \
                std::make_index_sequence<FormatInfo::count()>());                               \
        static_assert(argCheckResult != ::wpp::internal::ArgCheckResult::InvalidFormat,         \
                      "Argument does not support the given extended format specification!");    \
        static_assert(argCheckResult == ::wpp::internal::ArgCheckResult::Success,               \
                      "Unexpected argument check result!");                                     \
    }

/**
 * Calculates the trace hash, used to generate the trace message GUID.
 */
#define __WPP_NG_CALCULATE_TRACE_HASH(baseDirectoryIndex, flag, level, fmt, ...)             \
    ::wpp::internal::md5::md5Sum(                                                            \
        ::wpp::internal::makeString("TMF_NG:") +                                             \
        ::wpp::internal::makeString<sizeof(__FILE__) - baseDirectoryIndex - 1>(              \
            __FILE__ + baseDirectoryIndex) +                                                 \
        ::wpp::internal::makeString(                                                         \
            __WPP_NG_MAKE_STRING(__LINE__) "FUNC=" __FUNCSIG__ "FLAG=" __WPP_NG_MAKE_STRING( \
                flag) "LEVEL=" __WPP_NG_MAKE_STRING(level) fmt __WPP_NG_MAKE_STRING(__VA_ARGS__)))

/**
 * Annotates the trace information into the PDB file.
 */
#define __WPP_NG_ANNOTATE_TRACE_INFO(hash, flag, level, fmt, FormatInfo, ...)                    \
    __annotation(L"TMF_NG:", __WPP_NG_MAKE_WIDE(__FILE__), __WPP_NG_MAKE_WSTRING(__LINE__),      \
                 L"FUNC=" __WPP_NG_MAKE_WIDE(__FUNCSIG__), L"FLAG=" __WPP_NG_MAKE_WSTRING(flag), \
                 L"LEVEL=" __WPP_NG_MAKE_WSTRING(level), __WPP_NG_MAKE_WIDE(fmt),                \
                 __WPP_NG_MAKE_WSTRING(__VA_ARGS__));                                            \
    ::wpp::internal::AnnotateArgsCaller<hash.a, hash.b, hash.c, hash.d,                          \
                                        decltype(FormatInfo::value()),                           \
                                        decltype(std::forward_as_tuple(__VA_ARGS__))>{}(         \
        std::make_index_sequence<FormatInfo::count()>())

/**
 * This is the main tracing macro used by wpp-ng.
 *
 * provider - a TraceProvider
 * flag - a UCHAR with only a single bit set (a power of 2)
 * level - a wpp::TraceLevel value
 * fmt - a string literal containing the trace format
 * ... - the arguments to trace. The arguments must match the format string.
 */
#define WPP_NG_DO_TRACE(provider, flag, level, fmt, ...)                                          \
    do {                                                                                          \
        __WPP_NG_VALIDATE_BASIC_PARAMETERS(provider, flag, level, fmt, __VA_ARGS__);              \
        constexpr const auto ___wpp_ng_baseDirectoryIndex =                                       \
            ::wpp::internal::getBaseDirectoryIndex(__FILE__);                                     \
        constexpr const auto ___wpp_ng_hash = __WPP_NG_CALCULATE_TRACE_HASH(                      \
            ___wpp_ng_baseDirectoryIndex, flag, level, fmt, __VA_ARGS__);                         \
        static constexpr const auto ___wpp_ng_guid = ::wpp::internal::md5ToUUID3(___wpp_ng_hash); \
        constexpr const auto ___wpp_ng_paramter_count =                                           \
            std::tuple_size_v<decltype(std::forward_as_tuple(__VA_ARGS__))>;                      \
        __WPP_NG_STRING_MAKER(FormatType, fmt);                                                   \
        using FormatInfo = decltype(::wpp::internal::getFormatInfo<FormatType>());                \
        __WPP_NG_VALIDATE_FORMAT_AND_ARGS(FormatInfo, ___wpp_ng_paramter_count, __VA_ARGS__);     \
        __WPP_NG_ANNOTATE_TRACE_INFO(___wpp_ng_hash, flag, level, fmt, FormatInfo, __VA_ARGS__);  \
        ::wpp::internal::wppNGTraceNew<decltype(FormatInfo::value()), flag, level>(               \
            provider, ___wpp_ng_guid, __VA_ARGS__);                                               \
    } while (0)
