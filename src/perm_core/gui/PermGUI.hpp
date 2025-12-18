#pragma once
#include "mc/deps/core/string/HashedString.h"
#include "perm_core/PermStorage.hpp"
#include <functional>
#include <string>
#include <vector>

class Player;

namespace permc {

struct ResourceProvider {
    virtual ~ResourceProvider() = default;
    using Ptr                   = std::shared_ptr<ResourceProvider>; // form callback need to be copyable

    virtual std::string  getLocaleCode(Player& player)                                  = 0;
    virtual PermStorage& getStorage(Player& player)                                     = 0;
};

struct PermGUI {
    PermGUI() = delete;

    //             / env   \
    // entry ---> |  member | -> submit
    //             \ guest /

    static void sendTo(Player& player, ResourceProvider::Ptr provider, std::function<void(Player&)> onBack = nullptr);

    static void sendEditView(Player& player, PermStorage::TargetField targetField, ResourceProvider::Ptr provider);
};


} // namespace permc