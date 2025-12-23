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
}

} // namespace permc