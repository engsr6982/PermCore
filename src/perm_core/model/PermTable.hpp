#pragma once
#include <ll/api/reflection/Reflection.h>
#include <type_traits>

namespace permc {


struct EnvironmentPerms final {
    bool allowFireSpread;   // 火焰蔓延
    bool allowMonsterSpawn; // 怪物生成
    bool allowAnimalSpawn;  // 动物生成
    bool allowActorDestroy; // 实体破坏(破坏方块、拾取方块、放置方块)
};

struct RolePerms final {
    struct Entry final {
        bool member;
        bool guest;
    };
    Entry allowDestroy{};          // 允许破坏方块
    Entry allowPlace{};            // 允许放置方块
    Entry useBucket{};             // 允许使用桶(水/岩浆/...)
    Entry allowAxePeeled{};        // 允许斧头去皮
    Entry useHoe{};                // 允许使用锄头
    Entry useShovel{};             // 允许使用铲子
    Entry placeBoat{};             // 允许放置船
    Entry placeMinecart{};         // 允许放置矿车
    Entry useButton{};             // 允许使用按钮
    Entry useDoor{};               // 允许使用门
    Entry useFenceGate{};          // 允许使用栅栏门
    Entry allowInteractEntity{};   // 允许与实体交互
    Entry useTrapdoor{};           // 允许使用活板门
    Entry editSign{};              // 允许编辑告示牌
    Entry useShulkerBox{};         // 允许使用潜影盒
    Entry useCraftingTable{};      // 允许使用工作台
    Entry useLever{};              // 允许使用拉杆 // TODO: 拉杆、按钮合并为 useRedstoneComponent ?
    Entry useFurnaces{};           // 允许使用所有熔炉类方块（熔炉、高炉、烟熏炉）
    Entry allowPlayerDamage{};     // 玩家受到伤害
    Entry allowHostileDamage{};    // 敌对生物受到伤害
    Entry allowFriendlyDamage{};   // 友好生物受到伤害
    Entry allowNeutralDamage{};    // 中立/特殊生物受到伤害
    Entry allowPlayerPickupItem{}; // 允许玩家拾取物品
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