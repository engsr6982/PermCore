#pragma once

namespace permc {

enum class PermRole {
    Admin  = 0, // 全局管理员
    Owner  = 1, // 主人
    Member = 2, // 成员
    Guest   = 3  // 游客
};

}