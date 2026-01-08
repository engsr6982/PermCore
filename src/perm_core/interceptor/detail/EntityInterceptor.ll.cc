#pragma once
#include "perm_core/interceptor/PermInterceptor.hpp"

#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/level/Level.h"


#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"
#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/InterceptorTrace.hpp"
#include "perm_core/model/PermMapping.hpp"

namespace permc {


void PermInterceptor::registerLLEntityInterceptor(InterceptorConfig::ListenerConfig const& config) {
    auto& bus = ll::event::EventBus::getInstance();
    registerListenerIf(config.SpawnedMobEvent, [&]() {
        return bus.emplaceListener<ll::event::SpawnedMobEvent>([&](ll::event::SpawnedMobEvent& ev) {
            TRACE_THIS_EVENT(ll::event::SpawnedMobEvent);
            // ! 非标准事件拦截流程

            auto mob = ev.mob();
            if (!mob) {
                return; // 因为事件是拦截特定类型实体生成，如果实体不存在这里直接跳过
            }

            auto     spawner     = ev.spawner();
            auto&    blockSource = ev.blockSource();
            BlockPos pos         = ev.pos();

            TRACE_ADD_MESSAGE(
                "spawner={}, mob={}",
                spawner ? spawner->getTypeName() : "null",
                mob ? mob->getTypeName() : "null"
            );

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, pos);
            TRACE_STEP_PRE_CHECK(decision);
            if (decision == PermDecision::Deny) {
                mob->despawn();
                TRACE_STEP("preCheck deny, despawn", decision);
                return;
            }
            if (decision == PermDecision::Allow) {
                return;
            }

            if (auto table = delegate.getPermTable(blockSource, pos)) {
                bool const isMonster = mob->hasCategory(::ActorCategory::Monster) || mob->hasFamily("monster");
                if ((isMonster && !table->environment.allowMonsterSpawn)
                    || (!isMonster && !table->environment.allowAnimalSpawn)) {
                    mob->despawn();
                    TRACE_STEP("perm deny, despawn", decision);
                    return;
                }
            }

            decision = delegate.postPolicy(blockSource, pos);
            TRACE_STEP("postPolicy", decision);
            if (decision == PermDecision::Deny) {
                mob->despawn();
                TRACE_STEP("postPolicy deny, despawn", decision);
            }
        });
    });

    registerListenerIf(config.ActorHurtEvent, [&]() {
        return bus.emplaceListener<ll::event::ActorHurtEvent>([&](ll::event::ActorHurtEvent& ev) {
            TRACE_THIS_EVENT(ll::event::ActorHurtEvent);

            auto&    actor       = ev.self();
            auto&    source      = ev.source();
            auto&    blockSource = actor.getDimensionBlockSource();
            BlockPos pos         = actor.getPosition();

            if (source.getEntityType() != ActorType::Player) {
                TRACE_ADD_MESSAGE("source is not player");
                return;
            }
            auto player = actor.getLevel().getPlayer(source.getEntityUniqueID());
            if (!player) {
                TRACE_ADD_MESSAGE("player not found");
                return;
            }

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, pos);
            TRACE_STEP_PRE_CHECK(decision);
            if (applyDecision(decision, ev)) {
                return;
            }

            auto role = delegate.getRole(*player, blockSource, pos);
            if (applyPrivilege(role, ev)) {
                return;
            }

            if (auto table = delegate.getPermTable(blockSource, pos)) {
                if (auto entry = PermMapping::get().lookup<RolePerms::Entry>(actor.getTypeName().data(), table)) {
                    if (applyRoleInterceptor(role, *entry, ev)) return;
                }
            }

            applyDecision(delegate.postPolicy(blockSource, pos), ev);
        });
    });
}
} // namespace permc