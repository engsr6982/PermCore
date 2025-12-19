#pragma once
#include "../model/PermStorage.hpp"
#include "mc/deps/core/string/HashedString.h"
#include <functional>
#include <string>
#include <vector>

class Player;

namespace permc {

struct PermGUI {
    PermGUI() = delete;

    using PermStorageProvider = std::function<PermStorage&(Player&)>;

    //             / env   \
    // entry ---> |  member | -> save
    //             \ guest /

    static void sendTo(
        Player&                      player,
        std::string                  localeCode,
        PermStorageProvider          provider,
        std::function<void(Player&)> onBack = nullptr
    );

    enum class EditTarget { Environment, Member, Guest };
    static void
    sendEditView(Player& player, EditTarget targetField, std::string localeCode, PermStorageProvider provider);
};

} // namespace permc