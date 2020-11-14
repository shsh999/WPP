# WPP++
This is a modern c++17 implementation of [WPP Software Tracing](https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/wpp-software-tracing), that requires no additional preprocessor or generated source/header files.

WPP++ allows an easily extensible way for tracing messages using windows ETW with a python-like string formatting syntax, while the trace information (string/argument types) is not part of the generated binary.

## Requirements
* Visual Studio 2019 with c++17 support (tested with visual studio 16.7.7)
* Required flags:
  * `/std:c++17`
  * `/Zi` (**not `/ZI`**) (`C/C++->Debug Information Format->Program Database (/Zi)`)


## Example
```c++
#include "wpp/DefaultTracing.h"

constexpr const GUID CONTROL_GUID{
    0x11223344, 0xaaaa, 0xbbbb, {0xcc, 0xcc, 0xdd, 0xff, 0xff, 0x11, 0x22, 0x33}};

int main() {
    wpp::WppTraceGuard guard{CONTROL_GUID};

    WPP_TRACE_INFO("Hello {:s}!", "world");

    return 0;
}
```

Compile the program, and use [scripts\tracepdb.py](scripts\tracepdb.py) to generate a `tmf` (trace message format) file:
```
py -3 scripts\tracepdb.py -of format_info.tmf path\to\Example.pdb
```

Use logman to capture the traces from the file:
```
logman create trace MyTraceSession -p {11223344-AAAA-BBBB-CCCC-DDFFFF112233} 255 255 -o log_file.etl

logman start MyTraceSession

<run a program using wpp++>

logman stop MyTraceSession
```

Now, you can use a standard tool such as `tracefmt` or `traceview` to parse the generated `log_file_000001.etl`.

## The trace syntax
The trace syntax is similar to python's string formatting syntax, and can be used to modify the way an argument is printed.

For example:
```cpp
WPP_TRACE_INFO("Decimal integer: {:d}", 10); // Traces "Decimal integer: 10"
WPP_TRACE_INFO("Hex integer: {:x}", 10); // Traces "Hex integer: a"
```

### Supported formats

#### Integers
The supported integer types are `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`, `int8_t`, `int16_t`, `int32_t`, `int64_t` and `byte`.

`int`, `long` etc. are aliases for these types, and they are supported as well.

* `d` - decimal (default if no format is specified)
* `x`, `X` - hexadecimal
* `b`, `B` - binary (currently printed as hex)
* `o` - base 8 integers
* `z` - Used to specify a size_t / ptrdiff_t type, and can be followed by any other valid integer format (`zd`, `zx`, etc.)

#### Characters
The supported character types are `char` and `wchar_t`.
They support all the integer format specifications and the special `c` specification, which is the default.

#### Floating point numbers
Supported types: `float`, `double`, `long double`.

* `a`, `A`
* `e`, `E`
* `f` (default), `F` 
* `g`, `G`

For the exact meaning of each specification, see [MSDN documentation](https://docs.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions?view=msvc-160&viewFallbackFrom=vs-2019).

#### Pointers
Any pointer can be printed with the `p` (default) specification.

#### Null-terminated Strings
The string types are `char*` and `wchar_t*`, and they support the following formats:
* `s` (default) - the string value.
* `x` - hexadecimal representation of the string
* `xd` - a hexdump of the string.
* `p` - print the address of the string (like other pointers)

#### GUIDs
GUIDs can be traced with an empty specifier, yielding the expected output.

## How does it work?
### Unique trace identifiers
WPP++ uses a constexpr implementation of MD5 in order to generate a unique trace GUID for each trace. This GUID is later used to 

### "Leaking" data from the compilation process
Similarily to WPP, WPP++ uses the MSVC `__annotation(...)` intrinsic in order to write the trace information into the generated PDB file.

The trace information required:
* file
* line
* function
* trace flag
* trace level
* format string
* argument types

Everything except for the argument types can be traced directly to the PDB file, as it is available directly on the call site:
```c++
#define __WPP_ANNOTATE_TRACE_INFO(flag, level, fmt, ...) __annotation(L"TMF_NG:", __WPP_MAKE_WIDE(__FILE__), __WPP_MAKE_WSTRING(__LINE__), L"FUNC=" __WPP_MAKE_WIDE(__FUNCSIG__), L"FLAG=" __WPP_MAKE_WSTRING(flag), L"LEVEL=" __WPP_MAKE_WSTRING(level), __WPP_MAKE_WIDE(fmt), __WPP_MAKE_WSTRING(__VA_ARGS__));
```

However, generating the type information is more problematic, as there is no trivial C++ way to stringify the types using macros.

This problem is solved by calling a function templated on the traced items types, and using MSVC's `__FUNCSIG__` macro to trace the types as part of the function signature. In order to match the type annotation with the rest of the data, the calculated trace hash is also passed as template arguments to the annotation function, and is also traced as part of the function signature.

### Parsing log files
[tracepdb.py](scripts\tracepdb.py) can be used to generate regular WPP compatible `.tmf` files containing trace information. The script extracts the annotations from the PDB files, matches the basic information and the type information, and then generates a matching "legacy" WPP trace format.

The generated `tmf` file can be used by "regular" tools such as `tracefmt` or `traceview` in order to parse `etl` log files.
