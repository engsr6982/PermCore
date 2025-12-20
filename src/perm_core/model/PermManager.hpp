#pragma once
#include <ll/api/Expected.h>

class HashedString;

namespace permc {

class PermManager final {
public:
    using AnyTypeName   = HashedString;
    using PermFieldName = HashedString;

    static PermManager& get();

    ll::Expected<> loadMapping(std::filesystem::path const& path);

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

} // namespace permc
