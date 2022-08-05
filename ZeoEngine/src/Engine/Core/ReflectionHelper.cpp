#include "ZEpch.h"
#include "Engine/Core/ReflectionHelper.h"

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Renderer/Shader.h"

namespace ZeoEngine {

	using namespace entt::literals;

	const char* GetComponentDisplayNameFull(U32 compId)
	{
		const auto compType = entt::resolve(compId);
		const char* compIcon = compType.func("get_icon"_hs).invoke({}).cast<const char*>();
		const auto compName = GetMetaObjectName(compType);
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
		if (DoesPropExist(Reflection::Struct, type))
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
				case entt::type_hash<I8>::value():						return BasicMetaType::I8;
				case entt::type_hash<I32>::value():						return BasicMetaType::I32;
				case entt::type_hash<I64>::value():						return BasicMetaType::I64;
				case entt::type_hash<U8>::value():						return BasicMetaType::UI8;
				case entt::type_hash<U32>::value():						return BasicMetaType::UI32;
				case entt::type_hash<U64>::value():						return BasicMetaType::UI64;
				// Floating point
				case entt::type_hash<float>::value():					return BasicMetaType::FLOAT;
				case entt::type_hash<double>::value():					return BasicMetaType::DOUBLE;
				// Class
				case entt::type_hash<std::string>::value():				return BasicMetaType::STRING;
				case entt::type_hash<Vec2>::value():					return BasicMetaType::VEC2;
				case entt::type_hash<Vec3>::value():					return BasicMetaType::VEC3;
				case entt::type_hash<Vec4>::value():					return BasicMetaType::VEC4;
				case entt::type_hash<Ref<Texture2D>>::value():			return BasicMetaType::TEXTURE;
				case entt::type_hash<Ref<ParticleTemplate>>::value():	return BasicMetaType::PARTICLE;
				case entt::type_hash<Ref<Mesh>>::value():				return BasicMetaType::MESH;
				case entt::type_hash<Ref<Material>>::value():			return BasicMetaType::MATERIAL;
				case entt::type_hash<Ref<Shader>>::value():				return BasicMetaType::SHADER;
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

	const char* GetEnumDisplayName(const entt::meta_any& enumValue)
	{
		// Get current enum value name by iterating all enum values and comparing
		for (const auto enumData : enumValue.type().data())
		{
			if (enumValue == enumData.get({}))
			{
				auto valueName = GetMetaObjectName(enumData);
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
