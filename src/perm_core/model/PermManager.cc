#include "PermManager.hpp"
#include "PermTable.hpp"

#include "ll/api/reflection/Deserialization.h"
#include "ll/api/reflection/Serialization.h"
#include <ll/api/io/FileUtils.h>

#include "mc/deps/core/string/HashedString.h"
#include "mc/world/item/VanillaItemNames.h"

#include "nlohmann/json.hpp"


namespace permc {


struct PermManager::Impl {
    std::unordered_map<PermFieldName, size_t> const fieldOffset_;  // 1. 权限字段 => 权限表中的偏移量
    std::unordered_map<TypeName, PermFieldName>     typeMapping_;  // 2. TypeName => 权限字段
    std::unordered_map<TypeName, size_t>            finalMapping_; // 3. TypeName => 权限表中的偏移量

    explicit Impl() {
        {
            PermTable  table{};
            auto&      unConstMap = const_cast<std::unordered_map<PermFieldName, size_t>&>(fieldOffset_);
            auto const baseAddr   = reinterpret_cast<char const*>(&table); // 基地址

            using EntryType = RolePerms::Entry; // 目标类型：视为“叶子”，直接记录 Offset，不继续递归
            auto visitor    = [&](auto&& self, auto& currentObj) -> void {
                ll::reflection::forEachMember(currentObj, [&](std::string_view name, auto& field) {
                    using FieldType = std::remove_cvref_t<decltype(field)>;

                    // 情况 1: 是角色权限节点 (Entry) 或者是环境权限节点 (bool)
                    if constexpr (std::is_same_v<FieldType, EntryType> || std::is_same_v<FieldType, bool>) {
                        auto const   fieldAddr = reinterpret_cast<char const*>(&field);
                        size_t const offset    = fieldAddr - baseAddr;
                        unConstMap.emplace(name, offset);
                    }
                    // 情况 2: 是其他结构体 (RolePerms, EnvironmentPerms)
                    else if constexpr (ll::reflection::is_reflectable_v<FieldType>) {
                        self(self, field);
                    }
                });
            };
            visitor(visitor, table);
        }

#ifdef PERMC_DEBUG
        std::ostringstream oss;
        oss << "fieldOffset_:\n";
        for (auto const& [name, offset] : fieldOffset_) {
            oss << "  " << name.getString() << ": " << offset << "\n";
        }
        std::cout << oss.str() << std::endl;
#endif
    }
};

PermManager::PermManager() : impl_(std::make_unique<Impl>()) {}
PermManager::~PermManager() = default;
PermManager& PermManager::get() {
    static PermManager instance;
    return instance;
}
ll::Expected<> PermManager::initTypeNameMapping(std::filesystem::path const& path) {
    initDefaultMapping();
    if (auto exp = loadMapping(path); !exp) {
        return exp;
    }
    if (auto exp = ensureMapping(); !exp) {
        return exp;
    }
    return compileFinalMapping();
}
std::optional<size_t> PermManager::lookup(TypeName const& typeName) const {
    if (auto it = impl_->finalMapping_.find(typeName); it != impl_->finalMapping_.end()) {
        return it->second;
    }
    return std::nullopt;
}

#define REQUIRE_PERM_FIELD(FIELD)                                                                                      \
    static_assert(                                                                                                     \
        std::same_as<std::remove_cvref_t<decltype(std::declval<RolePerms>().FIELD)>, RolePerms::Entry>,                \
        "Please synchronize the '" #FIELD "' field in the mapping table"                                               \
    );

void PermManager::initDefaultMapping() {
    impl_->typeMapping_.clear();
    impl_->typeMapping_ = {
      // TODO
    };
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
ll::Expected<> PermManager::ensureMapping() {
    InvalidPermFieldNameError::InvalidPermFieldNames invalidNames;

    for (auto const& [typeName, fieldName] : impl_->typeMapping_) {
        if (impl_->fieldOffset_.find(fieldName) == impl_->fieldOffset_.end()) {
            invalidNames.push_back(fieldName);
        }
    }

    if (!invalidNames.empty()) {
        return ll::makeError<InvalidPermFieldNameError>(std::move(invalidNames));
    }

    return {};
}
ll::Expected<> PermManager::compileFinalMapping() {
    for (auto const& [typeName, fieldName] : impl_->typeMapping_) {
        auto iter = impl_->fieldOffset_.find(fieldName);
        if (iter != impl_->fieldOffset_.end()) {
            impl_->finalMapping_[typeName] = iter->second;
        }
    }
    return {};
}


InvalidPermFieldNameError::InvalidPermFieldNameError(InvalidPermFieldNames invalidPermFieldNames)
: invalidPermFieldNames(std::move(invalidPermFieldNames)) {}
InvalidPermFieldNameError::~InvalidPermFieldNameError() = default;
std::string InvalidPermFieldNameError::message() const noexcept {
    std::ostringstream oss;
    oss << "invalid permission field name: ";
    for (auto const& name : invalidPermFieldNames) {
        oss << name.getString() << ", ";
    }
    return oss.str();
}

} // namespace permc