#include "PermManager.hpp"
#include "PermTable.hpp"

#include "ll/api/reflection/Deserialization.h"
#include "ll/api/reflection/Serialization.h"
#include <ll/api/io/FileUtils.h>

#include "mc/deps/core/string/HashedString.h"

#include "nlohmann/json.hpp"


namespace permc {


struct PermManager::Impl {
    std::unordered_map<PermFieldName, size_t>      fieldOffset_; // 1. 权限字段 => 权限表中的偏移量
    std::unordered_map<AnyTypeName, PermFieldName> typeMapping_; // 2. TypeName => 权限字段
    std::unordered_map<AnyTypeName, size_t>        typeOffset_;  // 3. TypeName => 权限表中的偏移量

    explicit Impl() {
        PermTable  table{};
        auto const baseAddr = reinterpret_cast<char const*>(&table); // 基地址

        using EntryType = RolePerms::Entry; // 目标类型：视为“叶子”，直接记录 Offset，不继续递归
        auto visitor    = [&](auto&& self, auto& currentObj) -> void {
            ll::reflection::forEachMember(currentObj, [&](std::string_view name, auto& field) {
                using FieldType = std::remove_cvref_t<decltype(field)>;

                // 情况 1: 是角色权限节点 (Entry) 或者是环境权限节点 (bool)
                if constexpr (std::is_same_v<FieldType, EntryType> || std::is_same_v<FieldType, bool>) {
                    auto const   fieldAddr = reinterpret_cast<char const*>(&field);
                    size_t const offset    = fieldAddr - baseAddr;
                    fieldOffset_.emplace(name, offset);
                }
                // 情况 2: 是其他结构体 (RolePerms, EnvironmentPerms)
                else if constexpr (ll::reflection::is_reflectable_v<FieldType>) {
                    self(self, field);
                }
            });
        };
        visitor(visitor, table);

#ifdef PERMC_DEBUG
        std::ostringstream oss;
        oss << "fieldOffset_:\n";
        for (auto const& [name, offset] : fieldOffset_) {
            oss << "  " << name.getString() << ": " << offset << "\n";
        }
#endif
    }
};

PermManager::PermManager() : impl_(std::make_unique<Impl>()) {}
PermManager::~PermManager() = default;
PermManager& PermManager::get() {
    static PermManager instance;
    return instance;
}

ll::Expected<> PermManager::loadMapping(std::filesystem::path const& path) {
    auto defaultJson = ll::reflection::serialize<nlohmann::json>(impl_->typeMapping_);
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
        if (auto expected = ll::reflection::deserialize(impl_->typeMapping_, defaultJson.value()); !expected) {
            return expected;
        }
        return {};
    } catch (nlohmann::json::exception const& exception) {
        return ll::makeStringError(exception.what());
    }
}


} // namespace permc