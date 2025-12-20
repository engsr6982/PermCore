#pragma once
#include "mc/deps/core/string/HashedString.h"
#include "perm_core/model/PermTable.hpp"

#include <functional>
#include <string>
#include <vector>

class Player;

namespace permc {

struct PermGUI {
    PermGUI() = delete;

    //          / env    \
    // main -> |  member  | -> edit -> resolve -> submit
    //          \ guest  /

    struct DataCtx {
        using SubmitCallback = std::function<void(Player&, PermTable)>;
        using BackCallback   = std::function<void(Player&)>;
        using Ptr            = std::shared_ptr<DataCtx>;

        std::string    localeCode;
        PermTable      table;
        SubmitCallback submit;
        BackCallback   back = nullptr; // main form back button

        DataCtx(std::string localeCode, PermTable table, SubmitCallback submit, BackCallback back = nullptr)
        : localeCode(std::move(localeCode)),
          table(std::move(table)),
          submit(std::move(submit)),
          back(std::move(back)) {}

        DataCtx(DataCtx const&)            = delete;
        DataCtx& operator=(DataCtx const&) = delete;
    };

    template <typename... Args>
    static void sendTo(Player& player, Args&&... args) {
        sendTo(player, std::make_shared<DataCtx>(std::forward<Args>(args)...));
    }

    static void sendTo(Player& player, DataCtx::Ptr ctx);

private:
    enum class EditTarget { Environment, Member, Guest };
    static void sendEditView(Player& player, EditTarget target, DataCtx::Ptr ctx);
};

} // namespace permc