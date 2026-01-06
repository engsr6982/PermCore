# PermCore for Minecraft Bedrock Server

此库封装了 Minecraft bedrock server 的事件监听、权限模型，对于上层地皮、领地等可轻松接入，统一维护。

The library encapsulates the event listening and permission model of Minecraft bedrock server, which can be easily accessed and maintained for upper land, territory, etc.

## 使用 / Usage

由于依赖过于复杂、且因为 ODR 等问题，本仓库建议使用 submodules 引入。

Because of the complexity of the dependencies and the ODR problem, this repository recommends using submodules to introduce.

```bash
cd /path/to/your/xmake-project

git submodule add 'git@github.com:IceBlcokMC/PermCore.git'
```

- 配置 xmake.lua / Configure xmake.lua

```lua

-- 1. 包含构建脚本 / include build script
include("PermCore/static_lib.lua")

target("your_project")
    -- 2. 添加依赖 / add depends
    add_deps("PermCore")
    -- 3. 添加 include / add include
    add_includedirs("PermCore/src")
    -- 4. 添加 fmt 宏避免 ODR / add fmt macro to avoid ODR
    add_defines("FMT_HEADER_ONLY=1")
```

## 引入 / Import

TODO
