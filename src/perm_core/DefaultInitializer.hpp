#pragma once
#include "PermKeys.hpp"
#include "model/PermMapping.hpp"
#include "model/PermRegistry.hpp"

namespace permc {


struct DefaultInitializer {
    inline static ll::Expected<> initRegistry(std::filesystem::path path) {
        // role
        PermRegistry::registerPerm(keys::allowDestroy, PermCategory::Core, true, true);

        // env
        PermRegistry::registerPerm(keys::allowFireSpread, false);

        return PermRegistry::loadOverrides(path);
    }

    inline static ll::Expected<> initMapping(std::filesystem::path path) {
        // TODO: add default mapping
        return PermMapping::loadUserExtension(path);
    }
};


} // namespace permc