#include "ZEpch.h"
#include "Engine/Utils/ReflectionUtils.h"

#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/GameFramework/ParticleSystem.h"

namespace ZeoEngine {

	using namespace entt::literals;

	Reflection::BasicMetaType ReflectionUtils::EvaluateType(entt::meta_type type)
	{
		if (type.is_enum())
		{
			return Reflection::BasicMetaType::ENUM;
		}
		else if (type.is_sequence_container())
		{
			return Reflection::BasicMetaType::SEQCON;
		}
		if (DoesPropertyExist(Reflection::Struct, type))
		{
			return Reflection::BasicMetaType::STRUCT;
		}
		else if (type.is_associative_container())
		{
			return Reflection::BasicMetaType::ASSCON;
		}
		else
		{
			switch (type.info().hash())
			{
				// Integral
				case entt::type_hash<bool>::value():					return Reflection::BasicMetaType::BOOL;
				case entt::type_hash<I8>::value():						return Reflection::BasicMetaType::I8;
				case entt::type_hash<I32>::value():						return Reflection::BasicMetaType::I32;
				case entt::type_hash<I64>::value():						return Reflection::BasicMetaType::I64;
				case entt::type_hash<U8>::value():						return Reflection::BasicMetaType::UI8;
				case entt::type_hash<U32>::value():						return Reflection::BasicMetaType::UI32;
				case entt::type_hash<U64>::value():						return Reflection::BasicMetaType::UI64;
				// Floating point
				case entt::type_hash<float>::value():					return Reflection::BasicMetaType::FLOAT;
				case entt::type_hash<double>::value():					return Reflection::BasicMetaType::DOUBLE;
				// Class
				case entt::type_hash<std::string>::value():				return Reflection::BasicMetaType::STRING;
				case entt::type_hash<Vec2>::value():					return Reflection::BasicMetaType::VEC2;
				case entt::type_hash<Vec3>::value():					return Reflection::BasicMetaType::VEC3;
				case entt::type_hash<Vec4>::value():					return Reflection::BasicMetaType::VEC4;
				case entt::type_hash<Ref<Texture2D>>::value():			return Reflection::BasicMetaType::TEXTURE;
				case entt::type_hash<Ref<ParticleTemplate>>::value():	return Reflection::BasicMetaType::PARTICLE;
				case entt::type_hash<Ref<Mesh>>::value():				return Reflection::BasicMetaType::MESH;
				case entt::type_hash<Ref<Material>>::value():			return Reflection::BasicMetaType::MATERIAL;
				case entt::type_hash<Ref<Shader>>::value():				return Reflection::BasicMetaType::SHADER;
			}
		}

		return Reflection::BasicMetaType::NONE;
	}

	bool ReflectionUtils::DoesTypeContainData(entt::id_type typeId)
	{
		const auto datas = entt::resolve(typeId).data();
		return datas.begin() != datas.end();
	}

	const char* ReflectionUtils::GetEnumDisplayName(const entt::meta_any& enumValue)
	{
		// Get current enum value name by iterating all enum values and comparing
		for (const auto enumData : enumValue.type().data())
		{
			if (enumValue == enumData.get({}))
			{
				return GetMetaObjectName(enumData);
			}
		}
		return nullptr;
	}

	const char* ReflectionUtils::GetComponentDisplayNameFull(U32 compId)
	{
		const auto compType = entt::resolve(compId);
		const char* compIcon = compType.func("get_icon"_hs).invoke({}).cast<const char*>();
		const char* compName = GetMetaObjectName(compType);
		static char fullCompName[128];
		strcpy_s(fullCompName, compIcon);
		strcat_s(fullCompName, "  ");
		strcat_s(fullCompName, compName);
		return fullCompName;
	}

	entt::meta_any ReflectionUtils::ConstructComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
	{
		return compType.construct(entt::forward_as_meta(registry), entity);
	}

	void ReflectionUtils::RemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
	{
		compType.func("remove"_hs).invoke({}, entt::forward_as_meta(registry), entity);
	}

	entt::meta_any ReflectionUtils::GetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
	{
		return compType.func("get"_hs).invoke({}, entt::forward_as_meta(registry), entity);
	}

	entt::meta_any ReflectionUtils::HasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity)
	{
		return compType.func("has"_hs).invoke({}, entt::forward_as_meta(registry), entity);
	}

	entt::meta_any ReflectionUtils::CopyComponent(entt::meta_type compType, entt::registry& dstRegistry, entt::entity dstEntity, entt::meta_any& compInstance)
	{
		return compType.func("copy"_hs).invoke({}, entt::forward_as_meta(dstRegistry), dstEntity, entt::forward_as_meta(compInstance));
	}

	void ReflectionUtils::BindOnComponentDestroy(entt::meta_type compType, entt::registry& registry)
	{
		compType.func("bind_on_destroy"_hs).invoke({}, entt::forward_as_meta(registry));
	}

	void ReflectionUtils::SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue)
	{
		instance.type().func("set_enum_value_for_seq"_hs).invoke({}, entt::forward_as_meta(instance), entt::forward_as_meta(newValue));
	}

}
