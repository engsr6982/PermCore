#pragma once
#include "RolePermMeta.hpp"
#include "perm_core/infra/HashedStringView.hpp"

#include <ll/api/Expected.h>
#include <vector>

namespace permc {

class PermRegistry {
    struct Data {
        std::unordered_map<HashedString, bool, HashedStringHasher, HashedStringEqual>         environment;
        std::unordered_map<HashedString, RolePermMeta, HashedStringHasher, HashedStringEqual> roles;
    };

    static Data                      data_;
    static std::vector<HashedString> envOrderedKeys_;
    static std::vector<HashedString> roleOrderedKeys_;

public:
    PermRegistry() = delete;

    static ll::Expected<> loadOverrides(std::filesystem::path const& path);

    static void clear();

    // 角色权限
    static ll::Expected<> registerPerm(HashedStringView key, PermCategory cat, bool defMember, bool defGuest);
    // 环境权限
    static ll::Expected<> registerPerm(HashedStringView key, bool defVal);

    static optional_ref<RolePermMeta> getRoleMeta(HashedStringView key);

    static std::optional<bool> getEnvDefault(HashedStringView key);
    static std::optional<bool> getRoleDefault(HashedStringView key, bool isMember);

    static std::vector<HashedString> const& getEnvOrderedKeys();
    static std::vector<HashedString> const& getRoleOrderedKeys();
};

} // namespace permc
