#pragma once
#include <ll/api/Expected.h>

namespace permc {
struct PermTable;
}
class HashedString;

namespace permc {

class PermManager final {
public:
    using TypeName      = HashedString;
    using PermFieldName = HashedString;

    static PermManager& get();

    // init -> load -> ensure -> compile
    ll::Expected<> initTypeNameMapping(std::filesystem::path const& path);

    std::optional<size_t> lookup(TypeName const& typeName) const;

    template <typename T>
    std::optional<T> lookup(TypeName const& typeName, PermTable* table) const {
        if (auto offset = lookup(typeName)) {
            auto addr = reinterpret_cast<char*>(table);   // 字节指针
            return *reinterpret_cast<T*>(addr + *offset); // 按字节偏移
        }
        return std::nullopt;
    }

private:
    void           initDefaultMapping();
    ll::Expected<> loadMapping(std::filesystem::path const& path);
    ll::Expected<> ensureMapping();
    ll::Expected<> compileFinalMapping();

private:
    explicit PermManager();
    ~PermManager();
    PermManager(PermManager const&)            = delete;
    PermManager& operator=(PermManager const&) = delete;
    PermManager(PermManager&&)                 = delete;
    PermManager& operator=(PermManager&&)      = delete;

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

struct InvalidPermFieldNameError final : ll::ErrorInfoBase {
    using InvalidPermFieldNames = std::vector<PermManager::PermFieldName>;
    InvalidPermFieldNames invalidPermFieldNames;

    InvalidPermFieldNameError(InvalidPermFieldNames invalidPermFieldNames);
    ~InvalidPermFieldNameError() override;
    std::string message() const noexcept override;
};

} // namespace permc
