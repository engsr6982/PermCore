#pragma once
#include "perm_core/model/PermRole.hpp"
#include "perm_core/model/PermTable.hpp"

#include <mc/deps/core/utility/optional_ref.h>

#include "ll/api/coro/Generator.h"

class AABB;
class BlockPos;
class Player;
class BlockSource;
class Vec3;

namespace permc {

enum class PermDecision {
    Deny    = 0, // 拒绝(立即拦截)
    Allow   = 1, // 放行(不再执行后续检查)
    Abstain = 2  // 弃权(继续执行后续检查)
};

struct InterceptorDelegate {
    virtual ~InterceptorDelegate() = default;

    virtual PermDecision preCheck(BlockSource& blockSource, BlockPos const& blockPos) = 0;

    virtual PermRole getRole(Player& player, BlockSource& blockSource, BlockPos const& blockPos) = 0;

    virtual optional_ref<PermTable> getPermTable(BlockSource& blockSource, BlockPos const& blockPos) = 0;

    virtual ll::coro::Generator<PermTable const&> queryMatrix(BlockSource& blockSource, AABB const& aabb) = 0;

    virtual PermDecision postPolicy(BlockSource& blockSource, BlockPos const& vec3) = 0;

    /**
     * @brief 活塞推拉检查
     * @param blockSource 方块源
     * @param pistonPos 活塞位置
     * @param pushPos 被推方块位置
     * @param field 应该检查的权限字段指针
     * @return 拒绝/放行/弃权
     */
    virtual PermDecision handlePistonAction(
        BlockSource&    blockSource,
        BlockPos const& pistonPos,
        BlockPos const& pushPos,
        bool EnvironmentPerms::* field
    ) = 0;

    virtual PermDecision
    handleBlockFall(BlockSource& blockSource, BlockPos const& fallPos, bool EnvironmentPerms::* field) = 0;

    template <typename Ty, typename C>
    static Ty access(C& obj, Ty C::* field) {
        return obj.*field;
    }
};


} // namespace permc