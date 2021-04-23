#pragma once

#define ZE_EXPAND(x) x
#define ZE_STRINGIFY(x) #x
#define _ZE_CAT_IMPL(a, b) a##b
#define ZE_CAT(a, b) _ZE_CAT_IMPL(a, b)

#define ZE_BIT(x) (1 << x)

#define ZE_BIND_EVENT_FUNC(func) [this](auto&&... args) -> decltype(auto) { return this->func(std::forward<decltype(args)>(args)...); }
