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

1. 在 config 添加权限配置 / Add permission configuration in config

```cpp
#include "perm_core/interceptor/InterceptorConfig.hpp"

struct YourConfig {
    // 拦截器配置 / Interceptor configuration
    permc::InterceptorConfig interceptorConfig;
};
```

2. 在你的持久化模型里添加权限配置 / Add permission configuration in your persistent model

```cpp
#include "perm_core/model/PermTable.hpp"

struct YourModel {
    permc::PermTable ptable_{}; // 权限表 / Permission table
};
```

3. 实现你的 InterceptorDelegate / Implement your InterceptorDelegate

```cpp
#include "perm_core/interceptor/InterceptorDelegate.hpp"
#include <perm_core/interceptor/PermInterceptor.hpp>

class PlotInterceptorDelegate final : public permc::InterceptorDelegate {
    // ...
    // 实现所有虚函数(接口) / Implement all virtual functions (interfaces)
};
```

4. 在 mod 启动流程中添加动态映射表初始化(如果需要) / Add dynamic mapping table initialization in the mod startup process (if necessary)

```cpp
#include <perm_core/interceptor/PermInterceptor.hpp>
#include "perm_core/model/PermMapping.hpp"

bool YourMod::load() {
    // ...
    if (auto exp = permc::PermMapping::get().initTypeNameMapping(getSelf().getConfigDir() / "PermMapping.json"); !exp) {
        exp.error().log(getLogger());
        return false; // 出现异常时建议拒绝后续加载 / When an exception occurs, it is recommended to reject subsequent loading
    }
}
```

5. 在 mod 主类或其它生命周期于 mod 相同的类中持有拦截器实例 / Hold the interceptor instance in the main class or other classes with the same lifecycle as the mod

```cpp

class YourMod {
    std::unique_ptr<permc::PermInterceptor> interceptor{nullptr}; // 拦截器实例 / Interceptor instance

    bool enable() {
        // ...
        // 实例化拦截器 / Instantiate interceptor
        interceptor = std::make_unique<permc::PermInterceptor>(/* ... */);
    }

    bool disable() {
        // ...
        // 析构拦截器 / Destroy interceptor
        interceptor.reset();
        // ...
    }
}
```

> 注意：拦截器实例的生命周期需要手动保证  
> 拦截器代表(即 InterceptorDelegate)的生命周期由拦截器实例管理  
> 因此需要优先析构拦截器实例，再析构其它对象，否则会引发 seh 异常。

6. 为用户提供权限编辑 GUI / Provide users with permission editing GUI

```cpp
void example(Player& player, std::shared_ptr<YourHandle> handle) {
    permc::PermGUI::sendTo(
        player,
        player.getLocaleCode(),
        handle->getPermTable(),
        [whandle = std::weak_ptr{handle}](Player& player, permc::PermTable newTable) {
            auto localeCode = player.getLocaleCode();
            if (auto h = whandle.lock()) {
                h->setPermTable(std::move(newTable)); // 更新权限表 / Update permission table
                // 通知用户（可选） / Notify the user (optional)
            }
        }
    );
}
```

> `YourHandle` 为一个句柄类，持有权限表以及代理访问数据模型，当然你也可以使用其它方案进行传递 `PermTable`  
> `YourHandle` is a handle class that holds the permission table and accesses the data model through the proxy, and of course you can use other schemes to pass `PermTable`
