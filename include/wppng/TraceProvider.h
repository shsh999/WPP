#pragma once
#include <Windows.h>
#include <evntrace.h>

#include "TraceItems.h"

namespace wpp {

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
 * TODO: documentation
 *
 * Note: The struct is neither movable nor copyable, as the trace handle api requires a pointer to a
 * context - stored in this object.
 */
class TraceProvider {
public:
    TraceProvider(const GUID& controlGUID) {
        TRACE_GUID_REGISTRATION guids[] = {{&controlGUID, nullptr}};
        RegisterTraceGuids(controlCallback, &m_context, &controlGUID, 1, guids, nullptr, nullptr,
                           &m_controlHandle);
        // TODO?: Error handling if status != ERROR_SUCCESS
    }

    ~TraceProvider() noexcept {
        if (m_controlHandle) {
            // Since this is a destructor, nothing could be done in the case of a failure
            UnregisterTraceGuids(m_controlHandle);
        }
    }

    TraceProvider(const TraceProvider&) = delete;
    TraceProvider& operator=(const TraceProvider&) = delete;
    TraceProvider(TraceProvider&&) = delete;
    TraceProvider& operator=(TraceProvider&&) = delete;

    /**
     * Trace the given trace items to ETW.
     */
    /*template<typename... Args>
    constexpr void traceMessage(UCHAR flag, TraceLevel level, const GUID& messageGuid,
                                Args&&... args) noexcept {
        if (areTracesEnabled(flag, level)) {
            doTrace(messageGuid,
                    TraceItemMaker<internal::remove_cvref_t<Args>>::make(std::forward<Args>(args))...);
        }
    }*/

    constexpr bool areTracesEnabled(UCHAR flag, TraceLevel level) const noexcept {
        return (flag & m_context.enabledFlags) &&
               (m_context.enabledLevels >= static_cast<uint32_t>(level));
    }

    /*template<typename... Args>
    constexpr void traceMessageFromTraceItems(UCHAR flag, TraceLevel level, const GUID& messageGuid,
                            Args&&... args) const noexcept {
        if (areTracesEnabled(flag, level)) {
            std::apply(
                [this, &messageGuid](auto&&... args2) {
                    constexpr const uint16_t MESSAGE_ID = 10;
                    constexpr const uint32_t TRACE_FLAGS =
                        TRACE_MESSAGE_GUID | TRACE_MESSAGE_SEQUENCE | TRACE_MESSAGE_SYSTEMINFO |
                        TRACE_MESSAGE_TIMESTAMP;

                    TraceMessage(m_context.sessionHandle, TRACE_FLAGS, &messageGuid, MESSAGE_ID,
                                 std::forward<decltype(args2)>(args2)..., static_cast<size_t>(0));
                },
                std::tuple_cat(makeTracePairs(std::forward<Args>(args))...));
        }
    }*/

    template<typename... Args>
    void traceMessageFromTraceItems(const GUID& messageGuid, Args&&... args) const noexcept {
        std::apply(
            [sessionHandle = m_context.sessionHandle, &messageGuid](auto&&... args2) {
                constexpr const uint16_t MESSAGE_ID = 10;
                constexpr const uint32_t TRACE_FLAGS = TRACE_MESSAGE_GUID | TRACE_MESSAGE_SEQUENCE |
                                                       TRACE_MESSAGE_SYSTEMINFO |
                                                       TRACE_MESSAGE_TIMESTAMP;

                TraceMessage(sessionHandle, TRACE_FLAGS, &messageGuid, MESSAGE_ID,
                             std::forward<decltype(args2)>(args2)..., static_cast<size_t>(0));
            },
            std::tuple_cat(makeTracePairs(std::forward<Args>(args))...));
    }

    /*template<typename... Args>
    constexpr void doTrace(const GUID& messageGuid, Args&&... args) const noexcept {
        std::apply(
            [this, &messageGuid](auto&&... args2) {
                constexpr const uint16_t MESSAGE_ID = 10;
                constexpr const uint32_t TRACE_FLAGS = TRACE_MESSAGE_GUID | TRACE_MESSAGE_SEQUENCE |
                                                       TRACE_MESSAGE_SYSTEMINFO |
                                                       TRACE_MESSAGE_TIMESTAMP;

                TraceMessage(m_context.sessionHandle, TRACE_FLAGS, &messageGuid, MESSAGE_ID,
                             std::forward<decltype(args2)>(args2)..., static_cast<size_t>(0));
            },
            std::tuple_cat(makeTracePairs(std::forward<Args>(args))...));
    }*/

private:
    struct TraceContext {
        TRACEHANDLE sessionHandle{};
        ULONG enabledLevels{};
        ULONG enabledFlags{};
    };

    TraceContext m_context{};
    TRACEHANDLE m_controlHandle{};

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
