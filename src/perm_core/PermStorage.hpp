#pragma once
#include "PermMeta.hpp"
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
    std::unordered_map<HashedString, PermMeta::ValueEntry, HashedStringHasher, HashedStringEqual> data;

    ll::Expected<> ensureData();

    optional_ref<const PermMeta::ValueEntry> get(HashedStringView key) const;

    enum class TargetField { Global, Member, Guest };
    void set(HashedStringView key, bool value, TargetField target);

    bool resolve(HashedStringView key, TargetField target) const;
};
static_assert(std::is_aggregate_v<PermStorage>);

} // namespace permc