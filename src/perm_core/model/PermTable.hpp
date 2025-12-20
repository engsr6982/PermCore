#pragma once
#include <ll/api/reflection/Reflection.h>
#include <type_traits>

namespace permc {


struct EnvironmentPerms final {
    bool allowFireSpread; // 火焰蔓延
};

struct RolePerms final {
    struct Entry final {
        bool member;
        bool guest;
    };
    Entry allowDestroy{}; // 允许破坏
};

struct PermTable final {
    RolePerms        role;
    EnvironmentPerms environment;
};

static_assert(std::is_aggregate_v<PermTable>, "Reflection depends on aggregate types");
static_assert(ll::reflection::member_count_v<PermTable> == 2);
static_assert(ll::reflection::member_count_v<RolePerms::Entry> == 2);


} // namespace permc