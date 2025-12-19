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

    /**
     * 加载权限重写文件
     * @param path 文件路径
     */
    static ll::Expected<> loadOverrides(std::filesystem::path const& path);

    /**
     * 清空注册表
     */
    static void clear();

    /**
     * 注册角色权限
     * @param key 权限键
     * @param cat UI 分类
     * @param defMember 成员默认值
     * @param defGuest 访客默认值
     */
    static ll::Expected<> registerPerm(HashedStringView key, PermCategory cat, bool defMember, bool defGuest);

    /**
     * 注册环境权限
     * @param key 权限键
     * @param defVal 默认值
     */
    static ll::Expected<> registerPerm(HashedStringView key, bool defVal);

    /**
     * 获取注册的角色权限元数据
     * @param key 权限键
     * @return 权限元数据 (如果未注册则返回空)
     */
    [[nodiscard]] static optional_ref<RolePermMeta> getRolePermMeta(HashedStringView key);

    /**
     * 获取环境权限的默认值
     * @param key 权限键
     * @return 默认值 (如果未注册则返回空)
     */
    [[nodiscard]] static std::optional<bool> defaultOf(HashedStringView key);

    /**
     * 获取角色权限的默认值
     * @param key 权限键
     * @param isMember 是否为成员 (否则为访客)
     * @return 默认值 (如果未注册则返回空)
     */
    [[nodiscard]] static std::optional<bool> defaultOf(HashedStringView key, bool isMember);

    // 获取有序键（按注册时顺序排序）
    [[nodiscard]] static std::vector<HashedString> const& getEnvOrderedKeys();
    [[nodiscard]] static std::vector<HashedString> const& getRoleOrderedKeys();
};

} // namespace permc
