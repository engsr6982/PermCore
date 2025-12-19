#pragma once
#include "PermStorage.hpp"
#include "PermRegistry.hpp"

namespace permc {

ll::Expected<> PermStorage::ensureData() {
    { // env
        auto iter = environment.begin();
        while (iter != environment.end()) {
            auto def = PermRegistry::defaultOf(iter->first);
            if (!def.has_value() || iter->second == def.value()) {
                iter = environment.erase(iter); // 不存在的权限或值等于默认值
                continue;
            }
            ++iter;
        }
    }
    { // roles
        auto iter = roles.begin();
        while (iter != roles.end()) {
            auto& [key, val] = *iter;

            auto metaOpt = PermRegistry::getRolePermMeta(key);
            if (!metaOpt) {
                iter = roles.erase(iter); // 不存在的权限，清理节点
                continue;
            }
            const auto& def = metaOpt.value().defValue;
            // 清理和默认值相同的存储
            if (val.member == def.member) val.member.reset();
            if (val.guest == def.guest) val.guest.reset();
            if (!val.member.has_value() && !val.guest.has_value()) {
                iter = roles.erase(iter); // 所有值都和默认值相同，清理节点
                continue;
            }
            ++iter;
        }
    }
    return {};
}
std::optional<bool> PermStorage::resolveEnvironment(HashedStringView key) const {
    auto iter = environment.find(key);
    if (iter != environment.end()) {
        return iter->second; // 存储了权限状态，直接返回
    }
    return PermRegistry::defaultOf(key); // 未存储权限状态，回退查询注册表
}
std::optional<RolePermMeta::RoleEntry> PermStorage::resolveRole(HashedStringView key) const {
    auto optional = PermRegistry::getRolePermMeta(key).transform([](RolePermMeta& meta) { return meta.defValue; });
    if (!optional) {
        return std::nullopt; // 不存在的权限
    }
    auto defaultEntry = *optional;

    auto iter = roles.find(key);
    if (iter == roles.end()) {
        return defaultEntry; // 未存储状态，返回默认值
    }

    auto& stored = iter->second;
    if (stored.member.has_value()) defaultEntry.member = *stored.member; // 重写默认值
    if (stored.guest.has_value()) defaultEntry.guest = *stored.guest;
    return defaultEntry;
}
std::optional<bool> PermStorage::resolveRole(HashedStringView key, bool isMember) const {
    auto entryOpt = resolveRole(key);
    if (!entryOpt) {
        return std::nullopt; // 不存在的权限
    }
    return isMember ? entryOpt.value().member : entryOpt.value().guest;
}
ll::Expected<> PermStorage::setEnvironment(HashedStringView key, bool value) {
    auto def = PermRegistry::defaultOf(key);
    if (!def) {
        return ll::makeStringError("unregistered key");
    }
    if (def.value() == value) {
        return {}; // 和默认值相同，不存储
    }
    environment.emplace(key.getString(), value);
    return {};
}
ll::Expected<> PermStorage::setMemberRole(HashedStringView key, bool value) {
    auto def = PermRegistry::defaultOf(key, true);
    if (!def) {
        return ll::makeStringError("unregistered key");
    }
    if (def.value() == value) {
        return {}; // 和默认值相同，不存储
    }
    auto iter = roles.find(key);
    if (iter == roles.end()) {
        roles.emplace(key.getString(), RoleSparseStorage{.member = value}); // 新建节点
        return {};
    }
    iter->second.member = value; // 更新节点
    return {};
}
ll::Expected<> PermStorage::setGuestRole(HashedStringView key, bool value) {
    auto def = PermRegistry::defaultOf(key, false);
    if (!def) {
        return ll::makeStringError("unregistered key");
    }
    if (def.value() == value) {
        return {}; // 和默认值相同，不存储
    }
    auto iter = roles.find(key);
    if (iter == roles.end()) {
        roles.emplace(key.getString(), RoleSparseStorage{.guest = value}); // 新建节点
        return {};
    }
    iter->second.guest = value; // 更新节点
    return {};
}


} // namespace permc