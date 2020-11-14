#pragma once

namespace wpp::internal {

/**
 * Find the index of the base directory of the given file path.
 * For example, if the path is "C:\Users\user\Desktop\project\file1.cpp", the result would be 22
 * (the index of 'p' of the word "project").
 *
 * In the case of a relative path not including a directory name, 0 is returned (the beginning of
 * the path).
 *
 * This is used to generate unique trace hashes while keeping the build reproducible.
 */
template<size_t N>
constexpr size_t getBaseDirectoryIndex(const char (&file)[N]) {
    constexpr const auto len = N - 1;
    size_t index = 0;
    size_t prevIndex = 0;
    for (size_t i = 0; i < len; ++i) {
        if (file[i] == '\\') {
            prevIndex = index;
            index = i + 1;  // point to the character after
        }
    }

    return prevIndex;
}

}  // namespace wpp::internal