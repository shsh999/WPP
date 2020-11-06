#pragma once
#include "catch.hpp"

#include "wppng/Md5.h"

using namespace wpp::internal::md5;

TEST_CASE("Computations are correct", "[MD5]") {
    SECTION("Empty string") {
        constexpr const auto sum = md5Sum("");
        // expected: d41d8cd98f00b204e9800998ecf8427e
        constexpr const auto expected = MD5Sum{0xd98c1dd4, 0x4b2008f, 0x980980e9, 0x7e42f8ec};
        STATIC_REQUIRE(sum == expected);
    }

    SECTION("Short string") {
        constexpr const auto sum = md5Sum("test");
        // expected: 098f6bcd4621d373cade4e832627b4f6
        constexpr const auto expected = MD5Sum{0xcd6b8f09, 0x73d32146, 0x834edeca, 0xf6b42726};
        STATIC_REQUIRE(sum == expected);
    }

    SECTION("String with length 56") {
        constexpr const auto sum =
            md5Sum("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        // expected: 3b0c8ac703f828b04c6c197006d17218
        constexpr const auto expected = MD5Sum{0xc78a0c3b, 0xb028f803, 0x70196c4c, 0x1872d106};
        STATIC_REQUIRE(sum == expected);
    }

    SECTION("Very long string") {
        constexpr const auto sum = md5Sum(
            "The quick brown fox jumped over the lazy dog! The quick brown fox jumped over the "
            "lazy dog! The quick brown fox jumped over the lazy dog! The quick brown fox jumped "
            "over the lazy dog! The quick brown fox jumped over the lazy dog!");
        // expected: 49d026e87d34318cd354ab828afc4273
        constexpr const auto expected = MD5Sum{0xe826d049, 0x8c31347d, 0x82ab54d3, 0x7342fc8a};
        STATIC_REQUIRE(sum == expected);
    }

    SECTION("ConstexprString computation") {
        constexpr auto sum = md5Sum(::wpp::internal::makeString(""));
        // expected: d41d8cd98f00b204e9800998ecf8427e
        constexpr const auto expected = MD5Sum{0xd98c1dd4, 0x4b2008f, 0x980980e9, 0x7e42f8ec};
        STATIC_REQUIRE(sum == expected);
    }
}
