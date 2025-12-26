#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/InterceptorTrace.hpp"
#include "perm_core/interceptor/PermInterceptor.hpp"

#include <ll/api/event/EventBus.h>

#include "ila/event/minecraft/world/actor/ArmorStandSwapItemEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerAttackBlockEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerDropItemEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerEditSignEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerInteractEntityEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerOperatedItemFrameEvent.h"

#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"

namespace permc {

void PermInterceptor::registerIlaPlayerInterceptor(ListenerConfig const& config) {
    auto& bus = ll::event::EventBus::getInstance();

    registerListenerIf(config.PlayerInteractEntityBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::PlayerInteractEntityBeforeEvent>(
            [&](ila::mc::PlayerInteractEntityBeforeEvent& ev) {
                TRACE_THIS_EVENT(ila::mc::PlayerInteractEntityBeforeEvent);

                auto&    player      = ev.self();
                auto&    target      = ev.target();
                auto&    blockSource = target.getDimensionBlockSource();
                BlockPos pos         = target.getPosition();

                TRACE_ADD_MESSAGE("player={}, target={}", player.getRealName(), target.getTypeName());

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
                    if (applyRoleInterceptor(role, table->role.allowInteractEntity, ev)) return;
                }

                applyDecision(delegate.postPolicy(blockSource, pos), ev);
            }
        );
    });

    // TODO: 验证移除 PlayerAttackBlockBeforeEvent 事件对 allowAttackDragonEgg 权限的影响

    registerListenerIf(config.ArmorStandSwapItemBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::ArmorStandSwapItemBeforeEvent>(
            [&](ila::mc::ArmorStandSwapItemBeforeEvent& ev) {
                TRACE_THIS_EVENT(ila::mc::ArmorStandSwapItemBeforeEvent);

                auto&    player      = ev.player();
                auto&    armorStand  = ev.self();
                auto&    blockSource = armorStand.getDimensionBlockSource();
                BlockPos pos         = armorStand.getPosition();

                TRACE_ADD_MESSAGE("player={}, armorStandPos={}", player.getRealName(), pos.toString());

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
                    if (applyRoleInterceptor(role, table->role.useArmorStand, ev)) return;
                }

                applyDecision(delegate.postPolicy(blockSource, pos), ev);
            }
        );
    });
}

} // namespace permc