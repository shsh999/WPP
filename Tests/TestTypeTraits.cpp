#include "catch.hpp"

#include "wppng/TypeTraits.h"
#include "wppng/TraceItems.h"

using namespace wpp::internal;
using namespace wpp;

TEST_CASE("isStringLiteral", "[TypeTraits]") {
    STATIC_REQUIRE(IsStringLiteral<decltype("asd")>);

    constexpr const auto str = "asd";
    STATIC_REQUIRE_FALSE(IsStringLiteral<decltype(str)>);
    
    STATIC_REQUIRE_FALSE(IsStringLiteral<void>);
    STATIC_REQUIRE_FALSE(IsStringLiteral<int>);
    STATIC_REQUIRE_FALSE(IsStringLiteral<const char*>);
    STATIC_REQUIRE_FALSE(IsStringLiteral<decltype(L"asd")>);
}

TEST_CASE("SimpleTraceItem", "[TypeTraits]") {

    struct SimpleTraceItem {
        constexpr const void* getPtr() const noexcept {
            return nullptr;
        }

        constexpr const size_t getSize() const noexcept {
            return 0;
        }
    };

    // getPtr does not return a const void*
    struct NotSimplePtr {
        void* getPtr() {
            return nullptr;
        }
        size_t getSize() {
            return 0;
        }
    };
    
    // getSize does not return a size_t, but a signed type
    struct NotSimpleSize {
        const void* getPtr() {
            return nullptr;
        }
        int64_t getSize() {
            return 0;
        }
    };

    

    STATIC_REQUIRE(IsSimpleTraceItem<SimpleTraceItem>::value);
    
    STATIC_REQUIRE_FALSE(IsSimpleTraceItem<NotSimplePtr>::value);
    STATIC_REQUIRE_FALSE(IsSimpleTraceItem<NotSimpleSize>::value);
    STATIC_REQUIRE_FALSE(IsSimpleTraceItem<int>::value);
}

TEST_CASE("ComplexTraceItem", "[TypeTraits]") {

    struct ComplexTraceItem {
        auto makeTracePairs() {
            return std::make_tuple(wpp::TracePair{nullptr, size_t(5)});
        }
    };

    struct ComplexTraceItemManyItems {
        auto makeTracePairs() {
            return std::make_tuple(wpp::TracePair{nullptr, size_t(5)}, wpp::TracePair{nullptr, size_t(5)},
                            wpp::TracePair{nullptr, size_t(5)}, wpp::TracePair{nullptr, size_t(5)});
        }
    };

    struct ComplexTraceItemNoItems {
        auto makeTracePairs() {
            return std::make_tuple();
        }
    };


    STATIC_REQUIRE(IsComplexTraceItem<ComplexTraceItem>::value);
    STATIC_REQUIRE(IsComplexTraceItem<ComplexTraceItemManyItems>::value);
    STATIC_REQUIRE(IsComplexTraceItem<ComplexTraceItemNoItems>::value);
    
    STATIC_REQUIRE_FALSE(IsComplexTraceItem<int>::value);
}
