#include "ZEpch.h"
#include "Engine/Core/ReflectionHelper.h"

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/ParticleSystem.h"

namespace ZeoEngine {

	BasicDataType DataSpec::Evaluate() const
	{
		auto DataType = Data.type();
		if (DataType.is_sequence_container())
		{
			return BasicDataType::SEQCON;
		}
		else if (DataType.is_associative_container())
		{
			return BasicDataType::ASSCON;
		}
		else if (DataType.is_integral())
		{
			if (IsTypeEqual<bool>(DataType))
			{
				return BasicDataType::BOOL;
			}
			else if (IsTypeEqual<int8_t>(DataType))
			{
				return BasicDataType::I8;
			}
			else if (IsTypeEqual<int32_t>(DataType))
			{
				return BasicDataType::I32;
			}
			else if (IsTypeEqual<int64_t>(DataType))
			{
				return BasicDataType::I64;
			}
			else if (IsTypeEqual<uint8_t>(DataType))
			{
				return BasicDataType::UI8;
			}
			else if (IsTypeEqual<uint32_t>(DataType))
			{
				return BasicDataType::UI32;
			}
			else if (IsTypeEqual<uint64_t>(DataType))
			{
				return BasicDataType::UI64;
			}
		}
		else if (DataType.is_floating_point())
		{
			if (IsTypeEqual<float>(DataType))
			{
				return BasicDataType::FLOAT;
			}
			else if (IsTypeEqual<double>(DataType))
			{
				return BasicDataType::DOUBLE;
			}
		}
		else if (DataType.is_enum())
		{
			return BasicDataType::ENUM;
		}
		else if (IsTypeEqual<std::string>(DataType))
		{
			return BasicDataType::STRING;
		}
		else if (IsTypeEqual<glm::vec2>(DataType))
		{
			return BasicDataType::VEC2;
		}
		else if (IsTypeEqual<glm::vec3>(DataType))
		{
			return BasicDataType::VEC3;
		}
		else if (IsTypeEqual<glm::vec4>(DataType))
		{
			return BasicDataType::VEC4;
		}
		else if (IsTypeEqual<Ref<Texture2D>>(DataType))
		{
			return BasicDataType::TEXTURE;
		}
		else if (IsTypeEqual<Ref<ParticleTemplate>>(DataType))
		{
			return BasicDataType::PARTICLE;
		}

		return BasicDataType::NONE;
	}

	ZE_REFL_REGISTRATION
	{
		entt::meta<bool>().ctor<>();
		entt::meta<uint8_t>().ctor<>();
		entt::meta<uint32_t>().ctor<>();
		entt::meta<uint64_t>().ctor<>();
		entt::meta<int8_t>().ctor<>();
		entt::meta<int32_t>().ctor<>();
		entt::meta<int64_t>().ctor<>();
		entt::meta<float>().ctor<>();
		entt::meta<double>().ctor<>();
		entt::meta<std::string>().ctor<>();
		entt::meta<glm::vec2>().ctor<>();
		entt::meta<glm::vec3>().ctor<>();
		entt::meta<glm::vec4>().ctor<>();
		entt::meta<Ref<Texture2D>>().ctor<>();
		entt::meta<Ref<ParticleTemplate>>().ctor<>();

		// Register common numeric conversions for registration purpose
		entt::meta<int32_t>()
			.type()
			.prop(PropertyType::Inherent)
			.conv<uint8_t>()
			.conv<uint32_t>()
			.conv<uint64_t>()
			.conv<int8_t>()
			.conv<int32_t>()
			.conv<int64_t>()
			.conv<float>()
			.conv<double>();

		entt::meta<float>()
			.type()
			.prop(PropertyType::Inherent)
			.conv<double>();

		entt::meta<double>()
			.type()
			.prop(PropertyType::Inherent)
			.conv<float>();
	}

	void InternalRemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
	{
		compType.func("remove"_hs).invoke({}, std::ref(registry), entity);
	}

	entt::meta_any InternalGetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
	{
		return compType.func("get"_hs).invoke({}, std::ref(registry), entity);
	}

	entt::meta_any InternalHasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
	{
		return compType.func("has"_hs).invoke({}, std::ref(registry), entity);
	}

	void BindOnDestroyFunc(entt::meta_type compType, entt::registry& registry)
	{
		compType.func("bind_on_destroy"_hs).invoke({}, std::ref(registry));
	}

	const char* GetEnumDisplayName(entt::meta_any enumValue)
	{
		// Get current enum value name by iterating all enum values and comparing
		for (const auto enumData : enumValue.type().data())
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

	// TODO:
	entt::meta_any CreateTypeDefaultValue(entt::meta_type type)
	{
		return type.func("create_default_value"_hs).invoke({});
	}

	void InternalInvokeOnDataValueEditChangeCallback(entt::meta_type type, entt::meta_handle instance, uint32_t dataId, std::any oldValue)
	{
		type.func("OnDataValueEditChange"_hs).invoke(std::move(instance), dataId, oldValue);
	}

	void InternalInvokePostDataValueEditChangeCallback(entt::meta_type type, entt::meta_handle instance, uint32_t dataId, std::any oldValue)
	{
		type.func("PostDataValueEditChange"_hs).invoke(std::move(instance), dataId, oldValue);
	}

}
