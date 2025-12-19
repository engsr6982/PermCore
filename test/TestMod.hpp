#pragma once
#include "ll/api/mod/NativeMod.h"

#include "perm_core/interceptor/PermInterceptor.hpp"

namespace test {

class TestMod {
    ll::mod::NativeMod&                     self;
    std::unique_ptr<permc::PermInterceptor> interceptor_;

public:
    TestMod() : self(*ll::mod::NativeMod::current()) {}
    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return self; }

    static TestMod& getInstance();

    bool load();
    bool enable();
    bool disable();
    bool unload();
};

} // namespace test
