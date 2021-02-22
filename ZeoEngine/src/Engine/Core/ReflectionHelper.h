#pragma once

#include <any>
#include <optional>

#include "Engine/Core/ReflectionCore.h"

namespace ZeoEngine {

	enum class BasicMetaType
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

	BasicMetaType EvaluateMetaType(const entt::meta_type type);

	struct DataSpec
	{
	public:
		const char* DataName;
		entt::meta_data Data;
		entt::meta_any ComponentInstance;

		int32_t ElementIndex;
		bool bIsSeqElement;
		entt::meta_sequence_container SeqView;

		DataSpec() = default;
		DataSpec(entt::meta_data data, const entt::meta_any& compInstance, bool IsSeqElement)
			: Data(data)
			, ComponentInstance(compInstance)
			, bIsSeqElement(IsSeqElement)
		{
			const auto dataName = GetMetaObjectDisplayName(Data);
			DataName = *dataName;
			TryUpdateSeqView();
		}

		void Update(const entt::meta_any& compInstance, int32_t elementIndex)
		{
			// NOTE: After this (meta_any) assignment, "compInstance" may be invalid! Use "ComponentInstance" instead!
			ComponentInstance = compInstance;
			TryUpdateSeqView();
			ElementIndex = elementIndex;
		}

		size_t GetContainerSize() const
		{
			if (SeqView)
			{
				return SeqView.size();
			}

			return 0;
		}

		entt::meta_any GetValue()
		{
			if (bIsSeqElement)
			{
				return SeqView[ElementIndex];
			}
			else
			{
				return Data.get(ComponentInstance);
			}
		}

		template<typename T>
		T GetValue()
		{
			if (bIsSeqElement)
			{
				return SeqView[ElementIndex].cast<T>();
			}
			else
			{
				return Data.get(ComponentInstance).cast<T>();
			}
		}

		template<typename T>
		void SetValue(T&& value)
		{
			if (bIsSeqElement)
			{
				SeqView[ElementIndex].cast<T>() = value;
			}
			else
			{
				Data.set(ComponentInstance, std::forward<T>(value));
			}
		}

	private:
		void TryUpdateSeqView()
		{
			if (Data.type().is_sequence_container())
			{
				SeqView = Data.get(ComponentInstance).as_sequence_container();
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

	// TODO: Remove
	entt::meta_any CreateTypeDefaultValue(entt::meta_type type);

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
