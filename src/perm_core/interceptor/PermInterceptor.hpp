#pragma once
#include "InterceptorDelegate.hpp"

#include <ll/api/event/ListenerBase.h>

#include <memory>
#include <utility>


namespace permc {

/**
 * @note 拦截器，用于拦截事件
 * @note 对于环境权限:
 *          1. 收集参数
 *          2. preCheck 预检
 *          3. 检查环境权限 (getStorage -> resolveEnvironment)
 *          4. postPolicy 后处理
 * @note 对于角色权限:
 *          1. 收集参数
 *          2. preCheck 预检
 *          3. 检查角色特权 (getRole)
 *          4. [查询 typename 对应权限 (PermMapping::lookup) (仅部分事件)]
 *          5. 检查角色权限 (getStorage -> resolveRole)
 *          6. postPolicy 后处理
 */
class PermInterceptor final {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    struct ListenerConfig {
        bool PlayerDestroyBlockEvent              = true; // LL
        bool PlayerPlacingBlockEvent              = true; // LL
        bool PlayerInteractBlockEvent             = true; // LL
        bool PlayerAttackEvent                    = true; // LL
        bool PlayerPickUpItemEvent                = true; // LL
        bool SpawnedMobEvent                      = true; // LL (env)
        bool ActorHurtEvent                       = true; // LL
        bool FireSpreadEvent                      = true; // LL (env)
        bool ActorDestroyBlockEvent               = true; // ILA (env)
        bool MobTakeBlockBeforeEvent              = true; // ILA (env)
        bool MobPlaceBlockBeforeEvent             = true; // ILA (env)
        bool ActorRideBeforeEvent                 = true; // ILA
        bool MobHurtEffectBeforeEvent             = true; // ILA
        bool ActorTriggerPressurePlateBeforeEvent = true; // ILA
        bool ProjectileCreateBeforeEvent          = true; // ILA
        bool PlayerInteractEntityBeforeEvent      = true; // ILA
        bool ArmorStandSwapItemBeforeEvent        = true; // ILA
        bool PlayerDropItemBeforeEvent            = true; // ILA
        bool PlayerOperatedItemFrameBeforeEvent   = true; // ILA
        bool PlayerEditSignBeforeEvent            = true; // ILA
        bool ExplosionBeforeEvent                 = true; // ILA (env)
        bool FarmDecayBeforeEvent                 = true; // ILA (env)
        bool PistonPushBeforeEvent                = true; // ILA (env)
        bool RedstoneUpdateBeforeEvent            = true; // ILA (env)
        bool BlockFallBeforeEvent                 = true; // ILA (env)
        bool WitherDestroyBeforeEvent             = true; // ILA (env)
        bool MossGrowthBeforeEvent                = true; // ILA (env)
        bool LiquidFlowBeforeEvent                = true; // ILA (env)
    };

    explicit PermInterceptor(std::unique_ptr<InterceptorDelegate> delegate, ListenerConfig const& config);
    ~PermInterceptor();

    [[nodiscard]] InterceptorDelegate&       getDelegate();
    [[nodiscard]] InterceptorDelegate const& getDelegate() const;

    void registerListenerIf(bool cond, std::function<ll::event::ListenerPtr()> factory);

private:
    void registerListener(ll::event::ListenerPtr listener);

    // detail
    void registerLLPlayerInterceptor(ListenerConfig const& config);
    void registerLLEntityInterceptor(ListenerConfig const& config);
    void registerLLWorldInterceptor(ListenerConfig const& config);

    void registerIlaEntityInterceptor(ListenerConfig const& config);
    void registerIlaPlayerInterceptor(ListenerConfig const& config);
    void registerIlaWorldInterceptor(ListenerConfig const& config);
};

} // namespace permc