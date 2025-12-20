#pragma once
#include "PermInterceptor.hpp"

#include "fmt/format-inl.h"
#include "fmt/format.h"

#include <iosfwd>
#include <optional>


namespace permc {

#define PERMC_INTERCEPTOR_TRACE

template <std::derived_from<ll::event::Event> T>
struct InterceptorTrace;

template <std::derived_from<ll::event::Event> T>
inline thread_local InterceptorTrace<T>* GlobalInterceptorTraceRef = nullptr;

template <std::derived_from<ll::event::Event> T>
struct InterceptorTrace {
    std::ostringstream oss;

    explicit InterceptorTrace(std::string_view event, std::source_location location = std::source_location::current()) {
        GlobalInterceptorTraceRef<T> = this;
        oss << "[InterceptorTrace] " << event << "\n";
        oss << "  [File] " << location.file_name() << ":" << location.line() << "\n";
    }
    ~InterceptorTrace() {
        GlobalInterceptorTraceRef<T> = nullptr;
        if (oss.tellp() > 0) {
            std::cout << "\n" << oss.str();
        }
    }

    template <typename... Args>
    void append(std::string_view fmt, Args&&... args) {
        oss << "  [Text] " << fmt::vformat(fmt, fmt::make_format_args(args...)) << "\n";
    }

    template <typename E>
        requires std::is_enum_v<E>
    void step(std::string_view name, E val) {
        oss << "  [Step] " << name << ": " << magic_enum::enum_name(val) << "\n";
    }

    template <typename E>
        requires std::is_enum_v<E>
    void step(std::string_view name, E val, std::string_view state) {
        oss << "  [Step] " << name << ": " << magic_enum::enum_name(val) << " => " << state << "\n";
    }
};

#ifdef PERMC_INTERCEPTOR_TRACE

#define TRACE_THIS_EVENT(EVENT)         auto __Stack_InterceptorTrace = InterceptorTrace<EVENT>{#EVENT};
#define TRACE_ADD_MESSAGE(...)          __Stack_InterceptorTrace.append(__VA_ARGS__)
#define TRACE_STEP(STEP_NAME, DECISION) __Stack_InterceptorTrace.step(STEP_NAME, DECISION)
#define TRACE_STEP_PRE_CHECK(DECISION)  TRACE_STEP("preCheck", DECISION)
#define TRACE_STEP_ROLE(ROLE)           TRACE_STEP("role", ROLE)

#define TRACE_STEP_T(T, ...)                                                                                           \
    if (GlobalInterceptorTraceRef<T> != nullptr) GlobalInterceptorTraceRef<T>->step(__VA_ARGS__)

#else

#define TRACE_THIS_EVENT(EVENT)         /* nothing */
#define TRACE_ADD_MESSAGE(...)          /* nothing */
#define TRACE_STEP(STEP_NAME, DECISION) /* nothing */
#define TRACE_STEP_PRE_CHECK(DECISION)  /* nothing */
#define TRACE_STEP_ROLE(ROLE)           /* nothing */
#define TRACE_STEP_T(T, ...)            /* nothing */

#endif


} // namespace permc