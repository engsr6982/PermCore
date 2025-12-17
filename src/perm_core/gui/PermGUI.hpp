#pragma once
#include "mc/deps/core/string/HashedString.h"
#include "perm_core/PermStorage.hpp"
#include <functional>
#include <string>
#include <vector>

class Player;

namespace permc {


struct PermGUI {
    PermGUI() = delete;

    using SnapshotEntry = std::pair<HashedString, bool>;
    using PermChangeSet   = std::vector<SnapshotEntry>;

    using OnSubmit = std::function<void(Player&, PermChangeSet set)>;

    /**
     * @brief Send a GUI to the player
     */
    static void sendTo(
        Player&                  player,
        PermStorage const&       storage,
        PermStorage::TargetField target,
        std::string const&       title,
        std::string const&       localeCode,
        OnSubmit                 submit
    );
};


} // namespace permc