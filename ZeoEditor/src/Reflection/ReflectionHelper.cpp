#include "Reflection/ReflectionHelper.h"

namespace ZeoEngine {

	entt::meta_any GetTypeInstance(entt::meta_type type, entt::registry& registry, entt::entity entity)
	{
		return type.func("get"_hs).invoke({}, std::ref(registry), entity);
	}

}
