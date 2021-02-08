#pragma once

#include <any>
#include <optional>

#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	enum class BasicDataType
	{
		NONE,
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

	struct DataSpec
	{
		const char* DataName;
		entt::meta_data Data;
		entt::meta_any ComponentInstance;

		DataSpec() = default;
		DataSpec(entt::meta_data data, const entt::meta_any& compInstance)
			: Data(data)
			, ComponentInstance(compInstance)
		{
			const auto dataName = GetMetaObjectDisplayName(Data);
			DataName = *dataName;
		}

		BasicDataType Evaluate() const;

		entt::meta_any GetValue()
		{
			return Data.get(ComponentInstance);
		}

		// NOTE: This function cannot be const or crash will occur!
		template<typename T>
		T GetValue()
		{
			return Data.get(ComponentInstance).cast<T>();
		}

		template<typename T>
		void SetValue(T&& value)
		{
			Data.set(ComponentInstance, std::forward<T>(value));
		}
	};

	void InternalRemoveComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
	entt::meta_any InternalGetComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
	entt::meta_any InternalHasComponent(entt::meta_type compType, entt::registry& registry, entt::entity entity);
	void BindOnDestroyFunc(entt::meta_type compType, entt::registry& registry);

	const char* GetEnumDisplayName(entt::meta_any enumValue);

	void SetEnumValueForSeq(entt::meta_any& instance, entt::meta_any& newValue);

	entt::meta_any CreateTypeDefaultValue(entt::meta_type type);

	void InternalInvokeOnDataValueEditChangeCallback(entt::meta_type type, entt::meta_handle instance, uint32_t dataId, std::any oldValue);
	void InternalInvokePostDataValueEditChangeCallback(entt::meta_type type, entt::meta_handle instance, uint32_t dataId, std::any oldValue);


	template<typename T>
	bool IsTypeEqual(entt::meta_type type)
	{
		return type.info().hash() == entt::type_hash<T>::value();
	}

	template<typename T>
	T GetDataValueByRef(entt::meta_data data, entt::meta_handle instance)
	{
		return data.get(std::move(instance)).cast<T>();
	}

	template<typename T>
	T GetDataValue(entt::meta_data data, entt::meta_handle instance)
	{
		return data.get(std::move(instance)).cast<T>();
	}

	template<typename T>
	void SetDataValue(entt::meta_data data, entt::meta_handle instance, T&& value)
	{
		data.set(std::move(instance), std::forward<T>(value));
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
