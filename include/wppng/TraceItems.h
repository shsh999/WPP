#pragma once
#include <cstdint>
#include <tuple>
#include <guiddef.h>
#include "TypeTraits.h"
#include "String.h"

namespace wpp {

/**
 * A fixed constexpr string, whose value is the concatenation of its character template arguments.
 * 
 * For example, FormatString<'a', 'b', 'c'>::value() == "abc"
 */
template<char... chars>
using FormatString = internal::FixedConstexprString<chars...>;

/**
 * This template is used in order to provide a customization interface for trace formats.
 * See examples in this file.
 */
template<typename T, typename Format = FormatString<>, typename Enable = void>
struct TraceItemMaker;

/**
 * This structure is a pair containing trace information, used by complex trace items to return
 * multiple trace items.
 */
struct TracePair {
    const void* ptr;
    size_t size;
};

/**
 * A convenience base-class for trace items holding cheap to copy data.
 */
template<typename T>
struct TrivialTraceItem {
    constexpr const void* getPtr() const noexcept {
        return &value;
    }

    constexpr size_t getSize() const noexcept {
        return sizeof(T);
    }

    T value;
};


/**
 * A convenience base-class for trace items holding a reference for an existing type.
 */
template<typename T>
using ReferenceTraceItem = TrivialTraceItem<const T&>;

/**
 * A convenience base-class for trace items that are traced as a size followed by the data.
 */
struct SizeAndDataTraceItem {
    constexpr SizeAndDataTraceItem(const void* data, uint16_t size) : ptr(data), size(size) {
        // Intentionally left blank.
    }

