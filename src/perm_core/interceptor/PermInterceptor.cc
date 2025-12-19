#include "PermInterceptor.hpp"

#include <ll/api/event/EventBus.h>

namespace permc {

struct PermInterceptor::Impl {
    std::unique_ptr<InterceptorDelegate> delegate;
    std::vector<ll::event::ListenerPtr>  listeners;
};

PermInterceptor::PermInterceptor(std::unique_ptr<InterceptorDelegate> delegate, ListenerConfig const& config)
: impl(std::make_unique<Impl>()) {
    impl->delegate = std::move(delegate);

    registerPlayerInterceptor(config);
    registerWorldInterceptor(config);
}
PermInterceptor::~PermInterceptor() {
    auto& bus = ll::event::EventBus::getInstance();
    for (auto& listener : impl->listeners) {
        bus.removeListener(listener);
    }
}
InterceptorDelegate&       PermInterceptor::getDelegate() { return *impl->delegate; }
InterceptorDelegate const& PermInterceptor::getDelegate() const { return *impl->delegate; }
void                       PermInterceptor::registerListener(ll::event::ListenerPtr listener) {
    if (!listener) return;
    impl->listeners.emplace_back(std::move(listener));
}

} // namespace permc