#include "perm_core/interceptor/InterceptorDelegate.hpp"
#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/InterceptorTrace.hpp"
#include "perm_core/interceptor/PermInterceptor.hpp"

#include "ll/api/event/EventBus.h"

#include "ila/event/minecraft/world/ExplosionEvent.h"
#include "ila/event/minecraft/world/PistonPushEvent.h"
#include "ila/event/minecraft/world/RedstoneUpdateEvent.h"
#include "ila/event/minecraft/world/SculkBlockGrowthEvent.h"
#include "ila/event/minecraft/world/WitherDestroyEvent.h"
#include "ila/event/minecraft/world/level/block/BlockFallEvent.h"
#include "ila/event/minecraft/world/level/block/DragonEggBlockTeleportEvent.h"
#include "ila/event/minecraft/world/level/block/FarmDecayEvent.h"
#include "ila/event/minecraft/world/level/block/LiquidFlowEvent.h"
#include "ila/event/minecraft/world/level/block/MossGrowthEvent.h"
#include "ila/event/minecraft/world/level/block/SculkCatalystAbsorbExperienceEvent.h"
#include "ila/event/minecraft/world/level/block/SculkSpreadEvent.h"

#include "mc/world/level/Explosion.h"
#include "mc/world/phys/AABB.h"

namespace permc {

void PermInterceptor::registerIlaWorldInterceptor(ListenerConfig const& config) {
    auto& bus = ll::event::EventBus::getInstance();

    registerListenerIf(config.ExplosionBeforeEvent, [&]() {
        return bus.emplaceListener<ila::mc::ExplosionBeforeEvent>([&](ila::mc::ExplosionBeforeEvent& ev) {
            TRACE_THIS_EVENT(ila::mc::ExplosionBeforeEvent);

            auto& explosion   = ev.explosion();
            auto& blockSource = explosion.mRegion;
            auto  centerPos   = BlockPos{explosion.mPos};
            auto  radius      = explosion.mRadius;

            TRACE_ADD_MESSAGE("centerPos={}, radius={}", centerPos.toString(), radius);

            auto& delegate = getDelegate();
            auto  decision = delegate.preCheck(blockSource, centerPos);
            TRACE_STEP_PRE_CHECK(decision);
            if (applyDecision(decision, ev)) {
                return;
            }

            // 爆炸中心点具有最高决策权
            if (auto centerTable = delegate.getPermTable(blockSource, centerPos)) {
                if (applyDecision(centerTable->environment.allowExplode, ev)) return;
            }

            auto aabb = AABB{};
            {
                float expandRadius = radius + 1.0f;

                aabb.min.x = centerPos.x - expandRadius;
                aabb.min.y = centerPos.y - expandRadius;
                aabb.min.z = centerPos.z - expandRadius;
                aabb.max.x = centerPos.x + expandRadius;
                aabb.max.y = centerPos.y + expandRadius;
                aabb.max.z = centerPos.z + expandRadius;
            }
            // 矩阵查询可能受影响的区域
            bool needFineCheck = false;
            auto iter          = delegate.queryMatrix(blockSource, aabb);
            for (auto const& tab : iter) {
                if (!tab.environment.allowExplode) {
                    // 矩阵中存在禁止爆炸的权限源，
                    // 需要进入受影响方块的精细判定
                    // ev.cancel();
                    // return;
                    needFineCheck = true;
                    break;
                }
            }

            // 精细判定受影响的方块集
            // TODO: 进行性能测试验证精细判定的性能影响在可控范围内
            auto& affected = explosion.mAffectedBlocks.get();
            if (needFineCheck) {
                static constexpr size_t MAX_AFFECTED_BLOCKS = 32;
                if (affected.size() <= MAX_AFFECTED_BLOCKS) {
                    auto iter = affected.begin();
                    while (iter != affected.end()) {
                        auto tab = delegate.getPermTable(blockSource, *iter);
                        if (tab && !tab->environment.allowExplode) {
                            iter = affected.erase(iter); // 剔除禁止爆炸的区域
                            continue;
                        }
                        ++iter;
                    }
                } else {
                    ev.cancel(); // 受影响的方块过多，考虑性能不进行精细判定
                    return;
                }
            }

            applyDecision(delegate.postPolicy(blockSource, centerPos), ev);
        });
    });
}

} // namespace permc