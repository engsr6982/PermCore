#pragma once
#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/InterceptorTrace.hpp"
#include "perm_core/interceptor/PermInterceptor.hpp"
#include "perm_core/model/PermMapping.hpp"

#include "ll/api/event/player/PlayerAttackEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerPickUpItemEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"

#include "mc/deps/core/string/HashedString.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/FishingRodItem.h"
#include "mc/world/item/HatchetItem.h"
#include "mc/world/item/HoeItem.h"
#include "mc/world/item/HorseArmorItem.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemTag.h"
#include "mc/world/item/ShovelItem.h"
#include "mc/world/item/VanillaItemTags.h"
#include "mc/world/level/block/BlastFurnaceBlock.h"
#include "mc/world/level/block/FurnaceBlock.h"
#include "mc/world/level/block/HangingSignBlock.h"
#include "mc/world/level/block/ShulkerBoxBlock.h"
#include "mc/world/level/block/SignBlock.h"
#include "mc/world/level/block/SmokerBlock.h"

namespace permc {

void PermInterceptor::registerLLPlayerInterceptor(ListenerConfig const& config) {
    auto& bus = ll::event::EventBus::getInstance();
    registerListenerIf(config.PlayerDestroyBlockEvent, [&]() {
        return bus.emplaceListener<ll::event::PlayerDestroyBlockEvent>([&](ll::event::PlayerDestroyBlockEvent& ev) {
            TRACE_THIS_EVENT(ll::event::PlayerDestroyBlockEvent);

            auto& player      = ev.self();
            auto& blockSource = player.getDimensionBlockSource();
            auto& pos         = ev.pos();
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
    });

    registerListenerIf(config.PlayerPlacingBlockEvent, [&]() {
        return bus.emplaceListener<ll::event::PlayerPlacingBlockEvent>([&](ll::event::PlayerPlacingBlockEvent& ev) {
            TRACE_THIS_EVENT(ll::event::PlayerPlacingBlockEvent);

            auto& player      = ev.self();
            auto& blockSource = player.getDimensionBlockSource();
            auto  pos         = ev.pos().relative(ev.face(), 1);
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
                if (applyRoleInterceptor(role, table->role.allowPlace, ev)) {
                    return;
                }
            }
            applyDecision(delegate.postPolicy(blockSource, pos), ev);
        });
    });

    registerListenerIf(config.PlayerInteractBlockEvent, [&]() {
        return bus.emplaceListener<ll::event::PlayerInteractBlockEvent>([&](ll::event::PlayerInteractBlockEvent& ev) {
            TRACE_THIS_EVENT(ll::event::PlayerInteractBlockEvent);

            auto& player      = ev.self();
            auto& blockSource = player.getDimensionBlockSource();
            auto& pos         = ev.blockPos();

            TRACE_ADD_MESSAGE(
                "player={}, pos={}, item={}",
                player.getRealName(),
                pos.toString(),
                ev.item().getTypeName()
            );

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
                if (auto item = ev.item().getItem()) {
                    void** vftable = *reinterpret_cast<void** const*>(item);
                    if (vftable == BucketItem::$vftable()) {
                        if (applyRoleInterceptor(role, table->role.useBucket, ev)) return;
                    } else if (item->hasTag(VanillaItemTags::Hatchet())) {
                        if (applyRoleInterceptor(role, table->role.useAxe, ev)) return;
                    } else if (item->hasTag(VanillaItemTags::Hoe())) {
                        if (applyRoleInterceptor(role, table->role.useHoe, ev)) return;
                    } else if (item->hasTag(VanillaItemTags::Shovel())) {
                        if (applyRoleInterceptor(role, table->role.useShovel, ev)) return;
                    } else if (item->hasTag(VanillaItemTags::Boats()) || item->hasTag(VanillaItemTags::Boat())) {
                        if (applyRoleInterceptor(role, table->role.placeBoat, ev)) return;
                    } else if (item->hasTag(VanillaItemTags::Minecart())) {
                        if (applyRoleInterceptor(role, table->role.placeMinecart, ev)) return;
                    }
                }
                // fallback
                if (auto entry = PermMapping::get().lookup<RolePerms::Entry>(ev.item().getTypeName(), table)) {
                    if (applyRoleInterceptor(role, *entry, ev)) {
                        return;
                    }
                }

                if (auto block = ev.block()) {
                    auto&  legacyBlock = block->getBlockType();
                    void** vftable     = *reinterpret_cast<void** const*>(&legacyBlock);
                    if (legacyBlock.isButtonBlock()) {
                        if (applyRoleInterceptor(role, table->role.useButton, ev)) return;
                    } else if (legacyBlock.isDoorBlock()) {
                        if (applyRoleInterceptor(role, table->role.useDoor, ev)) return;
                    } else if (legacyBlock.isFenceGateBlock()) {
                        if (applyRoleInterceptor(role, table->role.useFenceGate, ev)) return;
                    } else if (legacyBlock.isFenceBlock()) {
                        if (applyRoleInterceptor(role, table->role.allowInteractEntity, ev)) return;
                    } else if (legacyBlock.mIsTrapdoor) {
                        if (applyRoleInterceptor(role, table->role.useTrapdoor, ev)) return;
                    } else if (vftable == ShulkerBoxBlock::$vftable()) {
                        if (applyRoleInterceptor(role, table->role.useShulkerBox, ev)) return;
                    } else if (legacyBlock.isCraftingBlock()) {
                        if (applyRoleInterceptor(role, table->role.useCraftingTable, ev)) return;
                    } else if (legacyBlock.isLeverBlock()) {
                        if (applyRoleInterceptor(role, table->role.useLever, ev)) return;
                    } else if (vftable == BlastFurnaceBlock::$vftable() || vftable == FurnaceBlock::$vftable()
                               || vftable == SmokerBlock::$vftable()) {
                        if (applyRoleInterceptor(role, table->role.useFurnaces, ev)) return;
                    }
                    // fallback
                    if (auto entry = PermMapping::get().lookup<RolePerms::Entry>(block->getTypeName().data(), table)) {
                        if (applyRoleInterceptor(role, *entry, ev)) {
                            return;
                        }
                    }
                }
            }
            applyDecision(delegate.postPolicy(blockSource, pos), ev);
        });
    });

    registerListenerIf(config.PlayerAttackEvent, [&]() {
        return bus.emplaceListener<ll::event::PlayerAttackEvent>([&](ll::event::PlayerAttackEvent& ev) {
            TRACE_THIS_EVENT(ll::event::PlayerAttackEvent);

            auto&    player      = ev.self();
            auto&    actor       = ev.target();
            BlockPos pos         = actor.getPosition();
            auto&    blockSource = actor.getDimensionBlockSource();

            TRACE_ADD_MESSAGE(
                "player={}, target={}, pos={}",
                player.getRealName(),
                actor.getTypeName(),
                pos.toString()
            );

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
                if (auto entry = PermMapping::get().lookup<RolePerms::Entry>(actor.getTypeName().data(), table)) {
                    if (applyRoleInterceptor(role, *entry, ev)) return;
                }
            }

            applyDecision(delegate.postPolicy(blockSource, pos), ev);
        });
    });

    registerListenerIf(config.PlayerPickUpItemEvent, [&]() {
        return bus.emplaceListener<ll::event::PlayerPickUpItemEvent>([&](ll::event::PlayerPickUpItemEvent& ev) {
            TRACE_THIS_EVENT(ll::event::PlayerPickUpItemEvent);

            auto&    player      = ev.self();
            auto&    item        = ev.itemActor();
            auto&    blockSource = player.getDimensionBlockSource();
            BlockPos pos         = item.getPosition();

            TRACE_ADD_MESSAGE("player={}, item={}, pos={}", player.getRealName(), item.getTypeName(), pos.toString());

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
                if (applyRoleInterceptor(role, table->role.allowPlayerPickupItem, ev)) return;
            }

            applyDecision(delegate.postPolicy(blockSource, pos), ev);
        });
    });
}

} // namespace permc