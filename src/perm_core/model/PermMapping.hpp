#pragma once
#include "perm_core/infra/HashedStringView.hpp"

#include <ll/api/Expected.h>

#include <filesystem>
#include <unordered_map>


namespace permc {

struct PermMapping {
    using AnyTypeName = HashedString;
    using PermKey     = HashedString;
    using Mapping     = std::unordered_map<AnyTypeName, PermKey, HashedStringHasher, HashedStringEqual>;

    PermMapping() = delete;

    template <typename Factory>
    static void buildDefault(Factory&& factory) {
        factory(mapping_);
    }

    static ll::Expected<> loadUserExtension(std::filesystem::path const& path);

    static void clear();

    // TODO: 考虑方块多权限映射问题
    static void add(AnyTypeName typeName, PermKey key);

    static optional_ref<const HashedString> lookup(std::string_view typeName);

private:
    static Mapping mapping_;
};

} // namespace permc
