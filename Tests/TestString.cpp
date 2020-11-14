#include "catch.hpp"

#include "wpp/String.h"

using namespace wpp::internal;

TEST_CASE("String initialization from string literal", "[String]") {
    constexpr auto string = makeString("Hello");
    
    STATIC_REQUIRE(string.size() == 5);
    STATIC_REQUIRE(string[0] == 'H');
    STATIC_REQUIRE(string[1] == 'e');
    STATIC_REQUIRE(string[2] == 'l');
    STATIC_REQUIRE(string[3] == 'l');
    STATIC_REQUIRE(string[4] == 'o');
}

TEST_CASE("Comparison", "[String]") {
    constexpr const auto s1 = makeString("aaaa");
    constexpr const auto s2 = makeString("bbbb");
    constexpr const auto s3 = makeString("long string");

    SECTION("operator==") {
        STATIC_REQUIRE(s1 == s1);
        STATIC_REQUIRE(s1 == makeString("aaaa"));
        STATIC_REQUIRE_FALSE(s1 == s2);
        STATIC_REQUIRE_FALSE(s1 == s3);
    }

    SECTION("operator!=") {
        STATIC_REQUIRE(s1 != s2);
        STATIC_REQUIRE(s1 != s3);
        STATIC_REQUIRE_FALSE(s1 != makeString("aaaa"));
        STATIC_REQUIRE_FALSE(s1 != s1);
    }
}

TEST_CASE("String concatanation", "[String]") {
    constexpr const auto s1 = makeString("Hello ");
    constexpr const auto s2 = makeString("World!");
    constexpr const auto sum = s1 + s2;
    constexpr const auto expected = makeString("Hello World!");
    
    STATIC_REQUIRE((std::is_same_v<std::decay_t<decltype(sum)>, ConstexprString<s1.size() + s2.size()>>));
    STATIC_REQUIRE(sum.size() == s1.size() + s2.size());
    STATIC_REQUIRE(sum == expected);
}

TEST_CASE("Fixed strings", "[String]") {
    __WPP_STRING_MAKER(WordType, "WORD");
    STATIC_REQUIRE(WordType::value() == "WORD");

    __WPP_STRING_MAKER(WormType, "WORM");
    STATIC_REQUIRE(WormType::value() == "WORM");

    STATIC_REQUIRE(!std::is_same_v<WormType, WordType>);

    __WPP_STRING_MAKER(WordType2, "WORD");
    STATIC_REQUIRE(WordType2::value() == "WORD");

    STATIC_REQUIRE(std::is_same_v<WordType, WordType2>);
}