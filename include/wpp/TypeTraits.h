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

namespace helper {
template<typename T>
struct RecursiveDecayHelper {
    using type = std::decay_t<T>;
};

template<typename T>
struct RecursiveDecayHelper<T*> {
    using type = std::add_pointer_t<typename RecursiveDecayHelper<std::decay_t<T>>::type>;
};

template<typename T>
struct RecursiveDecayHelper<T* const> {
    using type = std::add_pointer_t<typename RecursiveDecayHelper<std::decay_t<T>>::type>;
};

template<typename T>
struct RecursiveDecayHelper<T* volatile> {
    using type = std::add_pointer_t<typename RecursiveDecayHelper<std::decay_t<T>>::type>;
};

template<typename T>
struct RecursiveDecayHelper<T* const volatile> {
    using type = std::add_pointer_t<typename RecursiveDecayHelper<std::decay_t<T>>::type>;
};

template<typename T>
struct RecursiveDecayHelper<T&> {
    using type = typename RecursiveDecayHelper<std::decay_t<T>>::type;
};

template<typename T>
struct RecursiveDecayHelper<T&&> {
    using type = typename RecursiveDecayHelper<std::decay_t<T>>::type;
};

template<typename T, std::size_t N>
struct RecursiveDecayHelper<T[N]> {
    using type = std::add_pointer_t<typename RecursiveDecayHelper<std::decay_t<T>>::type>;
};

template<typename T>
struct RecursiveDecayHelper<T[]> {
    using type = std::add_pointer_t<typename RecursiveDecayHelper<std::decay_t<T>>::type>;
};

}  // namespace helper

/**
 * Recursively decay a type, removing all const, volatile and reference qualifiers.
 * For example, `RecursiveDecay<const int* const * const>` yields `int**`.
 */
template<typename T>
using RecursiveDecay = typename helper::RecursiveDecayHelper<T>::type;

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

/**
 * Checks that the given type is a valid TraceItemMaker instantiation: it has a valid make function
 * taking an argument of the given type.
 */
template<typename Maker, typename Arg, typename = void>
struct HasMakeFunction : std::false_type {};

template<typename Maker, typename Arg>
struct HasMakeFunction<Maker, Arg, std::void_t<decltype(Maker::make(std::declval<Arg>()))>>
    : std::true_type {};

}  // namespace wpp::internal