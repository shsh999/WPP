#pragma once
#include <array>
#include <string_view>
#include <type_traits>

namespace wpp::internal {

/**
 * This is a compile-time string class for fixed-size strings.
 */
template<size_t N>
struct ConstexprString : std::array<char, N> {
    constexpr bool operator==(const ConstexprString& other) const noexcept {
        for (size_t i = 0; i < N; ++i) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    template<size_t M, std::enable_if_t<M != N, int> = 0>
    constexpr bool operator==([[maybe_unused]] const ConstexprString<M>& other) const noexcept {
        return false;
    }

    template<size_t M>
    constexpr bool operator!=(const ConstexprString<M>& other) const noexcept {
        return !(*this == other);
    }

    /**
     * Concatenates two ConstexprStrings.
     */
    template<size_t M>
    constexpr ConstexprString<N + M> operator+(const ConstexprString<M>& other) const noexcept {
        ConstexprString<N + M> result{};
        for (size_t i = 0; i < N; ++i) {
            result[i] = (*this)[i];
        }

        for (size_t i = 0; i < M; ++i) {
            result[N + i] = other[i];
        }

        return result;
    }
};

namespace internal {
template<size_t N, size_t... Ixs>
constexpr ConstexprString<N> makeString(const char* str, std::index_sequence<Ixs...>) noexcept {
    return ConstexprString<N>{str[Ixs]...};
}
}  // namespace internal

/**
 * Generators for fixed-size constexpr strings.
 */
template<size_t N>
constexpr ConstexprString<N> makeString(const char* str) noexcept {
    return internal::makeString<N>(str, std::make_index_sequence<N>());
}

template<size_t N>
constexpr ConstexprString<N - 1> makeString(const char (&str)[N]) noexcept {
    return makeString<N - 1>(static_cast<const char*>(str));
}

/**
 * This is a string class for fixed strings, exposing a string-view into the string.
 * Each different string is a unique instantiation of this class.
 */
template<char... chars>
struct FixedConstexprString {
    static constexpr const auto value() {
        return std::string_view(_value, sizeof...(chars));
    }

    static constexpr auto size() {
        return sizeof...(chars);
    }

private:
    static constexpr const char _value[sizeof...(chars)] = {chars...};
};

/**
 * A zero-size specialization, as an array of size 0 is not supported.
 */
template<>
struct FixedConstexprString<> {
    static constexpr const auto value() {
        return std::string_view("");
    }

    static constexpr auto size() {
        return 0;
    }
};

/**
 * A Generator for FixedConstexprStrings from a pointer to a string and indices.
 */
template<const char* const* str, std::size_t... Ixs>
static auto makeFixedString(std::index_sequence<Ixs...>) {
    return FixedConstexprString<(*str)[Ixs]...>();
}

template<const char* const str, std::size_t... Ixs>
static auto makeFixedString(std::index_sequence<Ixs...>) {
    return FixedConstexprString<str[Ixs]...>();
}

}  // namespace wpp::internal

/**
 * Generates a type named `name` whose static `::value()` function returns a string view of the
 * given string literal. `name` must be a valid class name, and `str` must be a string literal.
 */
#define __WPP_NG_STRING_MAKER(name, str)                                                 \
    static constexpr const auto ___wpp_ng_trace_str_##name = str;                        \
    using name = decltype(::wpp::internal::makeFixedString<&___wpp_ng_trace_str_##name>( \
        std::make_index_sequence<sizeof(str) - 1>()))
