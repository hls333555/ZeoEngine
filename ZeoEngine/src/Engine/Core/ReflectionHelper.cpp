#include "ZEpch.h"
#include "Engine/Core/ReflectionHelper.h"

namespace ZeoEngine {

	void AddType(entt::meta_type type, entt::registry& registry, entt::entity entity)
	{
		type.construct(std::ref(registry), entity);
	}

	entt::meta_any AddTypeById(entt::id_type typeId, entt::registry& registry, entt::entity entity)
	{
		return entt::resolve_type(typeId).construct(std::ref(registry), entity);
	}

	void RemoveType(entt::meta_type type, entt::registry& registry, entt::entity entity)
	{
		type.func("remove"_hs).invoke({}, std::ref(registry), entity);
	}

	entt::meta_any GetTypeInstance(entt::meta_type type, entt::registry& registry, entt::entity entity)
	{
		return type.func("get"_hs).invoke({}, std::ref(registry), entity);
	}

}
