#include "PermRegistry.hpp"
#include "PermMeta.hpp"

#include "ll/api/reflection/Deserialization.h"
#include "ll/api/reflection/Serialization.h"
#include <ll/api/io/FileUtils.h>

#include "nlohmann/json.hpp"


namespace permc {

decltype(PermRegistry::perms_) PermRegistry::perms_{};

ll::Expected<> PermRegistry::registerImpl(HashedStringView key, PermMeta meta) {
    if (perms_.contains(key)) {
        return ll::makeStringError(fmt::format("Perm already registered:{} ", key.getString()));
    }
    perms_.emplace(key, std::move(meta));
    return {};
}
ll::Expected<> PermRegistry::loadOverrides(std::filesystem::path const& path) {
    auto defaultJson = ll::reflection::serialize<nlohmann::json>(perms_);
    if (!defaultJson) {
        return ll::makeStringError(defaultJson.error().message());
    }

    if (!std::filesystem::exists(path)) {
        auto dump = defaultJson.value().dump(2);
        if (!ll::file_utils::writeFile(path, dump)) {
            return ll::makeStringError("failed to write " + path.string());
        }
        return {};
    }

    auto rawJson = ll::file_utils::readFile(path);
    if (!rawJson) {
        return ll::makeStringError("failed to read " + path.string());
    }
    try {
        auto json = nlohmann::json::parse(*rawJson);
        defaultJson->merge_patch(json);
        if (auto expected = ll::reflection::deserialize(perms_, defaultJson.value()); !expected) {
            return expected;
        }
        return ensureOverrides();
    } catch (nlohmann::json::exception const& exception) {
        return ll::makeStringError(exception.what());
    }
}
ll::Expected<> PermRegistry::ensureOverrides() {
    for (auto& [key, meta] : perms_) {
        auto& valueSet = meta.defValue;
        switch (meta.scope) {
        case PermMeta::Scope::Global:
            if (valueSet.guest || valueSet.member) {
                return ll::makeStringError(
                    fmt::format("Global scope permission '{}' cannot have 'member' or 'guest' value", key.getString())
                );
            }
            break;
        case PermMeta::Scope::Role:
            if (valueSet.global) {
                return ll::makeStringError(
                    fmt::format("Role scope permission '{}' cannot have 'global' value", key.getString())
                );
            }
            break;
        }
    }
    return {};
}
void           PermRegistry::clear() { perms_.clear(); }
ll::Expected<> PermRegistry::registerPerm(HashedStringView key, PermCategory cat, bool defMember, bool defGuest) {
    assert(cat != PermCategory::Environment);
    return registerImpl(key, PermMeta::make(cat, defMember, defGuest));
}
ll::Expected<> PermRegistry::registerPerm(HashedStringView key, PermCategory cat, bool defVal) {
    assert(cat == PermCategory::Environment);
    return registerImpl(key, PermMeta::make(cat, defVal));
}
optional_ref<PermMeta> PermRegistry::getMeta(HashedStringView key) {
    auto iter = perms_.find(key);
    if (iter == perms_.end()) {
        return {};
    }
    return {iter->second};
}
bool PermRegistry::getEnvDefault(HashedStringView key) {
    return getMeta(key).and_then(
                           [](PermMeta& meta) -> std::optional<bool> { return meta.defValue.global; }
    ).value_or(false);
}
bool PermRegistry::getRoleDefault(HashedStringView key, bool isMember) {
    return getMeta(key)
        .and_then([&isMember](PermMeta& meta) { return isMember ? meta.defValue.member : meta.defValue.guest; })
        .value_or(false);
}

} // namespace permc