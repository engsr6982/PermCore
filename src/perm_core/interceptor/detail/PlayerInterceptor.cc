#pragma once
#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/InterceptorTrace.hpp"
#include "perm_core/interceptor/PermInterceptor.hpp"

#include "ll/api/event/player/PlayerDestroyBlockEvent.h"

#include "mc/world/actor/player/Player.h"

namespace permc {

void PermInterceptor::registerPlayerInterceptor(ListenerConfig const& config) {
    registerListenerIf<ll::event::PlayerDestroyBlockEvent>(config.PlayerDestroyBlockEvent, [&](auto& ev) {
        TRACE_THIS_EVENT(ll::event::PlayerDestroyBlockEvent);

        auto&    player      = ev.self();
        BlockPos pos         = ev.pos();
        auto&    blockSource = player.getDimensionBlockSource();
        TRACE_ADD_MESSAGE("player={}, pos={}", player.getRealName(), pos.toString());

        auto& delegate = getDelegate();
        auto  decision = delegate.preCheck(blockSource, pos);
        TRACE_STEP_PRE_CHECK(decision);
        if (applyDecision(decision, ev)) {
            return;
        }

        auto role = delegate.getRole(player, blockSource, pos);
        TRACE_STEP_ROLE(role);
        if (applyPrivilege(role, ev)) {
            return;
        }

        if (auto table = delegate.getPermTable(blockSource, pos)) {
            if (applyRoleInterceptor(role, table->role.allowDestroy, ev)) {
                return;
            }
        }

        applyDecision(delegate.postPolicy(blockSource, pos), ev);
    });
}

} // namespace permc