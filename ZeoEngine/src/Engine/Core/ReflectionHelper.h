#pragma once

#include <any>
#include <optional>

#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	enum class BasicMetaType
	{
		NONE,
		STRUCT, // Custom struct
		SEQCON, // Sequence container
		ASSCON, // Associative container
		BOOL, I8, I32, I64, UI8, UI32, UI64,
		FLOAT, DOUBLE,
		ENUM,
		STRING, VEC2, VEC3, VEC4, TEXTURE, PARTICLE
	};

	template<typename T>
	std::optional<const char*> GetMetaObjectDisplayName(T metaObj)
	{
		auto prop = metaObj.prop(PropertyType::DisplayName);
		if (prop)
		{
			return prop.value().cast<const char*>();
		}
		else
		{
			prop = metaObj.prop(PropertyType::Name);
			if (prop)
			{
				return prop.value().cast<const char*>();
			}
		}
		return {};
	}

	static const char* GetComponentIcon(uint32_t compId);
	const char* GetComponentDisplayNameFull(uint32_t compId);

	BasicMetaType EvaluateMetaType(const entt::meta_type type);

	struct DataSpec
	{
	public:
		const char* DataName;
		entt::meta_data Data;
		entt::meta_any Instance; // Can be component instance, element instance or struct instance
		entt::meta_any ComponentInstance;

		bool bIsStructSubdata;
		bool bIsSeqElement; // NOTE: Even if this is true, Instance may not be valid!

		DataSpec() = default;
		DataSpec(entt::meta_data data, const entt::meta_any& compInstance, const entt::meta_any& instance, bool isStructSubdata, bool isSeqElement)
			: Data(data)
			, ComponentInstance(compInstance)
			, Instance(instance)
			, bIsStructSubdata(isStructSubdata)
			, bIsSeqElement(isSeqElement)
		{
			const auto dataName = GetMetaObjectDisplayName(Data);
			DataName = *dataName;
		}

		void Update(const entt::meta_any& compInstance, const entt::meta_any& instance)
		{
			// NOTE: After this (meta_any) assignment, "compInstance" may be invalid! Use "ComponentInstance" instead!
			ComponentInstance = compInstance;
			Instance = instance;
		}

		entt::meta_type GetType() const
		{
			if (bIsSeqElement) return Instance.type();
			return Data.type();
		}

		entt::meta_any GetValue()
		{
			if (bIsSeqElement)
			{
				return Instance;
			}
			else
			{
				return Data.get(Instance);
			}
		}

		template<typename T>
		T GetValue()
		{
			if (bIsSeqElement)
			{
				return Instance.cast<T>();
			}
			else
			{
				return Data.get(Instance).cast<T>();
			}
		}

		template<typename T>
		void SetValue(T&& value)
		{
			if (bIsSeqElement)
			{
				Instance.cast<T>() = value;
			}
			else
			{
				Data.set(Instance, std::forward<T>(value));
			}
		}
	};

	namespace Reflection {

		void RemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		entt::meta_any GetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
		entt::meta_any HasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);

		void BindOnDestroy(entt::meta_type compType, entt::registry& registry);

		void SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue);

	}
	
	const char* GetEnumDisplayName(entt::meta_any enumValue);

	template<typename T>
	bool IsTypeEqual(entt::meta_type type)
	{
		return type.info().hash() == entt::type_hash<T>::value();
	}

	template<typename T>
	bool DoesPropExist(PropertyType propType, T metaObj)
	{
		return static_cast<bool>(metaObj.prop(propType));
	}

	template<typename Ret, typename T>
	std::optional<Ret> GetPropValue(PropertyType propType, T metaObj)
	{
		const auto prop = metaObj.prop(propType);
		if (prop)
		{
			const auto value = prop.value();
			// As int32_t conversions are already registered, the system can convert to all registered integral types seamlessly
			return value.allow_cast<Ret>().cast<Ret>();
		}
		return {};
	}

}
