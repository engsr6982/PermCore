#pragma once
#include "PermStorage.hpp"
#include "PermRegistry.hpp"

namespace permc {

std::optional<PermStorage::RoleSparseStorage> PermStorage::get(HashedStringView key) const {
    auto it = roles.find(key);
    if (it != roles.end()) {
        return it->second;
    }
    return PermRegistry::getRoleMeta(key).transform([](RolePermMeta& meta) -> RoleSparseStorage {
        return {.member = meta.defValue.member, .guest = meta.defValue.guest};
    });
}
void PermStorage::set(HashedStringView key, bool value, TargetField target) {
    if (target == TargetField::Environment) {
        auto def = PermRegistry::getEnvDefault(key);
        if (!def.has_value()) {
            return;
        }
        auto defValue = def.value();
        auto iter     = environment.find(key);
        if (iter != environment.end()) {
            if (value == defValue) {
                environment.erase(iter); // 值相等，擦除存储
            } else {
                iter->second = value; // 值不等，更新存储
            }
        }
        if (iter == environment.end() && value != defValue) {
            environment.emplace(key, value);
        }
        return;
    }

    auto metaOpt = PermRegistry::getRoleMeta(key);
    if (!metaOpt) {
        return;
    }
    auto const& meta = metaOpt.value();

    // 获取对应的默认值
    bool defaultValue = false;
    switch (target) {
    case TargetField::Member:
        defaultValue = meta.defValue.member;
        break;
    case TargetField::Guest:
        defaultValue = meta.defValue.guest;
        break;
    default:;
    }

    auto iterator = roles.find(key);
    if (value == defaultValue) {
        // === 稀疏化逻辑：新值等于默认值 ===
        if (iterator != roles.end()) {
            auto& set = iterator->second;
            switch (target) {
            case TargetField::Member:
                set.member.reset();
                break;
            case TargetField::Guest:
                set.guest.reset();
                break;
            default:;
            }
            if (!set.member && !set.guest) {
                roles.erase(iterator); // 如果所有字段都空了，删除整个 Entry
            }
        }
    } else {
        // === 写入逻辑：新值不等于默认值 ===
        if (iterator == roles.end()) {
            HashedString ownedKey(key.getHash(), key.getString().data());
            iterator = roles.emplace(std::move(ownedKey), RoleSparseStorage{}).first;
        }

        auto& set = iterator->second;
        switch (target) {
        case TargetField::Member:
            set.member = value;
            break;
        case TargetField::Guest:
            set.guest = value;
            break;
        default:;
        }
    }
}
std::optional<bool> PermStorage::resolve(HashedStringView key, TargetField target) const {
    if (target == TargetField::Environment) {
        auto iter = environment.find(key);
        if (iter != environment.end()) {
            return iter->second;
        }
    } else {
        auto stored = get(key);
        if (stored.has_value()) {
            if (target == TargetField::Member) {
                if (stored->member.has_value()) return *stored->member;
            } else if (target == TargetField::Guest) {
                if (stored->guest.has_value()) return *stored->guest;
            }
        }
    }
    return std::nullopt;
}

ll::Expected<> PermStorage::ensureData() {
    { // env
        auto iter = environment.begin();
        while (iter != environment.end()) {
            auto def = PermRegistry::getEnvDefault(iter->first);
            if (!def.has_value() || iter->second == def.value()) {
                iter = environment.erase(iter);
                continue;
            }
            ++iter;
        }
    }
    { // roles
        auto iter = roles.begin();
        while (iter != roles.end()) {
            auto& [key, val] = *iter;
            auto metaOpt     = PermRegistry::getRoleMeta(key);
            if (!metaOpt) {
                iter = roles.erase(iter);
                continue;
            }
            const auto& def = metaOpt.value().defValue;
            if (val.member == def.member) val.member.reset();
            if (val.guest == def.guest) val.guest.reset();
            ++iter;
        }
    }
    return {};
}

} // namespace permc