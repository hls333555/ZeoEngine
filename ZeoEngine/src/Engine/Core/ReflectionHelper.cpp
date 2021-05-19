#include "ZEpch.h"
#include "Engine/Core/ReflectionHelper.h"

#include <glm/glm.hpp>
#include <IconsFontAwesome5.h>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/EngineTypes.h"

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
		if (type.is_enum())
		{
			return BasicMetaType::ENUM;
		}
		else if (type.is_sequence_container())
		{
			return BasicMetaType::SEQCON;
		}
		if (DoesPropExist(PropertyType::Struct, type))
		{
			return BasicMetaType::STRUCT;
		}
		else if (type.is_associative_container())
		{
			return BasicMetaType::ASSCON;
		}
		else
		{
			switch (type.info().hash())
			{
				// Integral
				case entt::type_hash<bool>::value():					return BasicMetaType::BOOL;
				case entt::type_hash<int8_t>::value():					return BasicMetaType::I8;
				case entt::type_hash<int32_t>::value():					return BasicMetaType::I32;
				case entt::type_hash<int64_t>::value():					return BasicMetaType::I64;
				case entt::type_hash<uint8_t>::value():					return BasicMetaType::UI8;
				case entt::type_hash<uint32_t>::value():				return BasicMetaType::UI32;
				case entt::type_hash<uint64_t>::value():				return BasicMetaType::UI64;
				// Floating point
				case entt::type_hash<float>::value():					return BasicMetaType::FLOAT;
				case entt::type_hash<double>::value():					return BasicMetaType::DOUBLE;
				// Class
				case entt::type_hash<std::string>::value():				return BasicMetaType::STRING;
				case entt::type_hash<glm::vec2>::value():				return BasicMetaType::VEC2;
				case entt::type_hash<glm::vec3>::value():				return BasicMetaType::VEC3;
				case entt::type_hash<glm::vec4>::value():				return BasicMetaType::VEC4;
				case entt::type_hash<Asset<Texture2D>>::value():		return BasicMetaType::TEXTURE;
				case entt::type_hash<Asset<ParticleTemplate>>::value():	return BasicMetaType::PARTICLE;
			}
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

	ZE_REGISTRATION
	{
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

		entt::meta<typename std::vector<bool>::reference>().conv<bool>();
	}

	const char* GetEnumDisplayName(const entt::meta_any& enumValue)
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
