#pragma once
#include "RolePermMeta.hpp"
#include "perm_core/infra/HashedStringView.hpp"

#include <ll/api/Expected.h>

#include <type_traits>
#include <unordered_map>

namespace permc {

/**
 * 稀疏存储容器
 * @note 仅存储和注册表中默认值不同的数据
 * @note 非必要请不要直接查询存储结构，请使用辅助方法查询
 * @note 统一约束 true: 允许/放行, false: 拒绝/拦截, std::nullopt: 未知键
 */
class PermStorage {
public:
    struct RoleSparseStorage {
        std::optional<bool> member;
        std::optional<bool> guest;
    };

    std::unordered_map<HashedString, bool, HashedStringHasher, HashedStringEqual>              environment;
    std::unordered_map<HashedString, RoleSparseStorage, HashedStringHasher, HashedStringEqual> roles;

    /**
     * 校验稀疏存储数据
     */
    ll::Expected<> ensureData();

    /**
     * 解析环境权限值
     * @param key 权限键
     */
    [[nodiscard]] std::optional<bool> resolveEnvironment(HashedStringView key) const;

    /**
     * 解析角色权限
     * @param key 权限键
     */
    [[nodiscard]] std::optional<RolePermMeta::RoleEntry> resolveRole(HashedStringView key) const;

    /**
     * 解析指定角色权限值
     * @param key 权限键
     * @param isMember 是否为成员
     */
    [[nodiscard]] std::optional<bool> resolveRole(HashedStringView key, bool isMember) const;

    ll::Expected<> setEnvironment(HashedStringView key, bool value);

    ll::Expected<> setMemberRole(HashedStringView key, bool value);

    ll::Expected<> setGuestRole(HashedStringView key, bool value);
};
static_assert(std::is_aggregate_v<PermStorage>);

} // namespace permc