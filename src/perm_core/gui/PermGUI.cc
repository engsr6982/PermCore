#include "PermGUI.hpp"
#include "perm_core/model/PermMapping.hpp"

#include "ll/api/form/CustomForm.h"
#include "ll/api/form/SimpleForm.h"
#include "ll/api/i18n/I18n.h"
#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>

#include "mc/world/actor/player/Player.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace permc {

using ll::i18n_literals::operator""_trl;
void PermGUI::sendTo(Player& player, DataCtx::Ptr ctx) {
    auto& localeCode = ctx->localeCode;

    auto f = ll::form::SimpleForm{};
    f.setTitle("Perm - 权限管理"_trl(localeCode));
    f.appendButton("环境权限"_trl(localeCode), [ctx](Player& player) {
        sendEditView(player, EditTarget::Environment, ctx);
    });
    f.appendButton("成员权限"_trl(localeCode), [ctx](Player& player) {
        sendEditView(player, EditTarget::Member, ctx);
    });
    f.appendButton("游客权限"_trl(localeCode), [ctx](Player& player) { sendEditView(player, EditTarget::Guest, ctx); });
    if (ctx->back) f.appendButton("返回"_trl(localeCode), [ctx](Player& player) { ctx->back(player); });
    f.sendTo(player);
}

void PermGUI::sendEditView(Player& player, EditTarget target, DataCtx::Ptr ctx) {
    auto& localeCode = ctx->localeCode;
    auto& i18n       = ll::i18n::getInstance();

    auto f = ll::form::CustomForm();
    switch (target) {
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

    // 转储为 json
    auto fields = nlohmann::json::object();
    if (target == EditTarget::Environment) {
        fields = ll::reflection::serialize<nlohmann::json>(ctx->table.environment).value();
    } else {
        fields = ll::reflection::serialize<nlohmann::json>(ctx->table.role).value();
    }

    static_assert(std::same_as<std::remove_const_t<decltype(std::declval<RolePerms::Entry>().member)>, bool>);
    static_assert(std::same_as<std::remove_const_t<decltype(std::declval<RolePerms::Entry>().guest)>, bool>);
    for (auto& [k, v] : fields.items()) {
        bool value = false;
        switch (target) {
        case EditTarget::Environment:
            value = v.get<bool>();
            break;
        case EditTarget::Member:
            value = v["member"].get<bool>();
            break;
        case EditTarget::Guest:
            value = v["guest"].get<bool>();
            break;
        }
        f.appendToggle(k, std::string{i18n.get(k, localeCode)}, value);
    }

    f.sendTo(
        player,
        [target, ctx, fields = std::move(fields)](
            Player&                           player,
            ll::form::CustomFormResult const& data,
            ll::form::FormCancelReason
        ) mutable {
            if (!data) {
                return;
            }

            // 更新上一步的数据
            for (auto& [k, v] : fields.items()) {
                bool newVal = std::get<uint64_t>(data->at(k));
                switch (target) {
                case EditTarget::Environment:
                    v = newVal;
                    break;
                case EditTarget::Member:
                    v["member"] = newVal;
                    break;
                case EditTarget::Guest:
                    v["guest"] = newVal;
                    break;
                }
            }

            // 反序列化回结构体
            if (target == EditTarget::Environment) {
                ll::reflection::deserialize(ctx->table.environment, fields).value();
            } else {
                ll::reflection::deserialize(ctx->table.role, fields).value();
            }

            if (ctx->submit) {
                ctx->submit(player, ctx->table);
            }
        }
    );
}


} // namespace permc