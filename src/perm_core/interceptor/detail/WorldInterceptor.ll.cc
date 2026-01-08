#pragma once
#include "perm_core/interceptor/InterceptorDelegate.hpp"
#include "perm_core/interceptor/InterceptorHelper.hpp"
#include "perm_core/interceptor/PermInterceptor.hpp"

#include "ll/api/event/world/FireSpreadEvent.h"

namespace permc {

void PermInterceptor::registerLLWorldInterceptor(InterceptorConfig::ListenerConfig const& config) {
    registerListenerIf(config.FireSpreadEvent, [&]() {
        return ll::event::EventBus::getInstance().emplaceListener<ll::event::FireSpreadEvent>(
            [&](ll::event::FireSpreadEvent& ev) {
                auto& blockSource = ev.blockSource();
                auto& pos         = ev.pos();

                auto& delegate = getDelegate();
                auto  result   = delegate.preCheck(blockSource, pos);
                if (applyDecision(result, ev)) {
                    return;
                }

                if (auto table = delegate.getPermTable(blockSource, pos)) {
                    if (applyDecision(table->environment.allowFireSpread, ev)) {
                        return;
                    }
                }

                applyDecision(delegate.postPolicy(blockSource, pos), ev);
            }
        );
    });
}

} // namespace permc