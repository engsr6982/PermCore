#pragma once
#include "PermMeta.hpp"
#include "perm_core/infra/HashedStringView.hpp"

#include <ll/api/Expected.h>

namespace permc {
struct PermMeta;
enum class PermCategory;
} // namespace permc

namespace permc {

class PermRegistry {
    static std::unordered_map<HashedString, PermMeta, HashedStringHasher, HashedStringEqual> perms_;

    static ll::Expected<> registerImpl(HashedStringView key, PermMeta meta);

public:
    PermRegistry() = delete;

    template <typename Factory>
    static void buildDefault(Factory&& factory) {
        factory(perms_);
    }

    static ll::Expected<> loadOverrides(std::filesystem::path const& path);

    static ll::Expected<> ensureOverrides();

    // 角色权限
    static ll::Expected<> registerPerm(HashedStringView key, PermCategory cat, bool defMember, bool defGuest);
    // 环境权限
    static ll::Expected<> registerPerm(HashedStringView key, PermCategory cat, bool defVal);

    static optional_ref<PermMeta> getMeta(HashedStringView key);

    static bool getEnvDefault(HashedStringView key);
    static bool getRoleDefault(HashedStringView key, bool isMember);
};

} // namespace permc
