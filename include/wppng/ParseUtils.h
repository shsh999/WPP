#pragma once
#include "String.h"
#include "TypeTraits.h"

namespace wpp::internal {

constexpr bool isDoubleOpenBracket(const char* str, size_t size) {
    return *str == '{' && size > 1 && str[1] == '{';
}

constexpr bool isDoubleCloseBracket(const char* str, size_t size) {
    return *str == '}' && size > 1 && str[1] == '}';
}

constexpr bool isOpenBracket(const char* str, size_t size) {
    return *str == '{' && (size <= 1 || str[1] != '{');
}

constexpr bool isCloseBracket(const char* str) {
    return *str == '}';
}

enum class ArgumentParseStatus { Success, ExcessOpens, ExcessCloses, MissingColonInFormat };

struct SingleArgumentResult {
    ArgumentParseStatus status;
    /// The start of the format specifier
    const char* start;
    /// The end of the format specifier
    const char* end;
    /// The number of characters processed before returning the result (typically more than
    /// `end - start`, as you have to add any non-format characters, `{:` and `}`)
    size_t charsProcessed;
};

/**
 * Get the next argument information from the given format string.
 */
constexpr SingleArgumentResult getSingleArgument(const char* str, size_t size) {
    size_t i = 0;
    while (i < size) {
        if (isOpenBracket(str + i, size - i)) {
            ++i;  // Skip the bracket
            const char* argStart = str + i;
            while (i < size && !isCloseBracket(str + i)) {
                ++i;
            }
            if (i >= size) {
                return {ArgumentParseStatus::ExcessOpens, nullptr, nullptr, 0};
            }

            const char* argEnd = str + i;
            if (argStart != argEnd) {
                if (*argStart != ':') {
                    return {ArgumentParseStatus::MissingColonInFormat, nullptr, nullptr, 0};
                }
                ++argStart;
            }
            // Got to a closing bracket!
            return {ArgumentParseStatus::Success, argStart, argEnd, i + 1};
        } else if (isDoubleOpenBracket(str + i, size - i) ||
                   isDoubleCloseBracket(str + i, size - i)) {
            i += 2;
        } else if (isCloseBracket(str + i)) {
            return {ArgumentParseStatus::ExcessCloses, nullptr, nullptr, 0};
        } else {
            ++i;
        }
    }
    return {ArgumentParseStatus::Success, nullptr, nullptr, i};
}

struct CountArgsResult {
    ArgumentParseStatus status;
    size_t count;
};

/**
 * Count the number of arguments found in the given string.
 * If the format string is invalid, an invalid status is returned with a count of 0.
 */
constexpr CountArgsResult countArgs(std::string_view format) {
    const auto size = format.size();
    const auto* str = format.data();
    size_t count = 0;
    size_t charsProcessed = 0;
    while (charsProcessed < size) {
        const auto result = getSingleArgument(str + charsProcessed, size - charsProcessed);

        if (result.status != ArgumentParseStatus::Success) {
            return {result.status, 0};
        }

        charsProcessed += result.charsProcessed;

        if (result.start == nullptr) {
            continue;
        }

        ++count;
    }

    return {ArgumentParseStatus::Success, count};
}

/**
 * Get an array of format specifiers from the given format string.
 * It is assumed that the number of arguments is known in advance and that the format string is
 * valid.
 */
template<size_t argCount>
constexpr auto getFormatInfo(std::string_view format) {
    std::array<std::string_view, argCount> result{};
    auto size = format.size();
    const auto* str = format.data();

    for (size_t i = 0; i < argCount; ++i) {
        // This will never fail, as getFormatInfo is always called after countArgs(), that validates
        // all arguments are correct.
        const auto argInfo = wpp::internal::getSingleArgument(str, size);

        str += argInfo.charsProcessed;
        size -= argInfo.charsProcessed;

        result[i] = {argInfo.start, static_cast<size_t>(argInfo.end - argInfo.start)};
    }

    return result;
}

/**
 * Convert the string_view at index Index in the given format to FixedConstexprString.
 */
template<typename FormatInfo, size_t Index, size_t... Ixs>
constexpr auto makeFixedString(std::index_sequence<Ixs...>) {
    static constexpr const auto format = FormatInfo::formatArray()[Index];
    static constexpr const auto data = format.data();
    return FixedConstexprString<data[Ixs]...>();
}

/**
 * Convert the given format info from an array of string views to a tuple of FixedConstexprStrings.
 */
template<typename FormatInfo, size_t... Ixs>
constexpr auto convertFormatInfo(std::index_sequence<Ixs...>) {
    static constexpr const auto formatArray = FormatInfo::formatArray();
    return std::make_tuple(
        makeFixedString<FormatInfo, Ixs>(std::make_index_sequence<formatArray[Ixs].size()>())...);
}

/**
 * Parse format information from the given template FixedConstexprString into a tuple of
 * FixedConstexprString representing the format specifiers.
 *
 * The function is used as a way to get compile-time information from a string, that should be
 * usable in compile-time - therefore the type of the result is more important than the value of the
 * result.
 */
template<typename StringType>
constexpr auto getFormatInfo() {
    struct FormatInfoType {
    private:
        static constexpr const auto countResult() {
            return countArgs(StringType::value());
        }

    public:
        /**
         * Get the argument count status for the given format string.
         */
        static constexpr const auto status() {
            return countResult().status;
        }

        /**
         * Get the number of arguments in the given format string.
         */
        static constexpr const auto count() {
            return countResult().count;
        }

        /**
         * Get an array of format specifier string-views.
         * For internal use only (but can't be private because it is required by a template, and a
         * template can't be used in locally defined classes)
         */
        static constexpr const auto formatArray() {
            return getFormatInfo<count()>(StringType::value());
        }

        /**
         * Get a tuple of the format specifiers.
         */
        static constexpr const auto value() {
            return convertFormatInfo<FormatInfoType>(std::make_index_sequence<count()>());
        }
    };

    return FormatInfoType{};
}

}  // namespace wpp::internal