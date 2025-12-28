#pragma once
#include "PermTable.hpp"


#include <ll/api/Expected.h>

namespace permc {
struct PermTable;
}
class HashedString;

namespace permc {

class PermMapping final {
public:
    using TypeName      = HashedString;
    using PermFieldName = HashedString;

    enum class FieldType { Bool, Entry };
    struct FieldDescriptor {
        size_t const    offset;
        size_t const    size;
        FieldType const type;
    };

    static PermMapping& get();

    // init -> load -> ensure -> compile
    ll::Expected<> initTypeNameMapping(std::filesystem::path const& path);

    template <typename T>
    std::optional<T> lookup(TypeName const& typeName, PermTable* table) const {
        if (auto descriptor = lookup(typeName)) {
            static_assert(std::same_as<T, bool> || std::same_as<T, RolePerms::Entry>);
            constexpr bool isBool = std::same_as<T, bool>;
            if ((isBool && descriptor->type != FieldType::Bool) || (!isBool && descriptor->type != FieldType::Entry)) {
                [[unlikely]] throw std::runtime_error("type mismatch");
            }
            auto addr = reinterpret_cast<char*>(table);              // 字节指针
            return *reinterpret_cast<T*>(addr + descriptor->offset); // 按字节偏移
        }
        return std::nullopt;
    }

private:
    void           initDefaultMapping();
    ll::Expected<> loadMapping(std::filesystem::path const& path);
    ll::Expected<> ensureMapping();
    ll::Expected<> compileFinalMapping();

    optional_ref<FieldDescriptor> lookup(TypeName const& typeName) const;

private:
    explicit PermMapping();
    ~PermMapping();
    PermMapping(PermMapping const&)            = delete;
    PermMapping& operator=(PermMapping const&) = delete;
    PermMapping(PermMapping&&)                 = delete;
    PermMapping& operator=(PermMapping&&)      = delete;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

struct InvalidPermFieldNameError final : ll::ErrorInfoBase {
    using InvalidPermFieldNames = std::vector<PermMapping::PermFieldName>;
    InvalidPermFieldNames invalidPermFieldNames;

    InvalidPermFieldNameError(InvalidPermFieldNames invalidPermFieldNames);
    ~InvalidPermFieldNameError() override;
    std::string message() const noexcept override;
};

} // namespace permc
