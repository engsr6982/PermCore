#include "PermInterceptor.hpp"

#include <ll/api/event/EventBus.h>

namespace permc {

struct PermInterceptor::Impl {
    std::unique_ptr<InterceptorDelegate> delegate;
    std::vector<ll::event::ListenerPtr>  listeners;
};

PermInterceptor::PermInterceptor(std::unique_ptr<InterceptorDelegate> delegate, InterceptorConfig const& config)
: impl(std::make_unique<Impl>()) {
    impl->delegate = std::move(delegate);

    registerLLPlayerInterceptor(config.listeners_);
    registerLLEntityInterceptor(config.listeners_);
    registerLLWorldInterceptor(config.listeners_);

    registerIlaEntityInterceptor(config.listeners_);
    registerIlaPlayerInterceptor(config.listeners_);
    registerIlaWorldInterceptor(config.listeners_);
}
PermInterceptor::~PermInterceptor() {
    auto& bus = ll::event::EventBus::getInstance();
    for (auto& listener : impl->listeners) {
        bus.removeListener(listener);
    }
}
InterceptorDelegate&       PermInterceptor::getDelegate() { return *impl->delegate; }
InterceptorDelegate const& PermInterceptor::getDelegate() const { return *impl->delegate; }
void PermInterceptor::registerListenerIf(bool cond, std::function<ll::event::ListenerPtr()> factory) {
    if (cond) {
        auto listener = factory();
        this->registerListener(std::move(listener));
    }
}
void PermInterceptor::registerListener(ll::event::ListenerPtr listener) {
    if (!listener) return;
    impl->listeners.emplace_back(std::move(listener));
}

} // namespace permc