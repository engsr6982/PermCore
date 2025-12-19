#pragma once
#include "perm_core/PermKeys.hpp"
#include "perm_core/interceptor/InterceptorDelegate.hpp"
#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/PermInterceptor.hpp"
#include "perm_core/model/PermStorage.hpp"

#include "ll/api/event/world/FireSpreadEvent.h"

namespace permc {

void PermInterceptor::registerWorldInterceptor(ListenerConfig const& config) {
    registerListenerIf<ll::event::FireSpreadEvent>(config.FireSpreadEvent, [&](auto& ev) {
        auto& blockSource = ev.blockSource();
        auto& pos         = ev.pos();

        auto& delegate = getDelegate();
        auto  result   = delegate.preCheck(blockSource, pos);
        if (applyDecision(result, ev)) {
            return;
        }

        auto value = delegate.getStorage(blockSource, pos).and_then([](PermStorage& storage) {
            return storage.resolveEnvironment(keys::allowFireSpread);
        });
        if (applyDecision(value, ev)) {
            return;
        }

        applyDecision(delegate.postPolicy(blockSource, pos), ev);
    });
}

} // namespace permc