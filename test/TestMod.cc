#include "TestMod.hpp"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/Overload.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/command/runtime/RuntimeCommand.h"
#include "ll/api/command/runtime/RuntimeEnum.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/mod/RegisterHelper.h"

#include "perm_core/interceptor/InterceptorDelegate.hpp"

#include "nlohmann/json.hpp"
#include "perm_core/gui/PermGUI.hpp"
#include "perm_core/model/PermMapping.hpp"

namespace test {

static permc::PermRole     gRole{permc::PermRole::Gust};
static permc::PermTable    gPermTable{};
static permc::PermDecision gPreDecision{permc::PermDecision::Abstain};
static permc::PermDecision gPostDecision{permc::PermDecision::Allow};

struct MyInterceptorDelegate final : public permc::InterceptorDelegate {
    permc::PermDecision preCheck(BlockSource& blockSource, const BlockPos& blockPos) override { return gPreDecision; }
    permc::PermRole     getRole(Player& player, BlockSource& blockSource, const BlockPos& blockPos) override {
        return gRole;
    }
    permc::PermDecision postPolicy(BlockSource& blockSource, const BlockPos& vec3) override { return gPostDecision; }
    optional_ref<permc::PermTable> getPermTable(BlockSource& blockSource, const BlockPos& blockPos) override {
        return gPermTable;
    }
    permc::PermDecision preCheck(BlockSource& blockSource, const AABB& aabb) override { return gPreDecision; }
    ll::coro::Generator<permc::PermTable const&> queryMatrix(BlockSource& blockSource, const AABB& aabb) override {
        co_yield gPermTable;
    }
    permc::PermDecision postPolicy(BlockSource& blockSource, const AABB& aabb) override { return gPostDecision; }
    permc::PermDecision handlePistonAction(
        BlockSource&    blockSource,
        const BlockPos& pistonPos,
        const BlockPos& pushPos,
        bool permc::EnvironmentPerms::* field
    ) override {
        return permc::PermDecision::Abstain;
    }
    permc::PermDecision
    handleBlockFall(BlockSource& blockSource, const BlockPos& fallPos, bool permc::EnvironmentPerms::* field) override {
        return permc::PermDecision::Abstain;
    }
    permc::PermDecision handleSpread(
        BlockSource&    source,
        const BlockPos& fromPos,
        const BlockPos& toPos,
        bool permc::EnvironmentPerms::* field
    ) override {
        return permc::PermDecision::Abstain;
    }
};

TestMod& TestMod::getInstance() {
    static TestMod instance;
    return instance;
}

bool TestMod::load() {
    if (auto exp = permc::PermMapping::get().initTypeNameMapping(getSelf().getConfigDir() / "PermMapping.json"); !exp) {
        exp.error().log(getSelf().getLogger());
    }

    return true;
}

bool TestMod::enable() {

    static permc::PermInterceptor::ListenerConfig config;

    auto delegate = std::make_unique<MyInterceptorDelegate>();
    interceptor_  = std::make_unique<permc::PermInterceptor>(std::move(delegate), config);


    auto& registrar = ll::command::CommandRegistrar::getInstance();
    auto& handle    = registrar.getOrCreateCommand("permc", "PermCore debugger");

    // permc - 输出当前决策信息
    handle.overload().execute([](CommandOrigin const&, CommandOutput& output) {
        output.success("role: {}", magic_enum::enum_name(gRole));
        output.success("preDecision: {}", magic_enum::enum_name(gPreDecision));
        output.success("postDecision: {}", magic_enum::enum_name(gPostDecision));
    });
    // permc role <role> - 设置当前角色
    struct SetRole {
        permc::PermRole role;
    };
    handle.overload<SetRole>().text("role").required("role").execute(
        [](auto const&, CommandOutput& output, SetRole const& data) {
            gRole = data.role;
            output.success("set role to {}", magic_enum::enum_name(data.role));
        }
    );
    // permc preDecision <decision> - 设置当前预决策
    struct SetDecision {
        permc::PermDecision decision;
    };
    handle.overload<SetDecision>()
        .text("preDecision")
        .required("decision")
        .execute([](auto const&, CommandOutput& output, SetDecision const& data) {
            gPreDecision = data.decision;
            output.success("set preDecision to {}", magic_enum::enum_name(data.decision));
        });
    // permc postDecision <decision> - 设置当前后决策
    handle.overload<SetDecision>()
        .text("postDecision")
        .required("decision")
        .execute([](auto const&, CommandOutput& output, SetDecision const& data) {
            gPostDecision = data.decision;
            output.success("set postDecision to {}", magic_enum::enum_name(data.decision));
        });
    // permc storage - 设置权限
    handle.overload().text("storage").execute([](CommandOrigin const& origin, CommandOutput& output) {
        auto entity = origin.getEntity();
        if (!entity || !entity->isPlayer()) {
            output.error("only player can use this command");
            return;
        }
        auto player = static_cast<Player*>(entity);
        permc::PermGUI::sendTo(
            *player,
            player->getLocaleCode(),
            gPermTable,
            [](Player& player, permc::PermTable newTable) { gPermTable = std::move(newTable); }
        );
    });


    return true;
}

bool TestMod::disable() { return true; }

bool TestMod::unload() { return true; }

} // namespace test

LL_REGISTER_MOD(test::TestMod, test::TestMod::getInstance());
