#include "ZEpch.h"
#include "Engine/Utils/ReflectionUtils.h"

#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	using namespace entt::literals;

	FieldType ReflectionUtils::MetaTypeToFieldType(entt::meta_type type)
	{
		if (type.is_enum())
		{
			return FieldType::Enum;
		}
		if (type.is_sequence_container())
		{
			return FieldType::SeqCon;
		}
		if (type.is_associative_container())
		{
			return FieldType::AssCon;
		}
		switch (type.info().hash())
		{
			// Integral
			case entt::type_hash<bool>::value():		return FieldType::Bool;
			case entt::type_hash<I8>::value():			return FieldType::I8;
			case entt::type_hash<I16>::value():			return FieldType::I16;
			case entt::type_hash<I32>::value():			return FieldType::I32;
			case entt::type_hash<I64>::value():			return FieldType::I64;
			case entt::type_hash<U8>::value():			return FieldType::U8;
			case entt::type_hash<U16>::value():			return FieldType::U16;
			case entt::type_hash<U32>::value():			return FieldType::U32;
			case entt::type_hash<U64>::value():			return FieldType::U64;
			// Floating point
			case entt::type_hash<float>::value():		return FieldType::Float;
			case entt::type_hash<double>::value():		return FieldType::Double;
			// Class
			case entt::type_hash<std::string>::value():	return FieldType::String;
			case entt::type_hash<Vec2>::value():		return FieldType::Vec2;
			case entt::type_hash<Vec3>::value():		return FieldType::Vec3;
			case entt::type_hash<Vec4>::value():		return FieldType::Vec4;
			case entt::type_hash<AssetHandle>::value():	return FieldType::Asset;
		}

		return FieldType::None;
	}

	bool ReflectionUtils::DoesComponentContainAnyField(U32 compID)
	{
		const auto dataList = entt::resolve(compID).data();
		return dataList.begin() != dataList.end();
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

	const char* ReflectionUtils::GetComponentDisplayNameFull(U32 compID)
	{
		const auto compType = entt::resolve(compID);
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

}
