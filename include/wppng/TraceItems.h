#pragma once
#include <cstdint>
#include <tuple>
#include <guiddef.h>
#include "TypeTraits.h"
#include "String.h"

namespace wpp {

/**
 * This template is used in order to provide a customization interface for trace formats.
 * See examples in this file.
 */
template<typename T>
struct TraceItemMaker;

/**
 * This struct should be returned from `make` functions in order to indicate that the given format
 * is not supported.
 */
struct InvalidFormatItem {};

/**
 * This structure is a pair containing trace information, used by complex trace items to return
 * multiple trace items.
 */
struct TracePair {
    const void* ptr;
    size_t size;
};

template<typename T>
constexpr auto makeTracePairs(const T& t) {
    if constexpr (internal::IsSimpleTraceItem<T>::value) {
        return std::make_tuple(t.getPtr(), t.getSize());
    } else if constexpr (internal::IsComplexTraceItem<T>::value) {
        return std::apply(
            [](auto&&... args) { return std::tuple_cat(std::make_tuple(args.ptr, args.size)...); },
            t.makeTracePairs());
    } else {
        static_assert(false, "Bad trace item type!");
    }
}

template<typename T>
struct PODTraceItem {
    constexpr const void* getPtr() const noexcept {
        return &value;
    }

    constexpr size_t getSize() const noexcept {
        return sizeof(T);
    }

    static constexpr bool isValidFormat(std::string_view str) {
        if (str.size() == 0) {
            return true;
        }
        if (str.size() != 1) {
            return false;
        }
        switch (str[0]) {
            case 'd':
            case 'x':
            case 'X':
            case 'b':
            case 'B':
            case 'o':
                return true;
            default:
                return false;
        }
    }

    T value;
};

#define WPP_NG_SPECIALIZE_TRIVIAL_LOGGER(RealType, Tag)          \
    template<>                                                   \
    struct TraceItemMaker<RealType> {                            \
        static constexpr auto make(RealType v) {                 \
            return Tag{v};                                       \
        }                                                        \
        template<typename Format>                                \
        static constexpr auto make(RealType v) {                 \
            if constexpr (Tag::isValidFormat(Format::value())) { \
                return Tag{v};                                   \
            } else {                                             \
                return InvalidFormatItem{};                      \
            }                                                    \
        }                                                        \
    }

#define WPP_NG_SPECIALIZE_VARIABLE_SIZE_LOGGER(RealType, Tag, VariableSizeTag)             \
    template<>                                                                             \
    struct TraceItemMaker<RealType> {                                                      \
        static constexpr auto make(RealType v) {                                           \
            return Tag{v};                                                                 \
        }                                                                                  \
        template<typename Format>                                                          \
        static constexpr auto make(RealType v) {                                           \
            if constexpr (sizeof(RealType) == sizeof(size_t) && Format::size() > 0 &&      \
                          Format::value()[0] == 'z') {                                     \
                if constexpr (VariableSizeTag::isValidFormat(Format::value().substr(1))) { \
                    return VariableSizeTag{v};                                             \
                } else {                                                                   \
                    return InvalidFormatItem;                                              \
                }                                                                          \
            } else if constexpr (Tag::isValidFormat(Format::value())) {                    \
                return Tag{v};                                                             \
            } else {                                                                       \
                return InvalidFormatItem{};                                                \
            }                                                                              \
        }                                                                                  \
    }

struct Int8Item : PODTraceItem<int8_t> {};
WPP_NG_SPECIALIZE_TRIVIAL_LOGGER(int8_t, Int8Item);

struct Int16Item : PODTraceItem<int16_t> {};
WPP_NG_SPECIALIZE_TRIVIAL_LOGGER(int16_t, Int16Item);

struct PtrDiffItem : PODTraceItem<ptrdiff_t> {};
struct SizeTItem : PODTraceItem<size_t> {};

struct Int32Item : PODTraceItem<int32_t> {};
WPP_NG_SPECIALIZE_VARIABLE_SIZE_LOGGER(int32_t, Int32Item, PtrDiffItem);

struct Int64Item : PODTraceItem<int64_t> {};
WPP_NG_SPECIALIZE_VARIABLE_SIZE_LOGGER(int64_t, Int64Item, PtrDiffItem);

struct UInt8Item : PODTraceItem<uint8_t> {};
WPP_NG_SPECIALIZE_TRIVIAL_LOGGER(uint8_t, UInt8Item);

struct UInt16Item : PODTraceItem<uint16_t> {};
WPP_NG_SPECIALIZE_TRIVIAL_LOGGER(uint16_t, UInt16Item);

struct UInt32Item : PODTraceItem<uint32_t> {};
WPP_NG_SPECIALIZE_VARIABLE_SIZE_LOGGER(uint32_t, UInt32Item, SizeTItem);

struct UInt64Item : PODTraceItem<uint64_t> {};
WPP_NG_SPECIALIZE_VARIABLE_SIZE_LOGGER(uint64_t, UInt64Item, SizeTItem);

struct ByteItem : PODTraceItem<std::byte> {};
WPP_NG_SPECIALIZE_TRIVIAL_LOGGER(std::byte, ByteItem);

struct GuidItem {
    constexpr const void* getPtr() const noexcept {
        return &value;
    }

    constexpr size_t getSize() const noexcept {
        return sizeof(GUID);
    }

