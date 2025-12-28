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

    registerListenerIf(config.PlayerDropItemBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::PlayerDropItemBeforeEvent>([&](ila::mc::PlayerDropItemBeforeEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::PlayerDropItemBeforeEvent);

            auto&    player      = ev.self();
            auto&    blockSource = player.getDimensionBlockSource();
            BlockPos pos         = player.getPosition();

            TRACE_ADD_MESSAGE("player={}", player.getRealName());

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
                if (applyRoleInterceptor(role, table->role.allowDropItem, ev)) return;
            }

            applyDecision(delegate.postPolicy(blockSource, pos), ev);
        });
    });

    registerListenerIf(config.PlayerOperatedItemFrameBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::PlayerOperatedItemFrameBeforeEvent>(
            [&](ila::mc::PlayerOperatedItemFrameBeforeEvent& ev) {
                TRACE_THIS_EVENT(ila::mc::PlayerOperatedItemFrameBeforeEvent);

                auto& player      = ev.self();
                auto& pos         = ev.blockPos();
                auto& blockSource = player.getDimensionBlockSource();

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
                    if (applyRoleInterceptor(role, table->role.useItemFrame, ev)) return;
                }

                applyDecision(delegate.postPolicy(blockSource, pos), ev);
            }
        );
    });

    registerListenerIf(config.PlayerEditSignBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::PlayerEditSignBeforeEvent>([&](ila::mc::PlayerEditSignBeforeEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::PlayerEditSignBeforeEvent);

            auto& player      = ev.self();
            auto& pos         = ev.pos();
            auto& blockSource = player.getDimensionBlockSource();

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
                if (applyRoleInterceptor(role, table->role.editSign, ev)) return;
            }

            applyDecision(delegate.postPolicy(blockSource, pos), ev);
        });
    });
}

} // namespace permc