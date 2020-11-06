#include "catch.hpp"

#include "wppng/PathUtils.h"

TEST_CASE("Path", "[Paths]") {
    STATIC_REQUIRE(wpp::internal::getBaseDirectoryIndex("file.cpp") == 0);
    STATIC_REQUIRE(wpp::internal::getBaseDirectoryIndex("directory\\file.txt") == 0);
    STATIC_REQUIRE(wpp::internal::getBaseDirectoryIndex("some\\directory\\file.txt") == 5);
    STATIC_REQUIRE(wpp::internal::getBaseDirectoryIndex("C:\\some\\directory\\file.txt") == 8);
}