    constexpr auto makeTracePairs() const {
        return std::make_tuple<TracePair, TracePair>({&size, sizeof(size)}, {ptr, size});
    }

private:
    const void* const ptr;
    const uint16_t size;
};

namespace internal {

/**
 * Checks that the given format is a valid integer format.
 */
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

/**
 * Checks whether the given format is a valid character format.
 */
constexpr bool isValidCharacterFormat(std::string_view format) {
    return format == "c" || isValidIntegerFormat(format);
}

/**
 * Checks whether the given format is a valid size_t/ptrdiff_t format: a format starting with 'z'
 * followed by a valid integer format.
 */
constexpr bool isValidPointerSizedIntegerFormat(std::string_view format) {
    return format.size() > 0 && format[0] == 'z' && isValidIntegerFormat(format.substr(1));
}

/**
 * Checks whether the given format is a valid pointer format.
 */
constexpr bool isValidPointerFormat(std::string_view format) {
    return format.size() == 0 || format == "p";
}

/**
 * Checks whether the given format is a valid floating-point number format.
 */
constexpr bool isValidFloatFormat(std::string_view format) {
    if (format.size() == 0) {
        return true;
    }

    if (format.size() != 1) {
        return false;
    }

    switch (format[0]) {
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
            return true;
        default:
            return false;
    }
}

}  // namespace internal

#define WPP_NG_SPECIALIZE_LOGGER(RealType, Tag, formatValidationFuncion)                \
    template<typename Format>                                                           \
    struct TraceItemMaker<RealType, Format,                                             \
                          std::enable_if_t<formatValidationFuncion(Format::value())>> { \
        static constexpr auto make(RealType value) {                                    \
            return Tag{value};                                                          \
        }                                                                               \
    }

#define WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(RealType, Tag) \
    WPP_NG_SPECIALIZE_LOGGER(RealType, Tag, internal::isValidIntegerFormat)

struct Int8Item : TrivialTraceItem<int8_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(int8_t, Int8Item);

struct Int16Item : TrivialTraceItem<int16_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(int16_t, Int16Item);

struct Int32Item : TrivialTraceItem<int32_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(int32_t, Int32Item);

struct Int64Item : TrivialTraceItem<int64_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(int64_t, Int64Item);

struct UInt8Item : TrivialTraceItem<uint8_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(uint8_t, UInt8Item);

struct UInt16Item : TrivialTraceItem<uint16_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(uint16_t, UInt16Item);

struct UInt32Item : TrivialTraceItem<uint32_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(uint32_t, UInt32Item);

struct UInt64Item : TrivialTraceItem<uint64_t> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(uint64_t, UInt64Item);

struct ByteItem : TrivialTraceItem<std::byte> {};
WPP_NG_SPECIALIZE_INTEGRAL_LOGGER(std::byte, ByteItem);

struct PtrDiffItem : TrivialTraceItem<ptrdiff_t> {};
WPP_NG_SPECIALIZE_LOGGER(ptrdiff_t, PtrDiffItem, internal::isValidPointerSizedIntegerFormat);

struct SizeTItem : TrivialTraceItem<size_t> {};
WPP_NG_SPECIALIZE_LOGGER(size_t, SizeTItem, internal::isValidPointerSizedIntegerFormat);

struct FloatItem : TrivialTraceItem<float> {};
WPP_NG_SPECIALIZE_LOGGER(float, FloatItem, internal::isValidFloatFormat);

struct DoubleItem : TrivialTraceItem<double> {};
WPP_NG_SPECIALIZE_LOGGER(double, DoubleItem, internal::isValidFloatFormat);

struct LongDoubleItem : TrivialTraceItem<long double> {};
WPP_NG_SPECIALIZE_LOGGER(long double, LongDoubleItem, internal::isValidFloatFormat);

struct GuidItem : ReferenceTraceItem<GUID> {};

template<>
struct TraceItemMaker<GUID> {
    static constexpr auto make(const GUID& guid) {
        return GuidItem{guid};
    }
};

struct CharItem : TrivialTraceItem<char> {};
WPP_NG_SPECIALIZE_LOGGER(char, CharItem, internal::isValidCharacterFormat);

struct WCharItem : TrivialTraceItem<wchar_t> {};
WPP_NG_SPECIALIZE_LOGGER(wchar_t, WCharItem, internal::isValidCharacterFormat);

struct PointerItem : TrivialTraceItem<const void*> {};

template<typename T, typename Format>
struct TraceItemMaker<T*, Format, std::enable_if_t<internal::isValidPointerFormat(Format::value())>> {
    static constexpr auto make(const T* ptr) {
        return PointerItem{static_cast<const void*>(ptr)};
    }
};

struct HexBufferItem : public SizeAndDataTraceItem {
    using SizeAndDataTraceItem::SizeAndDataTraceItem;
};

struct HexDumpItem : public SizeAndDataTraceItem {
    using SizeAndDataTraceItem::SizeAndDataTraceItem;
};

namespace internal {

template<typename CharType>
struct NullTerminatedStringItemType {
    explicit constexpr NullTerminatedStringItemType(const CharType* const ptr, size_t byteSize)
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

}  // namespace internal

struct StringItem : internal::NullTerminatedStringItemType<char> {
    using NullTerminatedStringItemType::NullTerminatedStringItemType;
};

struct WStringItem : internal::NullTerminatedStringItemType<wchar_t> {
    using NullTerminatedStringItemType::NullTerminatedStringItemType;
};

template<typename CharType, typename ItemType, bool includeNull = true>
struct StringTraceItemMaker {
    static constexpr auto make(const CharType* const ptr) {
        if constexpr (includeNull) {
            return ItemType(ptr, (std::char_traits<CharType>::length(ptr) + 1) * sizeof(CharType));
        } else {
            return ItemType(ptr, static_cast<uint16_t>(std::char_traits<CharType>::length(ptr) *
                                                       sizeof(CharType)));
        }
    }
};

// Specialization for null-terminated strings

template<>
struct TraceItemMaker<char*, FormatString<>> : StringTraceItemMaker<char, StringItem> {};

template<>
struct TraceItemMaker<char*, FormatString<'s'>> : StringTraceItemMaker<char, StringItem> {};

template<>
struct TraceItemMaker<char*, FormatString<'x'>> : StringTraceItemMaker<char, HexBufferItem, false> {
};

template<>
struct TraceItemMaker<char*, FormatString<'x', 'd'>>
    : StringTraceItemMaker<char, HexDumpItem, false> {};

template<>
struct TraceItemMaker<wchar_t*, FormatString<>> : StringTraceItemMaker<wchar_t, WStringItem> {};

template<>
struct TraceItemMaker<wchar_t*, FormatString<'s'>> : StringTraceItemMaker<wchar_t, WStringItem> {};

template<>
struct TraceItemMaker<wchar_t*, FormatString<'x'>>
    : StringTraceItemMaker<wchar_t, HexBufferItem, false> {};

template<>
struct TraceItemMaker<wchar_t*, FormatString<'x', 'd'>>
    : StringTraceItemMaker<wchar_t, HexDumpItem, false> {};

}  // namespace wpp

namespace wpp::internal {

/**
 * This struct is used to indicate that the given format is not supported.
 */
struct InvalidFormatItem {};

/**
 * Builds the matching trace item for the given type and format.
 */
template<typename Format, typename T>
auto buildTraceItem(T&& t) {
    using MakerType = TraceItemMaker<RecursiveDecay<T>, RecursiveDecay<Format>>;
    if constexpr (!HasMakeFunction<MakerType, T>::value) {
        return InvalidFormatItem{};
    } else {
        return MakerType::make(std::forward<T>(t));
    }
}

template<typename T>
constexpr auto makeTracePairs(const T& t) {
    if constexpr (IsSimpleTraceItem<T>::value) {
        return std::make_tuple(t.getPtr(), t.getSize());
    } else if constexpr (IsComplexTraceItem<T>::value) {
        return std::apply(
            [](auto&&... args) { return std::tuple_cat(std::make_tuple(args.ptr, args.size)...); },
            t.makeTracePairs());
    } else {
        static_assert(false, "Bad trace item type!");
    }
}

}  // namespace wpp::internal