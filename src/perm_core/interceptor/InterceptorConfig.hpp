#pragma once


namespace permc {


struct InterceptorConfig {
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
        bool DragonEggBlockTeleportBeforeEvent    = true; // ILA (env)
        bool SculkBlockGrowthBeforeEvent          = true; // ILA (env)
        bool SculkSpreadBeforeEvent               = true; // ILA (env)
    } listeners_;
    struct HookConfig {

    } hooks_;
};


} // namespace permc