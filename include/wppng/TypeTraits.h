#pragma once
#include <type_traits>
#include <tuple>

namespace wpp {

// Forward declaration
struct TracePair;

}  // namespace wpp

namespace wpp::internal {

/**
 * Check whether the given type is a string literal type.
 */
template<typename T>
static constexpr bool IsStringLiteral = std::is_same_v<
    T, std::add_lvalue_reference_t<const char[std::extent_v<std::remove_reference_t<T>>]>>;

/**
 * Checks that the given type is a vaild "simple" trace item:
 * - Has a `const void* getPtr()` function
 * - Has a `size_t getSize()` function.
 */
template<typename T, typename = void>
struct IsSimpleTraceItem : std::false_type {};

template<typename T>
struct IsSimpleTraceItem<
    T, std::enable_if_t<std::is_same_v<decltype(std::declval<T>().getPtr()), const void*> &&
                        std::is_same_v<decltype(std::declval<T>().getSize()), size_t>>>
    : std::true_type {};


template<typename T>
struct IsTraceTuple : std::false_type {};

template<typename... T>
struct IsTraceTuple<std::tuple<T...>> {
    static constexpr const bool value = (std::is_same_v<T, wpp::TracePair> && ...);
};

/**
 * Checks that the given type is a valid "complex" trace item:
 * - Has a `makeTracePairs()` function returning a tuple of TracePair objects.
 */
template<typename T, typename = void>
struct IsComplexTraceItem : std::false_type {};

template<typename T>
struct IsComplexTraceItem<
    T, std::enable_if_t<IsTraceTuple<decltype(std::declval<T>().makeTracePairs())>::value>>
    : std::true_type {};

}  // namespace wpp::internal