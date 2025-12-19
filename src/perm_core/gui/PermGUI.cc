#include "PermGUI.hpp"
#include "perm_core/model/PermRegistry.hpp"
#include "perm_core/model/RolePermMeta.hpp"
#include "perm_core/gui/PermGUI.hpp"

#include "ll/api/form/CustomForm.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/i18n/I18n.h"

#include "mc/world/actor/player/Player.h"

#include "magic_enum.hpp"

#include <memory>
#include <optional>
#include <string>

namespace permc {

using ll::i18n_literals::operator""_trl;
void PermGUI::sendTo(
    Player&                      player,
    std::string                  localeCode,
    PermStorageProvider          provider,
    std::function<void(Player&)> onBack
) {
    auto f = ll::form::SimpleForm{};
    f.setTitle("Perm - 权限管理"_trl(localeCode));
    f.appendButton("全局权限"_trl(localeCode), [localeCode, provider](Player& player) {
        sendEditView(player, EditTarget::Environment, localeCode, provider);
    });
    f.appendButton("成员权限"_trl(localeCode), [localeCode, provider](Player& player) {
        sendEditView(player, EditTarget::Member, localeCode, provider);
    });
    f.appendButton("游客权限"_trl(localeCode), [localeCode, provider](Player& player) {
        sendEditView(player, EditTarget::Guest, localeCode, provider);
    });
    if (onBack) {
        f.appendButton("返回"_trl(localeCode), [onBack](Player& player) { onBack(player); });
    }
    f.sendTo(player);
}

void PermGUI::sendEditView(
    Player&             player,
    EditTarget          targetField,
    std::string         localeCode,
    PermStorageProvider provider
) {
    auto f = ll::form::CustomForm();

    switch (targetField) {
    case EditTarget::Environment:
        f.setTitle("Perm - 全局权限管理"_trl(localeCode));
        break;
    case EditTarget::Member:
        f.setTitle("Perm - 成员权限管理"_trl(localeCode));
        break;
    case EditTarget::Guest:
        f.setTitle("Perm - 游客权限管理"_trl(localeCode));
        break;
    }

    auto& i18n         = ll::i18n::getInstance();
    auto& storage      = provider(player);
    auto  appendToggle = [&](HashedString const& key) {
        std::optional<bool> value;
        switch (targetField) {
        case EditTarget::Environment:
            value = storage.resolveEnvironment(key);
            break;
        case EditTarget::Member:
            value = storage.resolveRole(key, true);
            break;
        case EditTarget::Guest:
            value = storage.resolveRole(key, false);
            break;
        }
        if (value) {
            f.appendToggle(key, std::string{i18n.get(key, localeCode)}, *value);
        }
    };

    if (targetField == EditTarget::Environment) {
        for (auto& key : PermRegistry::getEnvOrderedKeys()) {
            appendToggle(key);
        }
    } else {
        std::optional<PermCategory> lastCategory{std::nullopt};
        for (auto& key : PermRegistry::getEnvOrderedKeys()) {
            if (auto meta = PermRegistry::getRolePermMeta(key)) {
                if (meta->category != lastCategory) {
                    lastCategory = meta->category;
                    f.appendDivider();
                    f.appendLabel("§e==> {}§r"_trl(localeCode, magic_enum::enum_name(*lastCategory)));
                }
            }
            appendToggle(key);
        }
    }

    f.sendTo(
        player,
        [pr = std::move(provider),
         targetField](Player& player, ll::form::CustomFormResult const& data, ll::form::FormCancelReason) {
            if (!data) {
                return;
            }
            auto& storage = pr(player);
            for (auto& [key, variant] : *data) {
                bool             val = std::get<uint64_t>(variant);
                HashedStringView keyView{key};
                switch (targetField) {
                case EditTarget::Environment:
                    storage.setEnvironment(keyView, val);
                    break;
                case EditTarget::Member:
                    storage.setMemberRole(keyView, val);
                    break;
                case EditTarget::Guest:
                    storage.setGuestRole(keyView, val);
                    break;
                }
            }
        }
    );
}


} // namespace permc