#pragma once
#include "PermStorage.hpp"
#include "PermRegistry.hpp"

namespace permc {

optional_ref<const PermMeta::ValueEntry> PermStorage::get(HashedStringView key) const {
    auto it = data.find(key);
    if (it != data.end()) {
        return {it->second};
    }
    return {PermRegistry::getMeta(key).transform([](PermMeta& meta) -> PermMeta::ValueEntry& {
        return meta.defValue;
    })};
}
void PermStorage::set(HashedStringView key, bool value, TargetField target) {
    auto metaOpt = PermRegistry::getMeta(key);
    if (!metaOpt) {
        return;
    }
    auto const& meta = metaOpt.value();

    // 校验 Scope 是否匹配 (防止把 Role 权限设成了 Global)
    if (target == TargetField::Global && meta.scope != PermMeta::Scope::Global) return;
    if (target != TargetField::Global && meta.scope != PermMeta::Scope::Role) return;

    // 获取对应的默认值
    bool defaultValue = false;
    switch (target) {
    case TargetField::Global:
        defaultValue = *meta.defValue.global;
        break;
    case TargetField::Member:
        defaultValue = *meta.defValue.member;
        break;
    case TargetField::Guest:
        defaultValue = *meta.defValue.guest;
        break;
    }

    auto it = data.find(key);
    if (value == defaultValue) {
        // === 稀疏化逻辑：新值等于默认值 ===
        if (it != data.end()) {
            auto& set = it->second;
            switch (target) {
            case TargetField::Global:
                set.global.reset();
                break;
            case TargetField::Member:
                set.member.reset();
                break;
            case TargetField::Guest:
                set.guest.reset();
                break;
            }
            // 如果所有字段都空了，删除整个 Entry
            if (!set.global && !set.member && !set.guest) {
                data.erase(it);
            }
        }
    } else {
        // === 写入逻辑：新值不等于默认值 ===
        if (it == data.end()) {
            HashedString ownedKey(key.getHash(), key.getString().data());
            it = data.emplace(std::move(ownedKey), PermMeta::ValueEntry{}).first;
        }

        auto& set = it->second;
        switch (target) {
        case TargetField::Global:
            set.global = value;
            break;
        case TargetField::Member:
            set.member = value;
            break;
        case TargetField::Guest:
            set.guest = value;
            break;
        }
    }
}
bool PermStorage::resolve(HashedStringView key, TargetField target) const {
    auto stored = get(key);
    if (stored.has_value()) {
        switch (target) {
        case TargetField::Global:
            if (stored->global.has_value()) return *stored->global;
            break;
        case TargetField::Member:
            if (stored->member.has_value()) return *stored->member;
            break;
        case TargetField::Guest:
            if (stored->guest.has_value()) return *stored->guest;
            break;
        }
    }
    return false;
}

ll::Expected<> PermStorage::ensureData() {
    auto iter = data.begin();
    while (iter != data.end()) {
        auto& [key, val] = *iter;
        auto metaOpt     = PermRegistry::getMeta(key);
        if (!metaOpt) {
            iter = data.erase(iter);
            continue;
        }
        const auto& def = metaOpt.value().defValue;
        if (val.global == def.global) val.global.reset();
        if (val.member == def.member) val.member.reset();
        if (val.guest == def.guest) val.guest.reset();
        if (!val.global && !val.member && !val.guest) {
            iter = data.erase(iter);
            continue;
        }
        ++iter;
    }
    return {};
}

} // namespace permc