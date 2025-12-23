#pragma once
#include "perm_core/interceptor/PermInterceptor.hpp"

#include "ila/event/minecraft/world/actor/ActorDestroyBlockEvent.h"
#include "ila/event/minecraft/world/actor/ActorRideEvent.h"
#include "ila/event/minecraft/world/actor/ActorTriggerPressurePlateEvent.h"
#include "ila/event/minecraft/world/actor/MobHurtEffectEvent.h"
#include "ila/event/minecraft/world/actor/MobPlaceBlockEvent.h"
#include "ila/event/minecraft/world/actor/MobTakeBlockEvent.h"
#include "ila/event/minecraft/world/actor/ProjectileCreateEvent.h"


#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/platform/UUID.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"
#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/InterceptorTrace.hpp"
#include "perm_core/model/PermMapping.hpp"

#include <ll/api/event/EventBus.h>


namespace permc {

void PermInterceptor::registerIlaEntityInterceptor(ListenerConfig const& config) {
    auto& bus = ll::event::EventBus::getInstance();
    registerListenerIf(config.ActorDestroyBlockEvent, [&]() {
        return bus.emplaceListener<ila::mc::ActorDestroyBlockEvent>([&](ila::mc::ActorDestroyBlockEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::ActorDestroyBlockEvent);

            auto& actor       = ev.self();
            auto& blockPos    = ev.pos();
            auto& blockSource = actor.getDimensionBlockSource();

            TRACE_ADD_MESSAGE("actor={}, pos={}", actor.getTypeName(), blockPos.toString());

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, blockPos);
            TRACE_STEP_PRE_CHECK(decision);
            if (applyDecision(decision, ev)) return;

            if (auto table = delegate.getPermTable(blockSource, blockPos)) {
                if (applyDecision(table->environment.allowActorDestroy, ev)) return;
            }

            applyDecision(delegate.postPolicy(blockSource, blockPos), ev);
        });
    });

    registerListenerIf(config.MobTakeBlockBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::MobTakeBlockBeforeEvent>([&](ila::mc::MobTakeBlockBeforeEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::MobTakeBlockBeforeEvent);

            auto& actor       = ev.self();
            auto& blockPos    = ev.pos();
            auto& blockSource = actor.getDimensionBlockSource();

            TRACE_ADD_MESSAGE("actor={}, pos={}", actor.getTypeName(), blockPos.toString());

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, blockPos);
            TRACE_STEP_PRE_CHECK(decision);
            if (applyDecision(decision, ev)) return;

            if (auto table = delegate.getPermTable(blockSource, blockPos)) {
                if (applyDecision(table->environment.allowActorDestroy, ev)) return;
            }

            applyDecision(delegate.postPolicy(blockSource, blockPos), ev);
        });
    });

    registerListenerIf(config.MobPlaceBlockBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::MobPlaceBlockBeforeEvent>([&](ila::mc::MobPlaceBlockBeforeEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::MobPlaceBlockBeforeEvent);

            auto& actor       = ev.self();
            auto& blockPos    = ev.pos();
            auto& blockSource = actor.getDimensionBlockSource();

            TRACE_ADD_MESSAGE("actor={}, pos={}", actor.getTypeName(), blockPos.toString());

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, blockPos);
            TRACE_STEP_PRE_CHECK(decision);
            if (applyDecision(decision, ev)) return;

            if (auto table = delegate.getPermTable(blockSource, blockPos)) {
                if (applyDecision(table->environment.allowActorDestroy, ev)) return;
            }

            applyDecision(delegate.postPolicy(blockSource, blockPos), ev);
        });
    });

    registerListenerIf(config.ActorRideBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::ActorRideBeforeEvent>([&](ila::mc::ActorRideBeforeEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::ActorRideBeforeEvent);

            Actor& passenger   = ev.self();
            Actor& target      = ev.target();
            auto&  blockSource = target.getDimensionBlockSource();

            if (!passenger.isPlayer()) {
                TRACE_ADD_MESSAGE("passenger is not player");
                return;
            }
            if (target.hasCategory(ActorCategory::Ridable)) {
                TRACE_ADD_MESSAGE("target is rideable");
                return;
            }

            TRACE_ADD_MESSAGE(
                "passenger: {}, target: {}",
                passenger.getActorIdentifier().mIdentifier.get(),
                target.getTypeName()
            );
            auto&    player   = static_cast<Player&>(passenger);
            BlockPos blockPos = target.getPosition();

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, blockPos);
            TRACE_STEP_PRE_CHECK(decision);
            if (applyDecision(decision, ev)) return;

            auto role = delegate.getRole(player, blockSource, blockPos);
            TRACE_STEP_ROLE(role);
            if (applyPrivilege(role, ev)) return;

            if (auto table = delegate.getPermTable(blockSource, blockPos)) {
                if (target.hasCategory(ActorCategory::BoatRideable)
                    || target.hasCategory(ActorCategory::MinecartRidable)) {
                    if (applyRoleInterceptor(role, table->role.allowRideTrans, ev)) return;
                } else {
                    if (applyRoleInterceptor(role, table->role.allowRideEntity, ev)) return;
                }
            }

            applyDecision(delegate.postPolicy(blockSource, blockPos), ev);
        });
    });

    registerListenerIf(config.MobHurtEffectBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::MobHurtEffectBeforeEvent>([&](ila::mc::MobHurtEffectBeforeEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::MobHurtEffectBeforeEvent);

            auto& actor       = ev.self();
            auto  sourceActor = ev.source();

            if (!sourceActor || !sourceActor->isPlayer()) {
                TRACE_ADD_MESSAGE("source is not player");
                return;
            }
            auto&    blockSource = actor.getDimensionBlockSource();
            auto&    player      = static_cast<Player&>(sourceActor.value());
            BlockPos blockPos    = actor.getPosition();

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, blockPos);
            TRACE_STEP_PRE_CHECK(decision);
            if (applyDecision(decision, ev)) return;

            auto role = delegate.getRole(player, blockSource, blockPos);
            TRACE_STEP_ROLE(role);
            if (applyPrivilege(role, ev)) return;

            if (auto table = delegate.getPermTable(blockSource, blockPos)) {
                if (auto entry = PermMapping::get().lookup<RolePerms::Entry>(actor.getTypeName().data(), table)) {
                    if (applyRoleInterceptor(role, *entry, ev)) return;
                }
            }

            applyDecision(delegate.postPolicy(blockSource, blockPos), ev);
        });
    });

    registerListenerIf(config.ActorTriggerPressurePlateBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::ActorTriggerPressurePlateBeforeEvent>(
            [&](ila::mc::ActorTriggerPressurePlateBeforeEvent& ev) {
                TRACE_THIS_EVENT(ila::mc::ActorTriggerPressurePlateBeforeEvent);

                auto& actor       = ev.self();
                auto& blockPos    = ev.pos();
                auto& blockSource = actor.getDimensionBlockSource();

                auto isPlayer = actor.isPlayer();

                TRACE_ADD_MESSAGE("pos={}, isPlayer={}", blockPos.toString(), isPlayer);

                auto& delegate = getDelegate();
                auto  decision = delegate.preCheck(blockSource, blockPos);
                TRACE_STEP_PRE_CHECK(decision);
                if (applyDecision(decision, ev)) return;

                auto role = PermRole::Gust;
                if (isPlayer) {
                    auto& player = static_cast<Player&>(actor);
                    role         = delegate.getRole(player, blockSource, blockPos);
                }
                TRACE_STEP_ROLE(role);
                if (applyPrivilege(role, ev)) return;

                if (auto table = delegate.getPermTable(blockSource, blockPos)) {
                    if (applyRoleInterceptor(role, table->role.usePressurePlate, ev)) return;
                }

                applyDecision(delegate.postPolicy(blockSource, blockPos), ev);
            }
        );
    });
}

} // namespace permc