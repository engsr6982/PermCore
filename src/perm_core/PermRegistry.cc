#include "PermRegistry.hpp"
#include "RolePermMeta.hpp"

#include "ll/api/reflection/Deserialization.h"
#include "ll/api/reflection/Serialization.h"
#include <ll/api/io/FileUtils.h>

#include "mc/deps/core/string/HashedString.h"

#include "nlohmann/json.hpp"

#include <vector>

namespace permc {

decltype(PermRegistry::data_)            PermRegistry::data_{};
decltype(PermRegistry::envOrderedKeys_)  PermRegistry::envOrderedKeys_{};
decltype(PermRegistry::roleOrderedKeys_) PermRegistry::roleOrderedKeys_{};

ll::Expected<> PermRegistry::loadOverrides(std::filesystem::path const& path) {
    auto defaultJson = ll::reflection::serialize<nlohmann::json>(data_);
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
        if (auto expected = ll::reflection::deserialize(data_, defaultJson.value()); !expected) {
            return expected;
        }
        return {};
    } catch (nlohmann::json::exception const& exception) {
        return ll::makeStringError(exception.what());
    }
}
void PermRegistry::clear() {
    data_.environment.clear();
    data_.roles.clear();
    envOrderedKeys_.clear();
    roleOrderedKeys_.clear();
}
ll::Expected<> PermRegistry::registerPerm(HashedStringView key, PermCategory cat, bool defMember, bool defGuest) {
    if (data_.roles.contains(key)) {
        return ll::makeStringError(fmt::format("perm already registered: {}", key.getString()));
    }
    data_.roles.emplace(key.getString(), RolePermMeta::make(cat, defMember, defGuest));
    roleOrderedKeys_.emplace_back(key.getString());
    return {};
}
ll::Expected<> PermRegistry::registerPerm(HashedStringView key, bool defVal) {
    if (data_.environment.contains(key)) {
        return ll::makeStringError(fmt::format("perm already registered: {}", key.getString()));
    }
    data_.environment.emplace(key.getString(), defVal);
    envOrderedKeys_.emplace_back(key.getString());
    return {};
}
optional_ref<RolePermMeta> PermRegistry::getRoleMeta(HashedStringView key) {
    auto iter = data_.roles.find(key);
    if (iter == data_.roles.end()) {
        return {};
    }
    return {iter->second};
}
std::optional<bool> PermRegistry::getEnvDefault(HashedStringView key) {
    auto iter = data_.environment.find(key);
    if (iter == data_.environment.end()) {
        return std::nullopt;
    }
    return iter->second;
}
std::optional<bool> PermRegistry::getRoleDefault(HashedStringView key, bool isMember) {
    return getRoleMeta(key).transform([&isMember](RolePermMeta& meta) {
        return isMember ? meta.defValue.member : meta.defValue.guest;
    });
}
std::vector<HashedString> const& PermRegistry::getEnvOrderedKeys() { return envOrderedKeys_; }
std::vector<HashedString> const& PermRegistry::getRoleOrderedKeys() { return roleOrderedKeys_; }

} // namespace permc