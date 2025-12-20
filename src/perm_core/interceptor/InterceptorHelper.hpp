#pragma once
#include "InterceptorDelegate.hpp"
#include "InterceptorTrace.hpp"
#include "perm_core/model/PermRole.hpp"
#include "perm_core/model/RolePermMeta.hpp"

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

// nullopt => Abstain; true => Allow; false => Deny
inline static PermDecision asDecision(std::optional<bool> result) {
    if (!result.has_value()) {
        return PermDecision::Abstain;
    }
    return *result ? PermDecision::Allow : PermDecision::Deny;
}

template <std::derived_from<ll::event::Event> T>
inline static bool applyDecision(std::optional<bool> result, T& event) {
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
inline static bool applyRoleInterceptor(PermRole role, std::optional<RolePermMeta::RoleEntry> entry, T& event) {
    TRACE_STEP_T(T, "applyRoleInterceptor", role);
    if (entry && !applyPrivilege(role, event)) {
        return applyDecision(role == PermRole::Member ? entry->member : entry->guest, event);
    }
    return false;
}


} // namespace permc