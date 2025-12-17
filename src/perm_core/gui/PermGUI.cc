#include "perm_core/gui/PermGUI.hpp"
#include "perm_core/PermMeta.hpp"
#include "perm_core/PermRegistry.hpp"

#include "ll/api/form/CustomForm.h"
#include "ll/api/i18n/I18n.h"

#include "mc/world/actor/player/Player.h"

#include "magic_enum.hpp"

#include <memory>
#include <optional>
#include <string>

namespace permc {

void PermGUI::sendTo(
    Player&                  player,
    PermStorage const&       storage,
    PermStorage::TargetField target,
    std::string const&       title,
    std::string const&       localeCode,
    OnSubmit                 submit
) {
    using ll::i18n_literals::operator""_trl;
    auto f = ll::form::CustomForm();
    f.setTitle(title);

    auto& i18n     = ll::i18n::getInstance();
    auto  snapshot = std::make_shared<PermChangeSet>();

    std::optional<PermCategory> lastCategory{std::nullopt};
    for (auto& key : PermRegistry::getOrderedKeys()) {
        auto meta = PermRegistry::getMeta(key);
        if (!meta) {
            continue;
        }
        // 过滤作用域
        bool isGlobalMeta   = (meta->scope == PermMeta::Scope::Global);
        bool isGlobalTarget = (target == PermStorage::TargetField::Global);
        if (isGlobalMeta != isGlobalTarget) continue;

        if (meta->category != lastCategory) {
            lastCategory = meta->category;
            f.appendLabel("§e=== {} ===§r"_trl(localeCode, magic_enum::enum_name(*lastCategory)));
            f.appendDivider();
        }

        bool val = storage.resolve(key, target);
        f.appendToggle(key, std::string{i18n.get(key, localeCode)});
        snapshot->emplace_back(key, val);
    }

    f.sendTo(
        player,
        [snapshot, onSubmit = std::move(submit)](Player& player, ll::form::CustomFormResult const& data, auto) {
            if (!data) {
                return;
            }
            PermChangeSet changes;
            for (auto& [key, val] : *snapshot) {
                bool newVal = std::get<uint64_t>(data->at(key));
                if (newVal != val) {
                    changes.emplace_back(key, newVal);
                }
            }
            onSubmit(player, std::move(changes));
        }
    );
}


} // namespace permc