    const GUID& value;
};

template<>
struct TraceItemMaker<GUID> {
    static constexpr auto make(const GUID& guid) {
        return GuidItem{guid};
    }
};

template<typename CharType>
struct CharTypeItem : PODTraceItem<CharType> {
    static constexpr bool isValidFormat(std::string_view str) {
        if (str.size() != 1) {
            return false;
        }
        return PODTraceItem<CharType>::isValidFormat(str) || str[0] == 'c';
    }
};

struct CharItem : CharTypeItem<char> {};
WPP_NG_SPECIALIZE_TRIVIAL_LOGGER(char, CharItem);

struct WCharItem : CharTypeItem<wchar_t> {};
WPP_NG_SPECIALIZE_TRIVIAL_LOGGER(wchar_t, WCharItem);

struct PointerItem : PODTraceItem<const void*> {};

template<typename T>
struct TraceItemMaker<T*> {
    static constexpr auto make(T* ptr) {
        return PointerItem{static_cast<const void*>(ptr)};
    }

    template<typename Format>
    static constexpr auto make(T* ptr) {
        if constexpr (Format::value() == "p") {
            return PointerItem{ptr};
        } else {
            return InvalidFormatItem{};
        }
    }
};

struct SizeAndDataItem {
    constexpr SizeAndDataItem(const void* data, uint16_t size) : ptr(data), size(size) {
    }

    constexpr auto makeTracePairs() const {
        return std::make_tuple<TracePair, TracePair>({&size, sizeof(size)}, {ptr, size});
    }

private:
    const void* const ptr;
    const uint16_t size;
};

struct HexBufferItem : public SizeAndDataItem {
    using SizeAndDataItem::SizeAndDataItem;
};

struct HexDumpItem : public SizeAndDataItem {
    using SizeAndDataItem::SizeAndDataItem;
};

namespace internal {

template<typename CharType>
struct StringItemType {
    explicit constexpr StringItemType(const CharType* const str)
        : StringItemType(str, sizeof(CharType) * (std::char_traits<CharType>::length(str) + 1)) {
    }

    explicit constexpr StringItemType(const CharType* const ptr, size_t byteSize)
        : ptr(ptr), size(byteSize) {
    }

    constexpr const void* getPtr() const noexcept {
        return ptr;
    }

    constexpr size_t getSize() const noexcept {
        return size;
    }

    const CharType* const ptr;
    const size_t size;
};

template<typename CharType, typename DefaultStringType>
struct StringTraceItemMaker {
    static constexpr auto make(const CharType* ptr) {
        return DefaultStringType(ptr);
    }

    template<typename Format>
    static constexpr auto make(const CharType* ptr) {
        return makeStringType<Format>(ptr);
    }

    template<typename Format, size_t N>
    static constexpr auto make(const CharType (&str)[N]) {
        return makeStringType<Format, N>(str);
    }

private:
    template<typename Format, size_t size = -1>
    static constexpr auto makeStringType(const CharType* str) {
        constexpr const auto format = Format::value();
        if constexpr (format == "" || format == "s") {
            if constexpr (size == -1) {
                return DefaultStringType(str);
            } else {
                return DefaultStringType(str, size * sizeof(CharType));
            }
        } else if constexpr (format == "p") {
            return PointerItem{str};
        } else if constexpr (format == "x" || format == "xd") {
            // Don't include null-terminator in hex dumps
            using ResultType = std::conditional_t<format == "x", HexBufferItem, HexDumpItem>;
            if constexpr (size == -1) {
                return ResultType(str,
                                  static_cast<uint16_t>(std::char_traits<CharType>::length(str) *
                                                        sizeof(CharType)));
            } else {
                return ResultType(str, static_cast<uint16_t>(size - 1) * sizeof(CharType));
            }
        } else {
            return InvalidFormatItem{};
        }
    }
};

}  // namespace internal

struct StringItem : internal::StringItemType<char> {
    using StringItemType::StringItemType;
};

struct WStringItem : internal::StringItemType<wchar_t> {
    using StringItemType::StringItemType;
};

template<>
struct TraceItemMaker<char*> : internal::StringTraceItemMaker<char, StringItem> {};

template<>
struct TraceItemMaker<const char*> : TraceItemMaker<char*> {};

template<>
struct TraceItemMaker<wchar_t*> : internal::StringTraceItemMaker<wchar_t, WStringItem> {};

template<>
struct TraceItemMaker<const wchar_t*> : TraceItemMaker<wchar_t*> {};

}  // namespace wpp

namespace wpp::internal {

struct TypeDoesNotSupportFormatting {};

template<typename T, typename Format, typename Arg, typename = void>
struct HasTemplateMake : std::false_type {};

template<typename T, typename Format, typename Arg>
struct HasTemplateMake<T, Format, Arg,
                       std::void_t<decltype(T::template make<Format>(std::declval<Arg>()))>>
    : std::bool_constant<!std::is_same_v<decltype(T::template make<Format>(std::declval<Arg>())),
                                         InvalidFormatItem>> {};

template<typename T, typename Arg, typename = void>
struct HasRegularMake : std::false_type {};

template<typename T, typename Arg>
struct HasRegularMake<T, Arg, std::void_t<decltype(T::make(std::declval<Arg>()))>>
    : std::true_type {};

/**
 * Build the matching trace item for the given type and format.
 */
template<typename Format, typename T>
auto buildTraceItem(T&& t) {
    using MakerType = TraceItemMaker<std::decay_t<T>>;
    constexpr const auto hasRegularMake = HasRegularMake<MakerType, T>::value;
    constexpr const auto hasTemplateMake = HasTemplateMake<MakerType, Format, T>::value;

    if constexpr (!hasTemplateMake && !hasRegularMake) {
        // Formatting not supported at all!
        return TypeDoesNotSupportFormatting{};
    } else if constexpr (hasTemplateMake && !(hasRegularMake && Format::value() == "")) {
        return MakerType::template make<Format>(std::forward<T>(t));
    } else if constexpr (Format::value() == "") {
        return MakerType::make(std::forward<T>(t));
    } else {
        return InvalidFormatItem{};
    }
}

}  // namespace wpp::internal