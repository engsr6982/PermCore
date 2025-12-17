#pragma once
#include "PermMeta.hpp"
#include "mc/deps/core/string/HashedString.h"
#include "perm_core/infra/HashedStringView.hpp"

#include <ll/api/Expected.h>
#include <vector>

namespace permc {
struct PermMeta;
enum class PermCategory;
} // namespace permc

namespace permc {

class PermRegistry {
    static std::unordered_map<HashedString, PermMeta, HashedStringHasher, HashedStringEqual> perms_;
    static std::vector<HashedString>                                                         orderedKeys_;

    static ll::Expected<> registerImpl(HashedStringView key, PermMeta meta);

public:
    PermRegistry() = delete;

    static ll::Expected<> loadOverrides(std::filesystem::path const& path);

    static ll::Expected<> ensureOverrides();

    static void clear();

    // 角色权限
    static ll::Expected<> registerPerm(HashedStringView key, PermCategory cat, bool defMember, bool defGuest);
    // 环境权限
    static ll::Expected<> registerPerm(HashedStringView key, PermCategory cat, bool defVal);

    static optional_ref<PermMeta> getMeta(HashedStringView key);

    static bool getEnvDefault(HashedStringView key);
    static bool getRoleDefault(HashedStringView key, bool isMember);

    static std::vector<HashedString> const& getOrderedKeys();
};

} // namespace permc
