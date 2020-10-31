#include "ZEpch.h"
#include "Engine/Core/ReflectionHelper.h"

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	static std::string i32_to_string(const int32_t& value)
	{
		return {};
	}

	static glm::vec2 i32_to_vec2(const int32_t& value)
	{
		return {};
	}

	static glm::vec3 i32_to_vec3(const int32_t& value)
	{
		return {};
	}

	static glm::vec4 i32_to_vec4(const int32_t& value)
	{
		return {};
	}

	static Ref<Texture2D> i32_to_texture2d(const int32_t& value)
	{
		return {};
	}

	ZE_REFL_REGISTRATION
	{
		// Register conversions from int32_t to various basic types to be used by container insertion
		entt::meta<int32_t>()
			.type()
			.prop(PropertyType::InherentType)
			.conv<bool>()
			.conv<uint8_t>()
			.conv<uint32_t>()
			.conv<uint64_t>()
			.conv<int8_t>()
			.conv<int32_t>()
			.conv<int64_t>()
			.conv<float>()
			.conv<double>()
			.conv<&i32_to_string>()
			.conv<&i32_to_vec2>()
			.conv<&i32_to_vec3>()
			.conv<&i32_to_vec4>()
			.conv<&i32_to_texture2d>();
	}

	entt::meta_any GetTypeInstance(entt::meta_type type, entt::registry& registry, entt::entity entity)
	{
		return type.func("get"_hs).invoke({}, std::ref(registry), entity);
	}

	const char* GetEnumDisplayName(entt::meta_any enumValue)
	{
		// Get current enum value name by iterating all enum values and comparing
		for (auto enumData : enumValue.type().data())
		{
			if (enumValue == enumData.get({}))
			{
				auto valueName = GetMetaObjectDisplayName(enumData);
				const char* valueNameChar = *valueName;
				return valueNameChar;
			}
		}
		return nullptr;
	}

	void SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue)
	{
		instance.type().func("set_enum_value"_hs).invoke({}, std::ref(instance), std::ref(newValue));
	}

	entt::meta_any CreateTypeDefaultValue(entt::meta_type type)
	{
		return type.func("create_default_value"_hs).invoke({});
	}

}
