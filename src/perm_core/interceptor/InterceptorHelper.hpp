#pragma once
#include "InterceptorDelegate.hpp"
#include "InterceptorTrace.hpp"
#include "perm_core/model/PermRole.hpp"

#include <ll/api/event/Event.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>

namespace permc {

/**
 * @return true  -> final (已决定，停止处理)
 *         false -> not final (继续交给下一层)
 * @example: if (resolveResult(result, T)) return;
 */
template <std::derived_from<ll::event::Event> T>
inline static bool applyDecision(PermDecision decision, T& event) {
    TRACE_STEP_T(T, "applyDecision", decision);
    switch (decision) {
    case PermDecision::Abstain:
        return false;
    case PermDecision::Deny:
        if constexpr (requires { event.cancel(); }) {
            event.cancel();
        }
    case PermDecision::Allow:
        return true;
    }
    [[unlikely]] throw std::runtime_error("PermDecision::applyDecision: invalid decision");
}

// true => Allow; false => Deny
#define asDecision(BOOL_V) (BOOL_V ? PermDecision::Allow : PermDecision::Deny)

template <std::derived_from<ll::event::Event> T>
inline static bool applyDecision(bool result, T& event) {
    return applyDecision(asDecision(result), event);
}

template <std::derived_from<ll::event::Event> T>
inline static bool applyPrivilege(PermRole role, T& event) {
    if (role == PermRole::Admin || role == PermRole::Owner) {
        TRACE_STEP_T(T, "applyPrivilege", role, "pass");
        return true;
    }
    TRACE_STEP_T(T, "applyPrivilege", role, "fail");
    return false;
}

template <std::derived_from<ll::event::Event> T>
inline static bool applyRoleInterceptor(PermRole role, RolePerms::Entry entry, T& event) {
    TRACE_SCOPE_T(T, "applyRoleInterceptor");
    if (!applyPrivilege(role, event)) {
        return applyDecision(role == PermRole::Member ? entry.member : entry.guest, event);
    }
    return false;
}


} // namespace permc