#pragma once
#include <optional>
#include <type_traits>

namespace permc {

enum class PermCategory {
    Unknown,    // 未知
    Core,       // 核心
    Combat,     // 战斗
    Item,       // 物品
    Entity,     // 实体
    Tools,      // 工具
    Functional, // 功能方块
    Decorative, // 装饰
    Special,    // 特殊
    // Environment // 环境 // 由于环境权限为全局权限，采取独立注册模型
};

/**
 * 角色权限元数据
 * 用于 PermRegistry 注册角色权限默认值等信息
 */
struct RolePermMeta {
    struct RoleEntry {
        bool member;
        bool guest;
    };

    PermCategory category{PermCategory::Unknown}; // UI分类
    RoleEntry    defValue{};                      // 默认值

    static inline RolePermMeta make(PermCategory cat, bool defMember, bool defGuest) {
        return RolePermMeta{
            cat,
            {defMember, defGuest}
        };
    }
};
static_assert(std::is_aggregate_v<RolePermMeta>); // for pfr reflection

} // namespace permc