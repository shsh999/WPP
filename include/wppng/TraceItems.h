#pragma once
#include <cstdint>
#include <tuple>
#include <guiddef.h>
#include "TypeTraits.h"
#include "String.h"

namespace wpp {

template<char... chars>
using FormatString = internal::FixedConstexprString<chars...>;

/**
 * This template is used in order to provide a customization interface for trace formats.
 * See examples in this file.
 */
template<typename T, typename Format, typename Enable = void>
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

    T value;
};

constexpr bool isValidIntegerFormat(std::string_view format) {
    if (format.size() == 0) {
        return true;
    }

    if (format.size() != 1) {
        return false;
    }

    switch (format[0]) {
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

#define WPP_NG_SPECIALIZE_LOGGER(RealType, Tag, formatValidationFuncion)                 \
    template<typename Format>                                                            \
    struct TraceItemMaker<RealType, Format,                                             \
                           std::enable_if_t<formatValidationFuncion(Format::value())>> { \
        static constexpr auto make(RealType value) {                                     \
            return Tag{value};                                                           \
        }                                                                                \
    }

#define WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(RealType, Tag) \
    WPP_NG_SPECIALIZE_LOGGER(RealType, Tag, isValidIntegerFormat)


constexpr bool isValidPointerSizedIntegerFormat(std::string_view format) {
    return format.size() > 0 && format[0] == 'z' &&
           isValidIntegerFormat(format.substr(1));
}


struct PtrDiffItem : PODTraceItem<ptrdiff_t> {};
WPP_NG_SPECIALIZE_LOGGER(ptrdiff_t, PtrDiffItem, isValidPointerSizedIntegerFormat);

struct SizeTItem : PODTraceItem<size_t> {};
WPP_NG_SPECIALIZE_LOGGER(size_t, SizeTItem, isValidPointerSizedIntegerFormat);


struct Int8Item : PODTraceItem<int8_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(int8_t, Int8Item);

struct Int16Item : PODTraceItem<int16_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(int16_t, Int16Item);


struct Int32Item : PODTraceItem<int32_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(int32_t, Int32Item);

struct Int64Item : PODTraceItem<int64_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(int64_t, Int64Item);

struct UInt8Item : PODTraceItem<uint8_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(uint8_t, UInt8Item);

struct UInt16Item : PODTraceItem<uint16_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(uint16_t, UInt16Item);

struct UInt32Item : PODTraceItem<uint32_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(uint32_t, UInt32Item);

struct UInt64Item : PODTraceItem<uint64_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(uint64_t, UInt64Item);

struct ByteItem : PODTraceItem<std::byte> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(std::byte, ByteItem);


struct GuidItem {
    constexpr const void* getPtr() const noexcept {
        return &value;
    }

    constexpr size_t getSize() const noexcept {
        return sizeof(GUID);
    }

    const GUID& value;
};

template<typename Format>
struct TraceItemMaker<GUID, Format, std::enable_if_t<Format::size() == 0>> {
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

constexpr bool isValidCharacterFormat(std::string_view format) {
    return format == "c" || isValidIntegerFormat(format);
}

struct CharItem : CharTypeItem<char> {};
WPP_NG_SPECIALIZE_LOGGER(char, CharItem, isValidCharacterFormat);

struct WCharItem : CharTypeItem<wchar_t> {};
WPP_NG_SPECIALIZE_LOGGER(wchar_t, WCharItem, isValidCharacterFormat);

struct PointerItem : PODTraceItem<const void*> {};

constexpr bool isValidPointerFormat(std::string_view format) {
    return format.size() == 0 || format == "p";
}

template<typename T, typename Format>
struct TraceItemMaker<T*, Format, std::enable_if_t<isValidPointerFormat(Format::value())>> {
    static constexpr auto make(const T* ptr) {
        return PointerItem{static_cast<const void*>(ptr)};
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

template<typename CharType, typename ItemType, bool includeNull = true>
struct StringTraceItemMaker2 {
    static constexpr auto make(const CharType* const ptr) {
        if constexpr (includeNull) {
            return ItemType(ptr, (std::char_traits<CharType>::length(ptr) + 1) * sizeof(CharType));
        } else {
            return ItemType(ptr, static_cast<uint16_t>(std::char_traits<CharType>::length(ptr) * sizeof(CharType)));
        }
    }

    template<size_t N>
    static constexpr auto make(const CharType (&str)[N]) {
        if constexpr (includeNull) {
            return ItemType(str, sizeof(CharType) * N);
        } else {
            return ItemType(str, sizeof(CharType) * (N - 1));
        }
    }
};

template<>
struct TraceItemMaker<char*, FormatString<>> : StringTraceItemMaker2<char, StringItem> {};

template<>
struct TraceItemMaker<char*, FormatString<'s'>> : StringTraceItemMaker2<char, StringItem> {};

template<>
struct TraceItemMaker<char*, FormatString<'x'>> : StringTraceItemMaker2<char, HexBufferItem, false> {};

template<>
struct TraceItemMaker<char*, FormatString<'x', 'd'>>
    : StringTraceItemMaker2<char, HexDumpItem, false> {};


template<>
struct TraceItemMaker<wchar_t*, FormatString<>> : StringTraceItemMaker2<wchar_t, WStringItem> {};

template<>
struct TraceItemMaker<wchar_t*, FormatString<'s'>> : StringTraceItemMaker2<wchar_t, WStringItem> {};

template<>
struct TraceItemMaker<wchar_t*, FormatString<'x'>>
    : StringTraceItemMaker2<wchar_t, HexBufferItem, false> {};

template<>
struct TraceItemMaker<wchar_t*, FormatString<'x', 'd'>>
    : StringTraceItemMaker2<wchar_t, HexDumpItem, false> {};


}  // namespace wpp

namespace wpp::internal {

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
    using MakerType = TraceItemMaker<RecursiveDecay<T>, RecursiveDecay<Format>>;
    constexpr const auto hasMake2 = HasRegularMake<MakerType, T>::value;
    if constexpr (!hasMake2) {
        return InvalidFormatItem{};
    } else {
        return MakerType::make(std::forward<T>(t));
    }
}

}  // namespace wpp::internal