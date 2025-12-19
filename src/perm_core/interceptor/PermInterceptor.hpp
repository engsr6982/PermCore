#pragma once
#include "InterceptorDelegate.hpp"
#include "perm_core/model/RolePermMeta.hpp"

#include <ll/api/event/Event.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>

#include <memory>
#include <utility>


namespace permc {

/**
 * @note 拦截器，用于拦截事件
 * @note 对于环境权限:
 *          1. 收集参数
 *          2. preCheck 预检
 *          3. 检查环境权限 (getStorage -> resolveEnvironment)
 *          4. postPolicy 后处理
 * @note 对于角色权限:
 *          1. 收集参数
 *          2. preCheck 预检
 *          3. 检查角色特权 (getRole)
 *          4. [查询 typename 对应权限 (PermMapping::lookup) (仅部分事件)]
 *          5. 检查角色权限 (getStorage -> resolveRole)
 *          6. postPolicy 后处理
 */
class PermInterceptor final {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    struct ListenerConfig {
        bool PlayerDestroyBlockEvent = true;

        // env
        bool FireSpreadEvent = true;
    };

    explicit PermInterceptor(std::unique_ptr<InterceptorDelegate> delegate, ListenerConfig const& config);
    ~PermInterceptor();

    [[nodiscard]] InterceptorDelegate&       getDelegate();
    [[nodiscard]] InterceptorDelegate const& getDelegate() const;

    template <std::derived_from<ll::event::Event> T>
    void registerListenerIf(bool cond, std::function<void(T&)>&& callback) {
        if (cond) {
            auto listener = ll::event::EventBus::getInstance().emplaceListener<T>(std::move(callback));
            this->registerListener(std::move(listener));
        }
    }


private:
    void registerListener(ll::event::ListenerPtr listener);

    // detail
    void registerPlayerInterceptor(ListenerConfig const& config);
    void registerWorldInterceptor(ListenerConfig const& config);
};

} // namespace permc