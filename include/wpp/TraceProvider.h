#pragma once
#include <Windows.h>
#include <evntrace.h>

#include "TraceItems.h"

namespace wpp {

/**
 * An enumeration defining a trace level.
 * The lower the trace level, the more important it is. For example, tracing a Warning will be
 * consumed by consumers waiting for Information and below, but not by consumers waiting for Errors.
 */
enum class TraceLevel : uint32_t {
    None = 0,         // Tracing is not on
    Critical = 1,     // Abnormal exit or termination
    Fatal = 1,        // Deprecated name for Abnormal exit or termination
    Error = 2,        // Severe errors that need logging
    Warning = 3,      // Warnings such as allocation failure
    Information = 4,  // Includes non-error cases (e.g.,Entry-Exit)
    Verbose = 5,      // Detailed traces from intermediate steps
    Reserved6 = 6,
    Reserved7 = 7,
    Reserved8 = 8,
    Reserved9 = 9,
};

/**
 * This class is an ETW trace provider, registering as a provider on construction and un-registering
 * on destruction.
 *
 * Currently, the class offers no error-handling: it is assumed that logging is not critical enough
 * to halt a program (and that practically such errors will never occur).
 *
 * Note: The class is neither movable nor copyable, as the trace handle api requires a pointer to a
 * context - stored in this object.
 */
class TraceProvider {
public:
    TraceProvider(const GUID& controlGUID) {
        TRACE_GUID_REGISTRATION guids[] = {{&controlGUID, nullptr}};
        // Currently, no error handling if status != ERROR_SUCCESS
        (void)RegisterTraceGuids(controlCallback, &m_context, &controlGUID, 1, guids, nullptr,
                                 nullptr, &m_controlHandle);
    }

    ~TraceProvider() noexcept {
        if (m_controlHandle) {
            // Since this is a destructor, nothing could be done in the case of a failure anyways
            UnregisterTraceGuids(m_controlHandle);
        }
    }

    // Prevent copy operations
    TraceProvider(const TraceProvider&) = delete;
    TraceProvider& operator=(const TraceProvider&) = delete;

    // Prevent move operations
    TraceProvider(TraceProvider&&) = delete;
    TraceProvider& operator=(TraceProvider&&) = delete;

    /**
     * Checks whether the given flag and trace level are currently enabled for tracing.
     */
    constexpr bool areTracesEnabled(UCHAR flag, TraceLevel level) const noexcept {
        return (flag & m_context.enabledFlags) &&
               (m_context.enabledLevels >= static_cast<uint32_t>(level));
    }

    /**
     * Traces the given trace items to ETW.
     */
    template<typename... TraceItems>
    void traceMessageFromTraceItems(const GUID& messageGuid,
                                    TraceItems&&... traceItems) const noexcept {
        std::apply(
            [sessionHandle = m_context.sessionHandle, &messageGuid](auto&&... args) {
                // Always use a constant message id, as there is a unique GUID for each message.
                constexpr const uint16_t MESSAGE_ID = 10;
                constexpr const uint32_t TRACE_FLAGS = TRACE_MESSAGE_GUID | TRACE_MESSAGE_SEQUENCE |
                                                       TRACE_MESSAGE_SYSTEMINFO |
                                                       TRACE_MESSAGE_TIMESTAMP;
                // Trace all the arguments, with a "null-terminator" to indicate the last argument.
                TraceMessage(sessionHandle, TRACE_FLAGS, &messageGuid, MESSAGE_ID,
                             std::forward<decltype(args)>(args)..., static_cast<size_t>(0));
            },
            std::tuple_cat(internal::makeTracePairs(std::forward<TraceItems>(traceItems))...));
    }

private:
    /**
     * This structure holds a trace context for a provider: the current ETW session handle, and
     * which traces should be issued.
     */
    struct TraceContext {
        /// The current session handle
        TRACEHANDLE sessionHandle{};
        /// Which trace levels are currently enabled
        ULONG enabledLevels{};
        /// Which trace flags are currently enabled
        ULONG enabledFlags{};
    };

    TraceContext m_context{};
    TRACEHANDLE m_controlHandle{};

    /**
     * The callback registered in RegisterTraceGuids, responsible for handling changes in the traced
     * events.
     */
    static ULONG WINAPI controlCallback(WMIDPREQUESTCODE RequestCode, PVOID Context, ULONG*,
                                        PVOID Header) {
        if (Context == nullptr) {
            return ERROR_INVALID_PARAMETER;
        }

        auto& traceContext = *reinterpret_cast<TraceContext*>(Context);

        switch (RequestCode) {
            case WMI_ENABLE_EVENTS: {
                auto traceHandle = GetTraceLoggerHandle(Header);
                auto enabledLevels = GetTraceEnableLevel(traceHandle);
                auto enabledFlags = GetTraceEnableFlags(traceHandle);

                traceContext.sessionHandle = traceHandle;
                traceContext.enabledLevels = enabledLevels;
                traceContext.enabledFlags = enabledFlags;

                break;
            }
            case WMI_DISABLE_EVENTS: {
                traceContext.sessionHandle = 0;
                traceContext.enabledLevels = 0;
                traceContext.enabledFlags = 0;

                break;
            }
            default:
                return ERROR_INVALID_PARAMETER;
        }

        return ERROR_SUCCESS;
    }
};

}  // namespace wpp
