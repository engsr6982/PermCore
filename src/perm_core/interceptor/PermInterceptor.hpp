#pragma once
#include "InterceptorConfig.hpp"
#include "InterceptorDelegate.hpp"

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
    explicit PermInterceptor(std::unique_ptr<InterceptorDelegate> delegate, InterceptorConfig const& config);
    ~PermInterceptor();

    [[nodiscard]] InterceptorDelegate&       getDelegate();
    [[nodiscard]] InterceptorDelegate const& getDelegate() const;

    void registerListenerIf(bool cond, std::function<ll::event::ListenerPtr()> factory);

private:
    void registerListener(ll::event::ListenerPtr listener);

    // detail
    void registerLLPlayerInterceptor(InterceptorConfig::ListenerConfig const& config);
    void registerLLEntityInterceptor(InterceptorConfig::ListenerConfig const& config);
    void registerLLWorldInterceptor(InterceptorConfig::ListenerConfig const& config);

    void registerIlaEntityInterceptor(InterceptorConfig::ListenerConfig const& config);
    void registerIlaPlayerInterceptor(InterceptorConfig::ListenerConfig const& config);
    void registerIlaWorldInterceptor(InterceptorConfig::ListenerConfig const& config);
};

} // namespace permc