#include "ZEpch.h"
#include "Engine/Core/ReflectionHelper.h"

#include <glm/glm.hpp>
#include <IconsFontAwesome5.h>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	const char* GetComponentIcon(uint32_t compId)
	{
		switch (compId)
		{
			case entt::type_hash<TransformComponent>::value():		return ICON_FA_MAP_MARKER_ALT;
			case entt::type_hash<SpriteRendererComponent>::value():	return ICON_FA_GHOST;
			case entt::type_hash<CameraComponent>::value():			return ICON_FA_CAMERA;
			case entt::type_hash<ParticleSystemComponent>::value():	return ICON_FA_FIRE_ALT;
		}

		return ICON_FA_CIRCLE_NOTCH;
	}

	const char* GetComponentDisplayNameFull(uint32_t compId)
	{
		const char* compIcon = GetComponentIcon(compId);
		const auto compType = entt::resolve(compId);
		const auto compName = GetMetaObjectDisplayName(compType);
		static char fullCompName[128];
		strcpy_s(fullCompName, compIcon);
		strcat_s(fullCompName, "  ");
		strcat_s(fullCompName, *compName);
		return fullCompName;
	}

	BasicMetaType EvaluateMetaType(const entt::meta_type type)
	{
		if (DoesPropExist(PropertyType::Struct, type))
		{
			return BasicMetaType::STRUCT;
		}
		else if (type.is_sequence_container())
		{
			return BasicMetaType::SEQCON;
		}
		else if (type.is_associative_container())
		{
			return BasicMetaType::ASSCON;
		}
		else if (type.is_integral())
		{
			if (IsTypeEqual<bool>(type))
			{
				return BasicMetaType::BOOL;
			}
			else if (IsTypeEqual<int8_t>(type))
			{
				return BasicMetaType::I8;
			}
			else if (IsTypeEqual<int32_t>(type))
			{
				return BasicMetaType::I32;
			}
			else if (IsTypeEqual<int64_t>(type))
			{
				return BasicMetaType::I64;
			}
			else if (IsTypeEqual<uint8_t>(type))
			{
				return BasicMetaType::UI8;
			}
			else if (IsTypeEqual<uint32_t>(type))
			{
				return BasicMetaType::UI32;
			}
			else if (IsTypeEqual<uint64_t>(type))
			{
				return BasicMetaType::UI64;
			}
		}
		else if (type.is_floating_point())
		{
			if (IsTypeEqual<float>(type))
			{
				return BasicMetaType::FLOAT;
			}
			else if (IsTypeEqual<double>(type))
			{
				return BasicMetaType::DOUBLE;
			}
		}
		else if (type.is_enum())
		{
			return BasicMetaType::ENUM;
		}
		else if (IsTypeEqual<std::string>(type))
		{
			return BasicMetaType::STRING;
		}
		else if (IsTypeEqual<glm::vec2>(type))
		{
			return BasicMetaType::VEC2;
		}
		else if (IsTypeEqual<glm::vec3>(type))
		{
			return BasicMetaType::VEC3;
		}
		else if (IsTypeEqual<glm::vec4>(type))
		{
			return BasicMetaType::VEC4;
		}
		else if (IsTypeEqual<Ref<Texture2D>>(type))
		{
			return BasicMetaType::TEXTURE;
		}
		else if (IsTypeEqual<Ref<ParticleTemplate>>(type))
		{
			return BasicMetaType::PARTICLE;
		}

		return BasicMetaType::NONE;
	}

	namespace Reflection {

		void RemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
		{
			compType.func("remove"_hs).invoke({}, std::ref(registry), entity);
		}

		entt::meta_any GetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
		{
			return compType.func("get"_hs).invoke({}, std::ref(registry), entity);
		}

		entt::meta_any HasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
		{
			return compType.func("has"_hs).invoke({}, std::ref(registry), entity);
		}

		void BindOnDestroy(entt::meta_type compType, entt::registry& registry)
		{
			compType.func("bind_on_destroy"_hs).invoke({}, std::ref(registry));
		}

		void SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue)
		{
			instance.type().func("set_enum_value_for_seq"_hs).invoke({}, std::ref(instance), std::ref(newValue));
		}

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

}
