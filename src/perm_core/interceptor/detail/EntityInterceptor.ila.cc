#pragma once
#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/InterceptorTrace.hpp"
#include "perm_core/interceptor/PermInterceptor.hpp"
#include "perm_core/model/PermMapping.hpp"

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
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/PlayerInventory.h"
#include "mc/world/actor/projectile/AbstractArrow.h"
#include "mc/world/actor/projectile/ThrownTrident.h"

#include <ll/api/event/EventBus.h>
#include <mc/nbt/CompoundTag.h>


namespace permc {

void PermInterceptor::registerIlaEntityInterceptor(InterceptorConfig::ListenerConfig const& config) {
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
                if (applyDecision(table->environment.allowMobGrief, ev)) return;
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
                if (applyDecision(table->environment.allowMobGrief, ev)) return;
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
                if (applyDecision(table->environment.allowMobGrief, ev)) return;
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

                auto role = PermRole::Guest;
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

    registerListenerIf(config.ProjectileCreateBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::ProjectileCreateBeforeEvent>([&](ila::mc::ProjectileCreateBeforeEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::ProjectileCreateBeforeEvent);

            // ! 由于此事件需要进行额外的操作，暂不使用通用拦截模型
            static auto tryReturnItem = [](Actor& projectile, Actor& owner) {
                // https://github.com/engsr6982/PLand/issues/139
                // 三叉戟、箭、喷溅药水、滞留药水、附魔之瓶、末影珍珠、雪球、鸡蛋
                // TODO: 三叉戟丢失附魔数据
                // TODO: 箭、喷溅药水、滞留药水、鸡蛋丢失特殊值
                // TODO: 附魔之瓶处理失败
                ActorType type = projectile.getEntityTypeId();
                switch (type) {
                case ActorType::Trident: // 三叉戟
                case ActorType::Arrow:   // 箭
                case ActorType::AbstractArrow: {
                    auto& abstractArrow = static_cast<AbstractArrow&>(projectile);
                    auto  pickupItem    = abstractArrow._getPickupItem();
                    if (!owner.add(pickupItem)) {
                        owner.spawnAtLocation(pickupItem, 1);
                    }
                    return;
                }
                case ActorType::ThrownPotion:     // 喷溅药水
                case ActorType::LingeringPotion:  // 滞留药水
                case ActorType::ExperiencePotion: // 附魔之瓶
                case ActorType::Enderpearl:       // 末影珍珠
                case ActorType::Snowball:         // 雪球
                case ActorType::IceBomb:          // 冰弹(Minecraft Edu)
                case ActorType::ThrownEgg: {      // 鸡蛋
                    auto itemStack = ItemStack{projectile.getTypeName()};
                    if (!owner.add(itemStack)) {
                        owner.spawnAtLocation(itemStack, 1);
                    }
                    return;
                }
                default: // ignore
                    break;
                }
            };

            auto& projectile = ev.self(); // 这里是弹射物，而非创建者
            auto  owner      = projectile.getOwner();
            if (!owner) {
                TRACE_ADD_MESSAGE("projectile has no owner");
                return;
            }

            auto&      blockSource   = projectile.getDimensionBlockSource();
            BlockPos   blockPos      = projectile.getPosition();
            auto const ownerIsPlayer = owner->isPlayer();

            TRACE_ADD_MESSAGE(
                "type={}, pos={}, ownerIsPlayer={}",
                projectile.getTypeName(),
                blockPos.toString(),
                ownerIsPlayer
            );

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, blockPos);
            TRACE_STEP_PRE_CHECK(decision);
            switch (decision) {
            case PermDecision::Deny:
                ev.cancel();
                tryReturnItem(projectile, *owner);
                [[fallthrough]];
            case PermDecision::Allow:
                return;
            case PermDecision::Abstain:
                break;
            }

            auto role = PermRole::Guest;
            if (ownerIsPlayer) {
                auto& player = static_cast<Player&>(*owner);
                role         = delegate.getRole(player, blockSource, blockPos);
            }
            TRACE_STEP_ROLE(role);
            if (applyPrivilege(role, ev)) return;

            if (auto table = delegate.getPermTable(blockSource, blockPos)) {
                if (projectile.isType(ActorType::FishingHook)) {
                    if (applyRoleInterceptor(role, table->role.allowFishingRodAndHook, ev)) return;
                } else {
                    if ((role == PermRole::Guest && table->role.allowProjectileCreate.guest)
                        || (role == PermRole::Member && table->role.allowProjectileCreate.member)) {
                        return;
                    }
                    ev.cancel();
                    tryReturnItem(projectile, *owner);
                    return;
                }
            }

            if (delegate.postPolicy(blockSource, blockPos) == PermDecision::Deny) {
                ev.cancel();
                tryReturnItem(projectile, *owner);
            }
        });
    });
}

} // namespace permc