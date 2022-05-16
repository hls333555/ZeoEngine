#include "ZEpch.h"
#include "Engine/Core/ReflectionHelper.h"

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Renderer/Shader.h"

namespace ZeoEngine {

	const char* GetComponentDisplayNameFull(uint32_t compId)
	{
		const auto compType = entt::resolve(compId);
		const char* compIcon = compType.func("get_icon"_hs).invoke({}).cast<const char*>();
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
				case entt::type_hash<bool>::value():								return BasicMetaType::BOOL;
				case entt::type_hash<int8_t>::value():								return BasicMetaType::I8;
				case entt::type_hash<int32_t>::value():								return BasicMetaType::I32;
				case entt::type_hash<int64_t>::value():								return BasicMetaType::I64;
				case entt::type_hash<uint8_t>::value():								return BasicMetaType::UI8;
				case entt::type_hash<uint32_t>::value():							return BasicMetaType::UI32;
				case entt::type_hash<uint64_t>::value():							return BasicMetaType::UI64;
				// Floating point
				case entt::type_hash<float>::value():								return BasicMetaType::FLOAT;
				case entt::type_hash<double>::value():								return BasicMetaType::DOUBLE;
				// Class
				case entt::type_hash<std::string>::value():							return BasicMetaType::STRING;
				case entt::type_hash<glm::vec2>::value():							return BasicMetaType::VEC2;
				case entt::type_hash<glm::vec3>::value():							return BasicMetaType::VEC3;
				case entt::type_hash<glm::vec4>::value():							return BasicMetaType::VEC4;
				case entt::type_hash<AssetHandle<Texture2D>>::value():				return BasicMetaType::TEXTURE;
				case entt::type_hash<AssetHandle<ParticleTemplate>>::value():		return BasicMetaType::PARTICLE;
				case entt::type_hash<AssetHandle<MeshAsset>>::value():				return BasicMetaType::MESH;
				case entt::type_hash<AssetHandle<Material>>::value():				return BasicMetaType::MATERIAL;
				case entt::type_hash<AssetHandle<Shader>>::value():			return BasicMetaType::SHADER;
			}
		}

		return BasicMetaType::NONE;
	}

	namespace Reflection {

		entt::meta_any ConstructComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
		{
			return compType.construct(entt::forward_as_meta(registry), entity);
		}

		void RemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
		{
			compType.func("remove"_hs).invoke({}, entt::forward_as_meta(registry), entity);
		}

		entt::meta_any GetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
		{
			return compType.func("get"_hs).invoke({}, entt::forward_as_meta(registry), entity);
		}

		entt::meta_any HasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
		{
			return compType.func("has"_hs).invoke({}, entt::forward_as_meta(registry), entity);
		}

		entt::meta_any CopyComponent(entt::meta_type compType, entt::registry& dstRegistry, entt::entity dstEntity, entt::meta_any& compInstance)
		{
			return compType.func("copy"_hs).invoke({}, entt::forward_as_meta(dstRegistry), dstEntity, entt::forward_as_meta(compInstance));
		}

		void BindOnComponentDestroy(entt::meta_type compType, entt::registry& registry)
		{
			compType.func("bind_on_destroy"_hs).invoke({}, entt::forward_as_meta(registry));
		}

		void SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue)
		{
			instance.type().func("set_enum_value_for_seq"_hs).invoke({}, entt::forward_as_meta(instance), entt::forward_as_meta(newValue));
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

	bool DoesTypeContainData(const entt::id_type compId)
	{
		const auto datas = entt::resolve(compId).data();
		return datas.begin() != datas.end();
	}

}
