#include "Reflection/ReflectionHelper.h"

namespace ZeoEngine {

	entt::meta_any GetTypeInstance(entt::meta_type type, entt::registry& registry, entt::entity entity)
	{
		return type.func("get"_hs).invoke({}, std::ref(registry), entity);
	}

	void AddType(entt::meta_type type, entt::registry& registry, entt::entity entity)
	{
		type.func("emplace"_hs).invoke({}, std::ref(registry), entity);
	}

	void RemoveType(entt::meta_type type, entt::registry& registry, entt::entity entity)
	{
		type.func("remove"_hs).invoke({}, std::ref(registry), entity);
	}

}
