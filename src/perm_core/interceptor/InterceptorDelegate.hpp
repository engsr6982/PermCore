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
};


} // namespace permc