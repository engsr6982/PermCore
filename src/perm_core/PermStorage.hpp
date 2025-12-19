#pragma once
#include "RolePermMeta.hpp"
#include "perm_core/infra/HashedStringView.hpp"

#include <ll/api/Expected.h>

#include <type_traits>
#include <unordered_map>

namespace permc {

/**
 * 稀疏存储容器
 */
class PermStorage {
public:
    struct RoleSparseStorage {
        std::optional<bool> member;
        std::optional<bool> guest;
    };

    std::unordered_map<HashedString, bool, HashedStringHasher, HashedStringEqual>              environment;
    std::unordered_map<HashedString, RoleSparseStorage, HashedStringHasher, HashedStringEqual> roles;

    ll::Expected<> ensureData();

    std::optional<RoleSparseStorage> get(HashedStringView key) const;

    enum class TargetField { Environment, Member, Guest };
    void set(HashedStringView key, bool value, TargetField target);

    // true: pass, false: deny, none: undefined(continue)
    std::optional<bool> resolve(HashedStringView key, TargetField target) const;
};
static_assert(std::is_aggregate_v<PermStorage>);

} // namespace permc