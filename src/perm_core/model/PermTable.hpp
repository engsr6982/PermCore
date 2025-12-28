#pragma once
#include <ll/api/reflection/Reflection.h>
#include <type_traits>

namespace permc {


struct EnvironmentPerms final {
    bool allowFireSpread;           // 火焰蔓延
    bool allowMonsterSpawn;         // 怪物生成
    bool allowAnimalSpawn;          // 动物生成
    bool allowMobGrief;             // 实体破坏(破坏方块/拾取方块/放置方块)
    bool allowExplode;              // 爆炸
    bool allowFarmDecay;            // 耕地退化
    bool allowPistonPushOnBoundary; // 活塞推动边界方块
    bool allowRedstoneUpdate;       // 红石更新
    bool allowBlockFall;            // 方块掉落
    bool allowWitherDestroy;        // 凋零破坏
    bool allowMossGrowth;           // 苔藓生长(蔓延)
    bool allowLiquidFlow;           // 流动液体
    bool allowDragonEggTeleport;    // 龙蛋传送
    bool allowSculkBlockGrowth;     // 幽匿尖啸体生长
    bool allowSculkSpread;          // 幽匿蔓延
};

struct RolePerms final {
    struct Entry final {
        bool member;
        bool guest;
    };
    Entry allowDestroy{};           // 允许破坏方块
    Entry allowPlace{};             // 允许放置方块
    Entry useBucket{};              // 允许使用桶(水/岩浆/...)
    Entry useAxe{};                 // 允许使用斧头
    Entry useHoe{};                 // 允许使用锄头
    Entry useShovel{};              // 允许使用铲子
    Entry placeBoat{};              // 允许放置船
    Entry placeMinecart{};          // 允许放置矿车
    Entry useButton{};              // 允许使用按钮
    Entry useDoor{};                // 允许使用门
    Entry useFenceGate{};           // 允许使用栅栏门
    Entry allowInteractEntity{};    // 允许与实体交互 // TODO: 解决歧义：玩家交互实体 & 玩家取走栅栏上的拴绳实体
    Entry useTrapdoor{};            // 允许使用活板门
    Entry editSign{};               // 允许编辑告示牌
    Entry useLever{};               // 允许使用拉杆
    Entry useFurnaces{};            // 允许使用所有熔炉类方块（熔炉/高炉/烟熏炉）
    Entry allowPlayerPickupItem{};  // 允许玩家拾取物品
    Entry allowRideTrans{};         // 允许骑乘运输工具（矿车/船）
    Entry allowRideEntity{};        // 允许骑乘实体
    Entry usePressurePlate{};       // 触发压力板
    Entry allowFishingRodAndHook{}; // 允许使用钓鱼竿和鱼钩
    Entry allowProjectileCreate{};  // 允许弹射物创建
    Entry useArmorStand{};          // 允许使用盔甲架
    Entry allowDropItem{};          // 允许丢弃物品
    Entry useItemFrame{};           // 允许操作物品展示框
    Entry useFlintAndSteel{};       // 使用打火石
    Entry useBeacon{};              // 使用信标
    Entry useBed{};                 // 使用床

    // 以下权限均通过 PermMapping 动态映射
    Entry allowPvP{};                 // 允许PvP
    Entry allowHostileDamage{};       // 敌对生物受到伤害
    Entry allowFriendlyDamage{};      // 友好生物受到伤害
    Entry allowSpecialEntityDamage{}; // 特殊生物受到伤害
    Entry useContainer{};             // 允许使用容器(箱子/木桶/潜影盒/发射器/投掷器/漏斗/雕纹书架/试炼宝库/...)
    Entry useWorkstation{};           // 工作站类(工作台/铁砧/附魔台/酿造台/锻造台/砂轮/织布机/切石机/制图台/合成器)
    Entry useBell{};                  // 使用钟
    Entry useCampfire{};              // 使用营火
    Entry useComposter{};             // 使用堆肥桶
    Entry useDaylightDetector{};      // 使用阳光探测器
    Entry useJukebox{};               // 使用唱片机
    Entry useNoteBlock{};             // 使用音符盒
    Entry useCake{};                  // 吃蛋糕
    Entry useComparator{};            // 使用红石比较器
    Entry useRepeater{};              // 使用红石中继器
    Entry useLectern{};               // 使用讲台
    Entry useCauldron{};              // 使用炼药锅
    Entry useRespawnAnchor{};         // 使用重生锚
    Entry useBoneMeal{};              // 使用骨粉
    Entry useBeeNest{};               // 使用蜂巢(蜂箱)
    Entry editFlowerPot{};            // 编辑花盆
};

struct PermTable final {
    RolePerms        role;
    EnvironmentPerms environment;
};

static_assert(std::is_aggregate_v<PermTable>, "Reflection depends on aggregate types");
static_assert(ll::reflection::member_count_v<PermTable> == 2);
static_assert(ll::reflection::member_count_v<RolePerms::Entry> == 2);
static_assert(std::is_standard_layout_v<PermTable>);
static_assert(std::is_trivially_copyable_v<RolePerms::Entry>);

// 成员校验
template <typename T>
struct IsValidPermField : std::false_type {};
template <>
struct IsValidPermField<bool> : std::true_type {};
template <>
struct IsValidPermField<RolePerms::Entry> : std::true_type {};

template <typename T>
consteval void staticCheckPermFields() {
    static_assert(ll::reflection::is_reflectable_v<T>, "T must be reflectable");
    T t{};
    ll::reflection::forEachMember(t, [](auto, auto field) {
        using FieldT = decltype(field);
        if constexpr (ll::reflection::is_reflectable_v<FieldT>) {
            staticCheckPermFields<FieldT>();
        } else {
            static_assert(
                IsValidPermField<FieldT>::value,
                "PermTable contains invalid field type! Must be bool or Entry."
            );
        }
    });
}
inline constexpr bool PermTableStaticCheck = (staticCheckPermFields<PermTable>(), true);

} // namespace permc