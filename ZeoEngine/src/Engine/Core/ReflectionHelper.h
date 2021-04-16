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
		const char* DataName;
		entt::meta_data Data;
		// Setting instance references to mutable to make sure non-const version of as_ref() is always invoked
		mutable entt::meta_any Instance; // Can be component instance, element instance or struct instance
		mutable entt::meta_any ComponentInstance;

		bool bIsStructSubdata;
		bool bIsSeqElement;

		DataSpec() = default;
		// NOTE: Instances should not be const otherwise const version of as_ref() will be invoked, then they will become const references and cannot get modified directly
		DataSpec(entt::meta_data data, entt::meta_any& compInstance, entt::meta_any& instance, bool isStructSubdata, bool isSeqElement)
			: Data(data)
			, ComponentInstance(compInstance.as_ref())
			, Instance(instance.as_ref())
			, bIsStructSubdata(isStructSubdata)
			, bIsSeqElement(isSeqElement)
		{
			const auto dataName = GetMetaObjectDisplayName(Data);
			DataName = *dataName;
		}
		DataSpec(const DataSpec& other)
		{
			DataName = other.DataName;
			Data = other.Data;
			Instance = other.Instance.as_ref();
			ComponentInstance = other.ComponentInstance.as_ref();
			bIsStructSubdata = other.bIsStructSubdata;
			bIsSeqElement = other.bIsSeqElement;
		}
		DataSpec& operator=(const DataSpec& other)
		{
			DataName = other.DataName;
			Data = other.Data;
			Instance = other.Instance.as_ref();
			ComponentInstance = other.ComponentInstance.as_ref();
			bIsStructSubdata = other.bIsStructSubdata;
			bIsSeqElement = other.bIsSeqElement;
			return *this;
		}

		void Update(entt::meta_any& compInstance, entt::meta_any& instance)
		{
			ComponentInstance = compInstance.as_ref();
			Instance = instance.as_ref();
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
				return Instance.as_ref(); // NOTE: We must call as_ref() to return reference here or it will return a copy since entt 3.7.0
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
				if constexpr (std::is_same<T, bool>::value)
				{
					return Instance.cast<typename std::vector<bool>::reference>();
				}
				else
				{
					return Instance.cast<T>();
				}
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
				if constexpr (std::is_same<T, bool&>::value)
				{
					Instance.cast<typename std::vector<bool>::reference>() = value;
				}
				else
				{
					Instance.cast<T>() = value;
				}
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
	
	const char* GetEnumDisplayName(const entt::meta_any& enumValue);

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
