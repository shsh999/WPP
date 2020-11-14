/**
 * This is an example "default" trace definition file for a WPP-like experience - a global trace
 * provider used with trace macros all over the program.
 */
#include <optional>
#include "Trace.h"

namespace wpp {

/**
 * The global trace provider.
 */
inline std::optional<TraceProvider> g_wppDefaultProvider = std::nullopt;

/**
 * Initializes the global trace provider with the given GUID.
 */
void wppInitTraces(const GUID& controlGuid) {
    g_wppDefaultProvider.emplace(controlGuid);
}

/**
 * Destroys the global trace provider.
 */
void wppStopTraces() {
    g_wppDefaultProvider.reset();
}

/**
 * A RAII guard for the global trace provider.
 */
struct WppTraceGuard {
    [[nodiscard]] explicit WppTraceGuard(const GUID& controlGuid) {
        wppInitTraces(controlGuid);
    }

    // Disallow copy operations
    WppTraceGuard(const WppTraceGuard&) = delete;
    WppTraceGuard& operator=(const WppTraceGuard&) = delete;

    // Disallow move operations
    WppTraceGuard(WppTraceGuard&&) = delete;
    WppTraceGuard& operator=(WppTraceGuard&&) = delete;

    ~WppTraceGuard() {
        wppStopTraces();
    }
};

};  // namespace wpp

//////////////////
// Trace Macros //
//////////////////

#define __WPP_TRACE_FLAG_LEVEL(flag, level, fmt, ...)                       \
    do {                                                                  \
        if (::wpp::g_wppDefaultProvider) {                                \
            auto& ___wpp_provider = *::wpp::g_wppDefaultProvider;         \
            WPP_DO_TRACE(___wpp_provider, flag, level, fmt, __VA_ARGS__); \
        }                                                                 \
    } while (0)

#define WPP_TRACE_INFO(fmt, ...) \
    __WPP_TRACE_FLAG_LEVEL(1, ::wpp::TraceLevel::Information, fmt, __VA_ARGS__)

#define WPP_TRACE_ERROR(fmt, ...) \
    __WPP_TRACE_FLAG_LEVEL(1, ::wpp::TraceLevel::Error, fmt, __VA_ARGS__)

#define WPP_TRACE_VERBOSE(fmt, ...) \
    __WPP_TRACE_FLAG_LEVEL(1, ::wpp::TraceLevel::Verbose, fmt, __VA_ARGS__)

#define WPP_TRACE_WARNING(fmt, ...) \
    __WPP_TRACE_FLAG_LEVEL(1, ::wpp::TraceLevel::Warning, fmt, __VA_ARGS__)