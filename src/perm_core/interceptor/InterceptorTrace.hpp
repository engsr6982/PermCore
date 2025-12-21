#pragma once
#include "PermInterceptor.hpp"

#include "fmt/format.h"

#include <iosfwd>


namespace permc {

#define PERMC_INTERCEPTOR_TRACE

inline constexpr bool kInterceptorTraceEnabled =
#ifdef PERMC_INTERCEPTOR_TRACE
    true;
#else
    false;
#endif

// fwd
template <std::derived_from<ll::event::Event> T>
struct InterceptorTrace;

template <typename T>
struct TraceContext {
    static thread_local InterceptorTrace<T>* current;
};
template <typename T>
thread_local InterceptorTrace<T>* TraceContext<T>::current = nullptr;


template <std::derived_from<ll::event::Event> T>
struct InterceptorTrace {
    std::ostringstream oss;
    int                depth = 0;

    explicit InterceptorTrace(std::string_view event, std::source_location location = std::source_location::current()) {
        TraceContext<T>::current = this;
        oss << "[InterceptorTrace] " << event << "\n";
        oss << "  [File] " << location.file_name() << ":" << location.line() << "\n";
    }
    ~InterceptorTrace() {
        TraceContext<T>::current = nullptr;
        std::cout << "\n" << oss.str();
    }

    void indent() {
        for (int i = 0; i < depth; ++i) oss << "  ";
    }

    template <typename... Args>
    void append(std::string_view fmt, Args&&... args) {
        indent();
        oss << "  [Data] " << fmt::vformat(fmt, fmt::make_format_args(args...)) << "\n";
    }

    template <typename E>
        requires std::is_enum_v<E>
    void step(std::string_view name, E val) {
        indent();
        oss << "  [Step] " << name << ": " << magic_enum::enum_name(val) << "\n";
    }

    template <typename E>
        requires std::is_enum_v<E>
    void step(std::string_view name, E val, std::string_view state) {
        indent();
        oss << "  [Step] " << name << ": " << magic_enum::enum_name(val) << " => " << state << "\n";
    }
};

template <typename T>
struct TraceScope {
    InterceptorTrace<T>* trace;

    explicit TraceScope(std::string_view name) : trace(TraceContext<T>::current) {
        if (trace) {
            ++trace->depth;
            trace->indent();
            trace->oss << "[Scope] " << name << " {\n";
        }
    }
    ~TraceScope() {
        if (trace) {
            trace->indent();
            --trace->depth;
            trace->oss << "}\n";
        }
    }
};


#ifdef PERMC_INTERCEPTOR_TRACE

#define TRACE_THIS_EVENT(EVENT)         auto __Stack_InterceptorTrace = InterceptorTrace<EVENT>{#EVENT};
#define TRACE_ADD_MESSAGE(...)          __Stack_InterceptorTrace.append(__VA_ARGS__)
#define TRACE_STEP(STEP_NAME, DECISION) __Stack_InterceptorTrace.step(STEP_NAME, DECISION)
#define TRACE_STEP_PRE_CHECK(DECISION)  TRACE_STEP("preCheck result", DECISION)
#define TRACE_STEP_ROLE(ROLE)           TRACE_STEP("player role", ROLE)

#define TRACE_STEP_T(T, ...)                                                                                           \
    if constexpr (permc::kInterceptorTraceEnabled) {                                                                   \
        if (auto* p = TraceContext<T>::current) p->step(__VA_ARGS__);                                                  \
    }

#define TRACE_SCOPE_T(T, NAME)                                                                                         \
    TraceScope<T> __trace_scope_##__LINE__ { NAME }


#else

#define TRACE_THIS_EVENT(EVENT)         /* nothing */
#define TRACE_ADD_MESSAGE(...)          /* nothing */
#define TRACE_STEP(STEP_NAME, DECISION) /* nothing */
#define TRACE_STEP_PRE_CHECK(DECISION)  /* nothing */
#define TRACE_STEP_ROLE(ROLE)           /* nothing */
#define TRACE_STEP_T(T, ...)            /* nothing */
#define TRACE_SCOPE_T(T, NAME)          /* nothing */

#endif


} // namespace permc